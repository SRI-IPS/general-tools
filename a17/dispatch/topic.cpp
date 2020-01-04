#include "topic.h"

namespace a17 {
namespace dispatch {

std::string Topic::str() const {
  auto str = std::string{""};
  if (!device_name.empty()) {
    str.append(device_name);
  }
  if (!node_name.empty()) {
    if (!str.empty() && str.back() != '/') {
      str.append("/");
    }
    str.append(node_name);
  }
  if (!topic.empty()) {
    if (!str.empty() && str.back() != '/') {
      str.append("/");
    }
    str.append(topic);
  }
  return str;
}

}  // namespace dispatch
}  // namespace a17
