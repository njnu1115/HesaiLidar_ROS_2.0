#pragma once

#include <yaml-cpp/yaml.h>

#include <cstdlib>
#include <iostream>
#include <string>

template <typename T>
inline void YamlReadAbort(const YAML::Node& yaml, const std::string& key, T& out_val) {
  if (!yaml[key] || yaml[key].Type() == YAML::NodeType::Null) {
    std::cout << " : Not set " << key << std::endl;
    std::cout << " value, Aborting!!!" << std::endl;
    exit(EXIT_FAILURE);
  }
  out_val = yaml[key].as<T>();
}

template <typename T>
inline bool YamlRead(const YAML::Node& yaml, const std::string& key, T& out_val, const T& default_val) {
  if (!yaml[key] || yaml[key].Type() == YAML::NodeType::Null) {
    out_val = default_val;
    return false;
  }
  out_val = yaml[key].as<T>();
  return true;
}

inline YAML::Node YamlSubNodeAbort(const YAML::Node& yaml, const std::string& node) {
  YAML::Node ret = yaml[node.c_str()];
  if (!ret) {
    std::cout << " : Cannot find sub node " << node << ". Aborting!!!" << std::endl;
    exit(EXIT_FAILURE);
  }
  return ret;
}
