#include "manager/node_manager.h"

#include <csignal>
#include <iostream>
#include <exception>
#include <memory>
#include <mutex>
#include <string>

#include "Version.h"
#ifdef ROS_FOUND
#include <ros/ros.h>
#include <ros/package.h>
#elif ROS2_FOUND
#include <rclcpp/rclcpp.hpp>
#endif

namespace {
#ifdef ROS2_FOUND
std::mutex g_mtx;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables) signal-handler state
std::condition_variable g_cv;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables) signal-handler state
#endif

bool g_sig_recv = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables) signal-handler state

void sigHandler(int sig) {
  (void)sig;
  g_sig_recv = true;
#ifdef ROS_FOUND
  ros::shutdown();
#elif ROS2_FOUND
  g_cv.notify_all();
#endif
}
}  // namespace

int main(int argc, char** argv) {  // NOLINT(misc-const-correctness) ros::init/rclcpp::init require non-const argv
  std::cout << "-------- Hesai Lidar ROS V" << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_TINY
            << " --------" << std::endl;
  (void)signal(SIGINT, sigHandler);  ///< bind ctrl+c signal with the sigHandler function
#ifdef ROS_FOUND
  ros::init(argc, argv, "hesai_ros_driver_node", ros::init_options::NoSigintHandler);
#elif ROS2_FOUND
  rclcpp::init(argc, argv);
#else
  (void)argc;
  (void)argv;
#endif

  std::string config_path{};

#ifdef RUN_IN_ROS_WORKSPACE
  config_path = ros::package::getPath("hesai_ros_driver");
#else
  config_path = static_cast<std::string>(PROJECT_PATH);
#endif

  config_path += "/config/config.yaml";

#ifdef ROS_FOUND
  ros::NodeHandle priv_hh("~");
  std::string path;
  priv_hh.param("config_path", path, std::string(""));
  if (!path.empty()) {
    config_path = path;
  }
#elif ROS2_FOUND
  // workaround to get config_path from ros parameter
  auto node = rclcpp::Node::make_shared("hesai_ros_driver_node");
  std::string path = node->declare_parameter<std::string>("config_path", "");
  node.reset();
  if (!path.empty()) {
    config_path = path;
  }
#endif

  YAML::Node config;
  try {
    config = YAML::LoadFile(config_path);
  } catch (const std::exception& e) {
    std::cerr << "Load config error: " << e.what() << std::endl;
    return -1;
  }
  std::shared_ptr<NodeManager> demo_ptr = std::make_shared<NodeManager>();
  demo_ptr->Init(config);
  demo_ptr->Start();
  // you can chose [!demo_ptr->IsPlayEnded()] or [1]
  // If you chose !demo_ptr->IsPlayEnded(), ROS node will end with the end of the PCAP.
  // If you select 1, the ROS node does not end with the end of the PCAP.
  while (!demo_ptr->IsPlayEnded() && !g_sig_recv) {
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
  demo_ptr->Stop();
  return 0;
}
