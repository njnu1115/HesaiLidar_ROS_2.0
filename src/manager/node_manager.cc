#include "manager/node_manager.h"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#ifdef ROS_FOUND
#include <cstdlib>  // for system()
#endif

void NodeManager::Init(const YAML::Node& config) {
  YAML::Node lidar_config = YamlSubNodeAbort(config, "lidar");
  for (size_t i = 0; i < lidar_config.size(); ++i) {
    std::shared_ptr<SourceDriver> source;
    source = std::make_shared<SourceDriver>(SourceType::DATA_FROM_LIDAR);
    source->Init(lidar_config[i]);
    sources_driver_.emplace_back(source);
  }
}

void NodeManager::Start() {
  for (auto& iter : sources_driver_) {
    if (iter != nullptr) {
      iter->Start();
    }
  }
}

void NodeManager::Stop() {
  for (auto& iter : sources_driver_) {
    if (iter != nullptr) {
      iter->Stop();
    }
  }
}

NodeManager::~NodeManager() { Stop(); }

[[nodiscard]] const std::vector<SourceDriver::Ptr>& NodeManager::GetSourcesDriver() const { return sources_driver_; }

bool NodeManager::IsPlayEnded() {
  size_t num{GetSourcesDriver().size()};  // NOLINT(cppcoreguidelines-init-variables) brace-init is intentional
  bool all_pcap_end = true;
#ifdef ROS_FOUND
  for (size_t i = 0; i < num; i++) {
    all_pcap_end = GetSourcesDriver()[i]->driver_ptr_->lidar_ptr_->IsPlayEnded();
    if (!all_pcap_end) {
      break;
    }
  }
  if (all_pcap_end) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "-----------------" << num
              << " pcap(s) end, we will close the node(s)!!!-----------------" << std::endl;
    // NOLINTNEXTLINE(cert-env33-c,bugprone-command-processor) intentional process spawn to tear down rviz
    if (system("rosnode kill rviz") == -1) {
      std::cerr << "Command Execution Error: rosnode kill rviz" << std::endl;
      all_pcap_end = false;
    }
  }
#elif ROS2_FOUND
  for (size_t i = 0; i < num; i++) {
    all_pcap_end = GetSourcesDriver()[i]->driver_ptr_->lidar_ptr_->IsPlayEnded();
    if (!all_pcap_end) {
      break;
    }
  }
  if (all_pcap_end) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "-----------------" << num
              << " pcap(s) end, we will close the node(s)!!!-----------------" << std::endl;
    std::cout.flush();
    // NOLINTNEXTLINE(cert-env33-c,bugprone-command-processor) intentional process spawn to tear down rviz2
    if (system("pkill -f rviz2") == -1) {
      std::cerr << "Command Execution Error: pkill -f rviz2" << std::endl;
      all_pcap_end = false;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    std::cout.flush();
  }
#endif
  return all_pcap_end;
}
