#include "rate_measure.h"

#include <cmath>

#include "nearest_interval_bin.h"

namespace a17 {
namespace utils {

RateMeasure::RateMeasure(std::chrono::steady_clock::duration sample_window)
    : sample_window_(sample_window.count()) {}

void RateMeasure::markSize(size_t size, int64_t now) {
  ticks_++;

  if (last_time_) {
    // interval
    auto interval = now - last_time_;
    intervals_.push(interval);
    interval_acc_ += interval;

    while (interval_acc_ > sample_window_) {
      interval_acc_ -= intervals_.front();
      intervals_.pop();
    }

    if (!intervals_.empty()) {
      avg_interval_ = interval_acc_ / intervals_.size();
      target_interval_ = NearestIntervalBin(avg_interval_, current_target_index_);
    }

    // jitter based on target interval
    auto jitter = std::fabs(interval - target_interval_);
    if (jitter > max_jitter_) max_jitter_ = jitter;
    jitters_.push(jitter);
    jitter_acc_ += jitter;

    while (jitters_.size() > intervals_.size()) {
      jitter_acc_ -= jitters_.front();
      jitters_.pop();
    }

    // size
    sizes_.push(size);
    size_acc_ += size;

    while (sizes_.size() > intervals_.size()) {
      size_acc_ -= sizes_.front();
      sizes_.pop();
    }
  }

  last_time_ = now;
}

void RateMeasure::reset() {
  last_time_ = 0;
  current_target_index_ = 0;
  target_interval_ = 0;
  avg_interval_ = 0;
  max_jitter_ = 0;
}

bool RateMeasure::isAtWindow() const {
  if (!target_interval_) return 0;
  return (ticks_ % (sample_window_ / target_interval_)) == 0;
}

}  // namespace utils
}  // namespace a17
