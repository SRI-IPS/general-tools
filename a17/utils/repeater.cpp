#include "repeater.h"

#include "bind.h"

namespace a17 {
namespace utils {

Repeater::Repeater(boost::asio::io_service &ios, std::chrono::steady_clock::duration interval,
                   RepeatOperation operation, bool auto_start)
    : timer_(ios, interval),
      interval_(interval),
      operation_(std::move(operation)),
      handler_(bind1(&Repeater::repeat)) {
  if (auto_start) start();
}

Repeater::Repeater(boost::asio::io_service &ios, int millis, RepeatOperation operation,
                   bool auto_start)
    : Repeater(ios, std::chrono::milliseconds(millis), operation, auto_start) {}

void Repeater::start() {
  if (!isRunning()) {
    timer_.expires_from_now(interval_);
    timer_.async_wait(handler_);
    is_running_ = true;
  }
}

void Repeater::stop() {
  boost::system::error_code ec;
  timer_.cancel(ec);
  is_running_ = false;
}

void Repeater::setInterval(std::chrono::steady_clock::duration interval) {
  auto oldInterval = interval_;
  interval_ = interval;
  if (isRunning()) {
    stop();
    start();
  }
}

void Repeater::repeat(const boost::system::error_code &ec) {
  if (!ec && isRunning()) {
    auto start = std::chrono::steady_clock::now();
    if (operation_()) {
      auto expired = timer_.expires_at();
      std::chrono::steady_clock::duration diff = start - expired;
      bool bump = diff > (interval_ / 2);
      auto from = bump ? start : expired;
      timer_.expires_at(from + interval_);
      timer_.async_wait(handler_);
    } else {
      is_running_ = false;
    }
  }
}

}  // namespace utils
}  // namespace a17
