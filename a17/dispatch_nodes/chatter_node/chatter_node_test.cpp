#include "catch.hpp"

#include <thread>

#include "a17/capnp_msgs/dispatch_nodes/chatter.capnp.h"

#include "a17/dispatch/directory.h"
#include "a17/dispatch/message_helpers.h"
#include "a17/dispatch/publisher.h"
#include "a17/dispatch/smart_capnp_builder.h"
#include "a17/dispatch/smart_capnp_reader.h"
#include "a17/dispatch/subscriber.h"

namespace a17 {
namespace dispatch_nodes {
namespace test {

using a17::capnp_msgs::dispatch_nodes::chatter::Chatter;

const uint16_t TEST_PORT = 9998;
const std::string TEST_MULTICAST = "224.0.88.1";

TEST_CASE("Chatter build and read", "[chatter_node]") {
  a17::utils::BufferPool pool;
  a17::dispatch::SmartCapnpBuilder builder(pool);
  auto msg = builder.initRoot<Chatter>();

  uint64_t timestamp = 1524191056000000;
  msg.setTimestamp(timestamp);
  msg.setSender("talker");
  msg.setMessage("Hello world #1");

  azmq::message message = builder.build();
  a17::dispatch::SmartCapnpReader reader(message, a17::dispatch::idOf<Chatter>());
  auto read_msg = reader.getRoot<Chatter>();
  REQUIRE(read_msg.getTimestamp() == timestamp);
  REQUIRE(!strcmp(read_msg.getSender().cStr(), "talker"));
  REQUIRE(!strcmp(read_msg.getMessage().cStr(), "Hello world #1"));
}

TEST_CASE("Talker listener pubsub", "[chatter_node]") {
  std::thread talker([]() {
    boost::asio::io_service ios;
    a17::utils::BufferPool pool;

    a17::dispatch::Directory directory(ios, "talker", TEST_PORT, TEST_MULTICAST);
    a17::dispatch::Publisher pub(ios, directory, "CHATTER",
                                 {a17::dispatch::typeOf<Chatter>()});

    boost::asio::deadline_timer timer(ios);
    timer.expires_from_now(boost::posix_time::milliseconds(500));
    timer.async_wait([&](const boost::system::error_code &ec) {
      CHECK(!ec);

      a17::dispatch::SmartCapnpBuilder builder(pool);
      auto msg = builder.initRoot<Chatter>();
      msg.setTimestamp(1524191056000000);
      msg.setSender("talker");
      msg.setMessage("Hello world #1");

      std::cout << "PUBLISHING" << std::endl;
      auto send_ec = pub.send(builder.getSmartMessage());
      CHECK(!send_ec);

      timer.expires_from_now(boost::posix_time::milliseconds(500));
      timer.async_wait([&](const boost::system::error_code &) { ios.stop(); });
    });

    ios.run();
  });

  boost::asio::io_service ios;
  a17::dispatch::Directory directory(ios, "listener", TEST_PORT, TEST_MULTICAST);

  uint8_t expect = 3;

  auto callback = [&](azmq::message_vector &msg_vec) {
    std::cout << "RECEIVING" << std::endl;
    if (--expect == 0) {
      ios.stop();
    }
    a17::dispatch::SmartCapnpReader reader(msg_vec);
    auto msg = reader.getRoot<Chatter>();
    CHECK(msg.getTimestamp() == 1524191056000000uLL);
    CHECK(!strcmp(msg.getSender().cStr(), "talker"));
    CHECK(!strcmp(msg.getMessage().cStr(), "Hello world #1"));
  };

  a17::dispatch::Subscriber sub1(ios, directory, "CHATTER", callback);
  a17::dispatch::Subscriber sub2(ios, directory, "CHATTER", callback);
  a17::dispatch::Subscriber sub3(ios, directory, "CHATTER", callback);
  ios.run();

  CHECK(expect == 0);

  talker.join();
}

}  // namespace test
}  // namespace dispatch_nodes
}  // namespace a17
