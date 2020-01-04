#include "directory_topic.h"

namespace a17 {
namespace dispatch {

std::string DirectoryTopic::to_string() const noexcept {
  std::ostringstream output_stream;
  output_stream << "{\n"
                << "  name: " << name << ",\n"
                << "  timestamp: " << timestamp << ",\n"
                << "  socketType: " << socketType << ",\n"
                << "  inputTypes: [\n";
  for (const auto input_type : inputTypes) {
    output_stream << "    " << input_type << ",\n";
  }
  output_stream << "  ],\n"
                << "  outputTypes: [\n";
  for (const auto output_type : outputTypes) {
    output_stream << "    " << output_type << ",\n";
  }
  output_stream << "  ],\n"
                << "  guid: " << guid << ",\n"
                << "  info: " << info << ",\n"
                << "}";
  return output_stream.str();
}

std::ostream &operator<<(std::ostream &out, const DirectoryTopic &obj) {
  return out << obj.to_string();
}

bool DirectoryTopic::operator==(const DirectoryTopic &other) const noexcept {
  return name == other.name && timestamp == other.timestamp && socketType == other.socketType &&
         address == other.address && inputTypes == other.inputTypes &&
         outputTypes == other.outputTypes && guid == other.guid && info == other.info;
}

bool DirectoryTopic::operator!=(const DirectoryTopic &other) const noexcept {
  return !(*this == other);
}

}  // namespace dispatch
}  // namespace a17
