#include "catch.hpp"
#include "watchdog.h"
#include "repeater.h"
#include "buffer_pool.h"
#include <boost/asio.hpp>

TEST_CASE("watchdog timer test", "[watchdog]") {
  boost::asio::io_service ios;
  a17::utils::BufferPool pool;
  auto wd_fired = false;
  auto timeoutCallback = [&]() {
    wd_fired = true;
    ios.stop();
  };
  a17::utils::Watchdog wd(ios, boost::posix_time::milliseconds(30), timeoutCallback);
  // every 200ms, mark the watchdog
  a17::utils::Repeater rep(ios, std::chrono::milliseconds(10), [&]() -> bool {
    wd.mark();
    return true;
  });
  // stop the repeater after 1 second
  boost::asio::deadline_timer timer(ios);
  timer.expires_from_now(boost::posix_time::milliseconds(100));
  timer.async_wait([&](const boost::system::error_code &) { rep.stop(); });

  ios.run();
  REQUIRE(wd_fired);
}
