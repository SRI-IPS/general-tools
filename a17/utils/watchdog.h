#ifndef A17_UTILS_WATCHDOG_H_
#define A17_UTILS_WATCHDOG_H_

#include <chrono>

#include <boost/asio.hpp>
#include <boost/asio/basic_deadline_timer.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

namespace a17 {
namespace utils {

/// If the mark() function is not called within the given timeout, then the callback will be called.
class Watchdog {
 public:
  Watchdog(boost::asio::io_service &ios, boost::posix_time::milliseconds timeout_ms,
           std::function<void()> callback);
  void mark();

 private:
  std::function<void()> callback_;
  boost::asio::deadline_timer timer_;
  boost::posix_time::milliseconds timeout_;
};

}  // namespace utils
}  // namespace a17

#endif  // A17_UTILS_WATCHDOG_H_
