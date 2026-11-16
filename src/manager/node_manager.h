#pragma once
#define BOOST_BIND_GLOBAL_PLACEHOLDERS  // retain the Bind placeholders (_1, _2, ...) in the global namespace
#include <vector>

#include "utility/yaml_reader.hpp"
#ifdef ROS_FOUND
#include "source_driver_ros1.hpp"
#elif ROS2_FOUND
#include "source_driver_ros2.hpp"
#endif

class NodeManager {
 public:
  // Initialize ROS nodes based on configuration files
  void Init(const YAML::Node& config);
  // Start working
  void Start();
  // Stop working
  void Stop();
  bool IsPlayEnded();
  ~NodeManager();
  NodeManager() = default;

  NodeManager(const NodeManager&) = delete;
  NodeManager& operator=(const NodeManager&) = delete;
  NodeManager(NodeManager&&) = delete;
  NodeManager& operator=(NodeManager&&) = delete;

  [[nodiscard]] const std::vector<SourceDriver::Ptr>& GetSourcesDriver() const;

 private:
  std::vector<SourceDriver::Ptr> sources_driver_{};
};
