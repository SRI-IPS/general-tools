#include "nearest_interval_bin.h"

#include <cmath>
#include <limits>

namespace a17 {
namespace utils {

namespace {

constexpr int kIntervalBinsSize = 10;
constexpr double kIntervalBins[] {
    1,         // 10 Hz
    1 / 0.75,  // 7.5 Hz
    1 / 0.6,   // 6 Hz
    2,         // 5 Hz
    2.5,       // 4 Hz
    3,         // 3.33 Hz
    4,         // 2.5 Hz
    5,         // 2 Hz
    1 / 0.15,  // 1.5 Hz
    10         // 1 Hz
};

constexpr uint64_t kTenToThe[] = {1,
                                  10,
                                  100,
                                  1000,
                                  10000,
                                  100000,
                                  1000000,
                                  10000000,
                                  100000000,
                                  1000000000,
                                  10000000000L,
                                  100000000000L,
                                  1000000000000L,
                                  10000000000000L,
                                  100000000000000L,
                                  1000000000000000L,
                                  10000000000000000L,
                                  100000000000000000L,
                                  1000000000000000000L,
                                  UINT64_MAX};

unsigned int baseTwoDigits(uint64_t x) {
  uint32_t hi = x >> 32;
  uint32_t lo = x & UINT32_MAX;
  if (hi == 0) {
    return lo ? 32 - __builtin_clz(lo) : 0;
  } else {
    return 64 - __builtin_clz(hi);
  }
}

int baseTenDigits(uint64_t x) {
  int guess[65] = {0,  0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,
                   5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9,  9,
                   10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15,
                   15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 18, 19};
  int digits = guess[baseTwoDigits(x)];
  return digits + (x >= kTenToThe[digits]);
}

}  // namespace

uint64_t NearestIntervalBin(uint64_t interval, uint8_t &index) {
  if (!interval) return 0;

  // divide by the base 10 exponent first, then find bin
  auto digits = baseTenDigits(interval);
  auto exp = kTenToThe[digits - 1];
  double x = interval / (double)exp;

  // this little joint optimizes the search for the interval bin
  // by starting from the previously chosen bin and traversing in
  // the direction of smaller differences.
  double diff = std::fabs(x - kIntervalBins[index]);

  double diff_left =
      index > 0 ? std::fabs(x - kIntervalBins[index - 1]) : std::numeric_limits<double>::max();

  double diff_right = index < kIntervalBinsSize - 1 ? std::fabs(x - kIntervalBins[index + 1])
                                                    : std::numeric_limits<double>::max();

  if (diff_left < diff) {
    while (diff_left < diff && index > 0) {
      index--;
      diff = diff_left;
      diff_left = std::fabs(x - kIntervalBins[index - 1]);
    }
  } else if (diff_right < diff) {
    while (diff_right < diff && index < kIntervalBinsSize - 1) {
      index++;
      diff = diff_right;
      diff_right = std::fabs(x - kIntervalBins[index + 1]);
    }
  }

  return static_cast<uint64_t>(kIntervalBins[index] * exp);
}

}  // namespace utils
}  // namespace a17
