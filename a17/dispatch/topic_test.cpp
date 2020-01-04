#include "catch.hpp"

#include "topic.h"

namespace a17 {
namespace dispatch {
namespace test {

TEST_CASE("Topic Test", "[dispatch]") {
  auto topic = Topic{"", "", ""};
  REQUIRE(topic.str() == "");
  topic = Topic{"device", "", ""};
  REQUIRE(topic.str() == "device");
  topic = Topic{"device", "node", ""};
  REQUIRE(topic.str() == "device/node");
  topic = Topic{"device", "node", "topic"};
  REQUIRE(topic.str() == "device/node/topic");
  topic = Topic{"", "node", "topic"};
  REQUIRE(topic.str() == "node/topic");
  topic = Topic{"", "", "topic"};
  REQUIRE(topic.str() == "topic");
  topic = Topic{"device/", "node", "topic"};
  REQUIRE(topic.str() == "device/node/topic");
  topic = Topic{"device/", "node/", "topic"};
  REQUIRE(topic.str() == "device/node/topic");
}

}  // namespace test
}  // namespace dispatch
}  // namespace a17
