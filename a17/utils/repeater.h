#pragma once

#include <boost/asio/basic_waitable_timer.hpp>
// Needed for boost 1.66+
#include <boost/asio/io_service.hpp> 
#include <chrono>

namespace a17 {
namespace utils {

using RepeatOperation = std::function<bool()>;

/// Repeatedly calls a given callback operation on a fixed time interval.
class Repeater {
 public:
  Repeater(boost::asio::io_service &ios, int millis, RepeatOperation operation,
           bool auto_start = true);
  Repeater(boost::asio::io_service &ios, std::chrono::steady_clock::duration interval,
           RepeatOperation operation, bool auto_start = true);

  void start();
  void stop();

  inline bool isRunning() { return is_running_; }

  inline std::chrono::steady_clock::duration getInterval() { return interval_; }
  void setInterval(std::chrono::steady_clock::duration interval);
  inline void setInterval(int millis) { setInterval(std::chrono::milliseconds(millis)); };

 protected:
  void repeat(const boost::system::error_code &ec);

 private:
  bool is_running_ = false;
  boost::asio::basic_waitable_timer<std::chrono::steady_clock> timer_;
  std::chrono::steady_clock::duration interval_;

  const RepeatOperation operation_;
  const std::function<void(const boost::system::error_code &)> handler_;
};

}  // namespace utils
}  // namespace a17
