#include "catch.hpp"

#include "a17/capnp_msgs/test.capnp.h"

#include "directory.h"
#include "message_helpers.h"
#include "publisher.h"
#include "smart_capnp_builder.h"
#include "smart_capnp_reader.h"
#include "smart_message_reader.h"
#include "subscriber.h"

namespace a17 {
namespace dispatch {
namespace test {

const uint16_t TEST_PORT = 9999;
const std::string TEST_MULTICAST = "224.0.88.1";

TEST_CASE("Discovery", "[socket]") {
  std::thread t1([]() {
    boost::asio::io_service ios;
    a17::utils::BufferPool pool;

    a17::dispatch::Directory directory(ios, "test1", TEST_PORT, TEST_MULTICAST);
    a17::dispatch::Publisher pub(ios, directory, "TEST/PUB",
                                 {a17::dispatch::typeOf<a17::capnp_msgs::test::DispatchTest>()});

    boost::asio::deadline_timer timer(ios);
    timer.expires_from_now(boost::posix_time::milliseconds(500));
    timer.async_wait([&](const boost::system::error_code &ec) {
      CHECK(!ec);

      a17::dispatch::SmartCapnpBuilder builder(pool);
      auto log = builder.initRoot<a17::capnp_msgs::test::DispatchTest>();
      log.setAddress("localhost");
      log.setTopic("QUEUE");
      log.setSocketType("PUSH");
      log.setEvent(a17::capnp_msgs::test::DispatchTest::Event::RECEIVE);

      std::cout << "PUBLISHING" << std::endl;
      auto ec2 = pub.send(builder.getSmartMessage());
      CHECK(!ec2);

      timer.expires_from_now(boost::posix_time::milliseconds(500));
      timer.async_wait([&](const boost::system::error_code &) { ios.stop(); });
    });

    ios.run();
  });

  boost::asio::io_service ios;
  a17::dispatch::Directory directory(ios, "test2", TEST_PORT, TEST_MULTICAST);

  uint8_t expect = 3;

  auto callback = [&](azmq::message_vector &msg_vec) {
    std::cout << "RECEIVING" << std::endl;
    if (--expect == 0) {
      ios.stop();
    }
    auto reader = a17::dispatch::SmartCapnpReader(msg_vec);
    auto log = reader.getRoot<a17::capnp_msgs::test::DispatchTest>();
    CHECK(!strcmp(log.getAddress().cStr(), "localhost"));
    CHECK(!strcmp(log.getTopic().cStr(), "QUEUE"));
    CHECK(!strcmp(log.getSocketType().cStr(), "PUSH"));
    CHECK(log.getEvent() == a17::capnp_msgs::test::DispatchTest::Event::RECEIVE);
  };

  a17::dispatch::Subscriber sub1(ios, directory, "TEST/PUB", callback);
  a17::dispatch::Subscriber sub2(ios, directory, "TEST/PUB", callback);
  a17::dispatch::Subscriber sub3(ios, directory, "TEST/PUB", callback);
  ios.run();

  CHECK(expect == 0);

  t1.join();
}

TEST_CASE("IPC", "[socket]") {
  a17::dispatch::Address address("ipc", "/tmp/dispatch_ipc_test");
  a17::utils::BufferPool pool;

  std::thread t1([&]() {
    boost::asio::io_service ios;
    a17::dispatch::Publisher pub(ios, address);

    boost::asio::deadline_timer timer(ios);
    timer.expires_from_now(boost::posix_time::milliseconds(500));
    timer.async_wait([&](const boost::system::error_code &ec) {
      CHECK(!ec);

      a17::dispatch::SmartCapnpBuilder builder(pool);
      auto log = builder.initRoot<a17::capnp_msgs::test::DispatchTest>();
      log.setAddress("localhost");
      log.setTopic("QUEUE");
      log.setSocketType("PUSH");
      log.setEvent(a17::capnp_msgs::test::DispatchTest::Event::RECEIVE);

      boost::system::error_code ec2;
      pub.send(builder, ec2);
      CHECK(!ec2);

      ios.stop();
    });

    ios.run();
  });

  boost::asio::io_service ios;
  a17::dispatch::Subscriber sub(ios, address, [&](azmq::message_vector &msg_vec) {
    ios.stop();
    auto reader = a17::dispatch::SmartCapnpReader(msg_vec);
    auto log = reader.getRoot<a17::capnp_msgs::test::DispatchTest>();
    CHECK(!strcmp(log.getAddress().cStr(), "localhost"));
    CHECK(!strcmp(log.getTopic().cStr(), "QUEUE"));
    CHECK(!strcmp(log.getSocketType().cStr(), "PUSH"));
    CHECK(log.getEvent() == a17::capnp_msgs::test::DispatchTest::Event::RECEIVE);
  });
  ios.run();

  t1.join();
}

TEST_CASE("Inproc", "[socket]") {
  a17::dispatch::Address address("inproc", "test_pub");
  a17::utils::BufferPool pool;

  std::thread t1([&]() {
    boost::asio::io_service ios;
    a17::dispatch::Publisher pub(ios, address);

    boost::asio::deadline_timer timer(ios);
    timer.expires_from_now(boost::posix_time::milliseconds(500));
    timer.async_wait([&](const boost::system::error_code &ec) {
      CHECK(!ec);

      a17::dispatch::SmartCapnpBuilder builder(pool);
      auto log = builder.initRoot<a17::capnp_msgs::test::DispatchTest>();
      log.setAddress("localhost");
      log.setTopic("QUEUE");
      log.setSocketType("PUSH");
      log.setEvent(a17::capnp_msgs::test::DispatchTest::Event::RECEIVE);

      auto ec2 = pub.send(builder.getSmartMessage());
      CHECK(!ec2);

      ios.stop();
    });

    ios.run();
  });

  boost::asio::io_service ios;
  a17::dispatch::Subscriber sub(ios, address, [&](azmq::message_vector &msg_vec) {
    ios.stop();
    auto reader = a17::dispatch::SmartCapnpReader(msg_vec);
    auto log = reader.getRoot<a17::capnp_msgs::test::DispatchTest>();
    CHECK(!strcmp(log.getAddress().cStr(), "localhost"));
    CHECK(!strcmp(log.getTopic().cStr(), "QUEUE"));
    CHECK(!strcmp(log.getSocketType().cStr(), "PUSH"));
    CHECK(log.getEvent() == a17::capnp_msgs::test::DispatchTest::Event::RECEIVE);
  });
  ios.run();

  t1.join();
}

}  // namespace test
}  // namespace dispatch
}  // namespace a17
