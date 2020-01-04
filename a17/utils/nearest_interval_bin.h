#ifndef A17_UTILS_NEAREST_INTERVAL_BIN_H_
#define A17_UTILS_NEAREST_INTERVAL_BIN_H_

#include <cstdint>

namespace a17 {
namespace utils {

/// Returns the nearest bin to the given interval.
uint64_t NearestIntervalBin(uint64_t interval, uint8_t &index);

}  // namespace utils
}  // namespace a17

#endif  // A17_UTILS_NEAREST_INTERVAL_BIN_H_
