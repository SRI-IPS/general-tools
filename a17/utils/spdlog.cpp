#include "spdlog.h"
#include <spdlog/common.h>  // For SPDLOG_VERSION

namespace a17 {
namespace utils {

/// Parses the given string into an spdlog level.
/// Valid levels are: {"trace", "debug", "info",  "warning", "error", "critical", "off"}
/// The corresponding short names can also be used: { "T", "D", "I", "W", "E", "C", "O" }
/// @throws std::invalid_argument if the string does not correspond to a spdlog level.
spdlog::level::level_enum SpdlogLevelFromString(const std::string &log_level_str) {
#if SPDLOG_VERSION >= 10000
  // spdlog 1.x and later
  auto level = spdlog::level::from_str(log_level_str);
  // from_str returns 'off' for invalid input, but the original code threw an exception.
  // We check for 'off' and also if the input string was actually "off" to replicate behavior.
  if (level == spdlog::level::off && log_level_str != "off" && log_level_str != "O") {
    throw std::invalid_argument{"Invalid log level: " + log_level_str};
  }
  return level;
#else
  // spdlog 0.x
  // See: spd::level::level_enum.
  constexpr auto kNumSpdlogLevels = 7;
  for (int i = 0; i < kNumSpdlogLevels; i++) {
    if (!strncasecmp(&log_level_str[0], spdlog::level::level_names[i], log_level_str.size()) ||
        !strcasecmp(&log_level_str[0], spdlog::level::short_level_names[i])) {
      return static_cast<spdlog::level::level_enum>(i);
    }
  }
  throw std::invalid_argument{"Invalid log level: " + log_level_str};
#endif
}

}  // namespace utils
}  // namespace a17
