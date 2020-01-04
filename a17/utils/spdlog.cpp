#include "spdlog.h"

namespace a17 {
namespace utils {

/// Parses the given string into an spdlog level.
/// Valid levels are: {"trace", "debug", "info",  "warning", "error", "critical", "off"}
/// The corresponding short names can also be used: { "T", "D", "I", "W", "E", "C", "O" }
/// @throws std::invalid_argument if the string does not correspond to a spdlog level.
spdlog::level::level_enum SpdlogLevelFromString(const std::string &log_level_str) {
  // See: spd::level::level_enum.
  constexpr auto kNumSpdlogLevels = 7;
  for (int i = 0; i < kNumSpdlogLevels; i++) {
    if (!strncasecmp(&log_level_str[0], spdlog::level::level_names[i], log_level_str.size()) ||
        !strcasecmp(&log_level_str[0], spdlog::level::short_level_names[i])) {
      return static_cast<spdlog::level::level_enum>(i);
    }
  }
  throw std::invalid_argument{"Invalid log level: " + log_level_str};
}

}  // namespace utils
}  // namespace a17
