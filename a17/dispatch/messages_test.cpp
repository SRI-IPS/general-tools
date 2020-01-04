#include "catch.hpp"

#include "a17/capnp_msgs/test.capnp.h"

#include "message_helpers.h"
#include "smart_capnp_builder.h"
#include "smart_capnp_reader.h"
#include "smart_message_reader.h"

namespace a17 {
namespace dispatch {
namespace test {

TEST_CASE("Builder and reader", "[message]") {
  a17::utils::BufferPool pool;
  a17::dispatch::SmartCapnpBuilder builder(pool);
  auto test_msg = builder.initRoot<a17::capnp_msgs::test::DispatchTest>();

  test_msg.setAddress("localhost");
  test_msg.setTopic("QUEUE");
  test_msg.setSocketType("PUSH");
  test_msg.setEvent(a17::capnp_msgs::test::DispatchTest::Event::RECEIVE);

  azmq::message message = builder.build();
  a17::dispatch::SmartCapnpReader reader(
      message, a17::dispatch::idOf<a17::capnp_msgs::test::DispatchTest>());
  auto test_reader = reader.getRoot<a17::capnp_msgs::test::DispatchTest>();
  REQUIRE(!strcmp(test_reader.getAddress().cStr(), test_msg.getAddress().cStr()));
  REQUIRE(!strcmp(test_reader.getTopic().cStr(), test_msg.getTopic().cStr()));
  REQUIRE(!strcmp(test_reader.getSocketType().cStr(), test_msg.getSocketType().cStr()));
  REQUIRE(test_reader.getEvent() == test_msg.getEvent());
}

TEST_CASE("Builder with buffer pool", "[message]") {
  a17::utils::BufferPool pool(64, 1);
  REQUIRE(!pool.empty());

  a17::dispatch::SmartCapnpBuilder builder(pool);
  builder.initRoot<a17::capnp_msgs::test::DispatchTest>();
  REQUIRE(pool.empty());
}

TEST_CASE("idOf", "[message]") {
  auto id = a17::dispatch::idOf<a17::capnp_msgs::test::DispatchTest>();
  REQUIRE(id == 11643037877147589208uLL);
}

TEST_CASE("typeOf", "[message]") {
  auto id = a17::dispatch::idOf<a17::capnp_msgs::test::DispatchTest>();
  auto id_str = a17::dispatch::typeOf<a17::capnp_msgs::test::DispatchTest>();
  REQUIRE(std::to_string(id) == id_str);
}

TEST_CASE("idFromSmartMessage", "[message]") {
  a17::utils::BufferPool pool;
  a17::dispatch::SmartCapnpBuilder builder(pool);
  builder.initRoot<a17::capnp_msgs::test::DispatchTest>();
  azmq::message_vector smart_msg = builder.getSmartMessage();
  REQUIRE(a17::dispatch::idFromSmartMessage(smart_msg) == 11643037877147589208uLL);
}

}  // namespace test
}  // namespace dispatch
}  // namespace a17
