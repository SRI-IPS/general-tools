#pragma once

#include <string>

namespace a17 {
namespace dispatch {

struct Topic {
  std::string device_name = "";
  std::string node_name = "";
  std::string topic = "";

  Topic(const std::string &device_name, const std::string &node_name, const std::string &topic)
      : device_name(device_name), node_name(node_name), topic(topic) {}

  /// Returns the fully-qualified string representing the topic.
  std::string str() const;
};

}  // namespace dispatch
}  // namespace a17
