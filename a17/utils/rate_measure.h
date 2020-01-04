#ifndef A17_UTILS_RATE_MEASURE_H_
#define A17_UTILS_RATE_MEASURE_H_

#include <queue>
#include <chrono>

namespace a17 {
namespace utils {

// Measure average frequency and jitter over time samples.
class RateMeasure {
 public:
  RateMeasure(std::chrono::steady_clock::duration sample_window = std::chrono::seconds(2));

  void reset();
  inline void resetMaxJitter() { max_jitter_ = 0; }

  // Mark next sample at the current moment or an optional supplied value. (Supplied value must be
  // nanoseconds to produce frequency in Hz and jitter in ms.)
  // @param size Size measurement
  // @param now Current time index in nano seconds
  void markSize(size_t size, int64_t now);

  inline void markSize(
      size_t size, std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) {
    markSize(size, now.time_since_epoch().count());
  }

  void mark(int64_t now) { markSize(0, now); }

  inline void mark(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) {
    mark(now.time_since_epoch().count());
  }

  /// Number of samples.
  inline size_t sampleCount() { return intervals_.size(); }
  /// Running average size in bytes.
  inline size_t size() const { return !sizes_.empty() ? size_acc_ / sizes_.size() : 0; }
  /// Running average frequency in Hz.
  inline double frequency() const { return avg_interval_ ? 1000000000.0 / avg_interval_ : 0; }
  /// Closest target frequency
  inline int64_t targetFrequency() const {
    return target_interval_ ? 1000000000 / target_interval_ : 0;
  }
  /// Closest target interval.
  inline int64_t targetInterval() const { return target_interval_; }
  /// Running average jitter in μs.
  inline double jitter() const {
    return !jitters_.empty() ? ((double)jitter_acc_) / jitters_.size() / 1000.0 : 0;
  }
  /// Percent jitter in target interval.
  inline double jitterPercent() const {
    return target_interval_ ? 100 * (jitter() * 1000) / target_interval_ : 0;
  }
  /// Resets max jitter and return previous max.
  double maxJitter() const { return max_jitter_ / 1000.0; }

  inline uint64_t ticks() const { return ticks_; }
  bool isAtWindow() const;

 private:
  int64_t sample_window_ = 0;
  int64_t target_interval_ = 0;
  uint8_t current_target_index_ = 0;

  std::queue<int64_t> intervals_;
  std::queue<int64_t> jitters_;
  std::queue<size_t> sizes_;
  uint64_t ticks_ = 0;

  int64_t last_time_ = 0;
  int64_t interval_acc_ = 0;
  int64_t jitter_acc_ = 0;
  size_t size_acc_ = 0;

  int64_t avg_interval_ = 0;
  int64_t max_jitter_ = 0;
};

}  // namespace utils
}  // namespace a17

#endif  // A17_UTILS_RATE_MEASURE_H_
