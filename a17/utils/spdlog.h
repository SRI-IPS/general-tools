#ifndef A17_UTILS_SPDLOG_UTILS_H
#define A17_UTILS_SPDLOG_UTILS_H

#include <spdlog/spdlog.h>

namespace a17 {
namespace utils {

/// Parses the given string into an spdlog level.
/// Valid levels are: {"trace", "debug", "info",  "warning", "error", "critical", "off"}
/// The corresponding short names can also be used: { "T", "D", "I", "W", "E", "C", "O" }
/// @throws std::invalid_argument if the string does not correspond to a spdlog level.
spdlog::level::level_enum SpdlogLevelFromString(const std::string &log_level_str);

}  // namespace utils
}  // namespace a17

#endif  // A17_UTILS_SPDLOG_UTILS_H
