#pragma once

#include <set>
#include <sstream>
#include <string>

#include "defs.h"

namespace a17 {
namespace dispatch {

struct DirectoryTopic {
  std::string name;
  long int timestamp = 0;
  int socketType = 0;
  std::string address;
  std::set<message_type> inputTypes;
  std::set<message_type> outputTypes;
  std::string guid;
  std::string info;

  std::string to_string() const noexcept;

  friend std::ostream &operator<<(std::ostream &out, const DirectoryTopic &obj);
  bool operator==(const DirectoryTopic &other) const noexcept;
  bool operator!=(const DirectoryTopic &other) const noexcept;
};

}  // namespace dispatch
}  // namespace a17
