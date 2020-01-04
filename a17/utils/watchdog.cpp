#include "watchdog.h"

namespace a17 {
namespace utils {

Watchdog::Watchdog(boost::asio::io_service &ios, boost::posix_time::milliseconds timeout_ms,
                   std::function<void()> callback)
    : callback_(std::move(callback)), timer_(ios, timeout_ms), timeout_(timeout_ms) {}

void Watchdog::mark() {
  timer_.expires_from_now(timeout_);
  timer_.async_wait([this](const boost::system::error_code &ec) {
    if (ec != boost::asio::error::operation_aborted) {
      callback_();
    }
  });
}

}  // namespace utils
}  // namespace a17
