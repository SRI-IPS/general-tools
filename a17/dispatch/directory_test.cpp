#include "catch.hpp"

#include "a17/capnp_msgs/test.capnp.h"

#include "directory.h"
#include "message_helpers.h"

namespace a17 {
namespace dispatch {
namespace test {

const uint16_t TEST_PORT = 9999;
const std::string TEST_MULTICAST = "224.0.88.1";

class TestDirectory : public a17::dispatch::Directory {
 public:
  char command_;
  std::string args_;

  TestDirectory(boost::asio::io_service &ios)
      : a17::dispatch::Directory(ios, "test", TEST_PORT, TEST_MULTICAST) {}

  virtual void send(const boost::asio::ip::udp::endpoint &endpoint, const char command,
                    const std::string &args = "") {
    command_ = command;
    args_ = args;
  }
};

TEST_CASE("Query topic", "[directory]") {
  boost::asio::io_service ios1;
  a17::dispatch::Directory directory1(ios1, "test1", TEST_PORT, TEST_MULTICAST);
  std::thread t1([&directory1, &ios1]() {
    directory1.add("TEST/TOPIC", ZMQ_REP, a17::dispatch::Address(45454), {"17488419364809454706"},
                   {"17488419364809454706"}, directory1.guid());

    boost::asio::deadline_timer timer(ios1);
    timer.expires_from_now(boost::posix_time::seconds(1));
    timer.async_wait([&](const boost::system::error_code &ec) { ios1.stop(); });

    ios1.run();
  });

  boost::asio::io_service ios2;
  a17::dispatch::Directory directory2(ios2, "test2", TEST_PORT, TEST_MULTICAST);

  bool received = false;
  directory2.observe("TEST/TOPIC", [&ios2, &directory1, &received](
                                       const std::string &topic_name,
                                       const a17::dispatch::GuidTopicMap &guid_topic_map) {
    ios2.stop();
    REQUIRE(topic_name == "TEST/TOPIC");
    REQUIRE(guid_topic_map.count(directory1.guid()) == 1);
    auto topic = guid_topic_map.at(directory1.guid());
    REQUIRE(topic.name == "TEST/TOPIC");
    REQUIRE(topic.socketType == ZMQ_REP);
    REQUIRE(topic.inputTypes.count("17488419364809454706"));
    REQUIRE(topic.inputTypes.count("vv") == false);
    REQUIRE(topic.outputTypes.count("17488419364809454706"));
    REQUIRE(topic.outputTypes.count("ss") == false);
    received = true;
  });

  boost::asio::deadline_timer timer(ios2);
  timer.expires_from_now(boost::posix_time::seconds(5));
  timer.async_wait([&ios2](const boost::system::error_code &ec) {
    ios2.stop();
    FAIL("query response not received in 5 seconds");
  });

  ios2.run();

  CHECK(received);

  t1.join();
}

TEST_CASE("Discovery Search", "[directory]") {
  boost::asio::io_service ios;
  TestDirectory directory(ios);
  directory.add("TEST/TOPIC", ZMQ_PUB,
                "tcp://" + a17::dispatch::OwnAddress::instance().address() + ":40404",
                {a17::dispatch::typeOf<a17::capnp_msgs::test::TestType>()},
                {a17::dispatch::typeOf<a17::capnp_msgs::test::TestType>()}, directory.guid());

  char buf[a17::dispatch::EVENT_BUFFER_SIZE + 1];
  strcpy(buf, "DISPATCH some-other-guid S TEST/TOPIC");
  directory.handleEvent(buf);
  REQUIRE(directory.command_ == a17::dispatch::DISCOVERY_AVAILABLE);
  auto id = a17::dispatch::idOf<a17::capnp_msgs::test::TestType>();
  REQUIRE(directory.args_ ==
          "TEST/TOPIC PUB tcp://" + a17::dispatch::OwnAddress::instance().address() + ":40404 " +
              std::to_string(id) + " " + std::to_string(id));
}

TEST_CASE("Discovery Exit", "[directory]") {
  char buf[a17::dispatch::EVENT_BUFFER_SIZE + 1];

  boost::asio::io_service ios;
  a17::dispatch::Directory directory(ios, "test", TEST_PORT, TEST_MULTICAST);

  sprintf(buf, "DISPATCH some-other-guid A TEST/TOPIC PUB tcp://%s:40960 - 41",
          a17::dispatch::OwnAddress::instance().address().c_str());
  directory.handleEvent(buf);
  REQUIRE(directory.topics().size() == 1);

  strcpy(buf, "DISPATCH some-other-guid X");
  directory.handleEvent(buf);
  REQUIRE(directory.topics().size() == 0);
}

}  // namespace test
}  // namespace dispatch
}  // namespace a17
