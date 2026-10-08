#include "catch.hpp"

#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "a17/capnp_msgs/dispatch_nodes/chatter.capnp.h"

#include "a17/dispatch/node.h"

namespace a17 {
namespace dispatch_nodes {
namespace test {

using a17::capnp_msgs::dispatch_nodes::chatter::Chatter;

TEST_CASE("Topic construction", "[chatter_node]") {
  // Remove any device name configured on the host so the expected string is deterministic.
  unsetenv("A17_DEVICE_NAME");

  a17::dispatch::Node node("TALKER");
  REQUIRE(node.topic("CHATTER").str() == "TALKER/CHATTER");
}

TEST_CASE("Talker and listener nodes exchange messages", "[chatter_node]") {
  a17::dispatch::Node talker("TALKER");
  a17::dispatch::Node listener("LISTENER");

  struct ReceivedMessage {
    uint64_t timestamp;
    std::string sender;
    std::string message;
  };

  std::mutex mutex;
  std::condition_variable cv;
  std::vector<ReceivedMessage> received;

  constexpr size_t kExpectedMessages = 3;

  auto sub = listener.registerCapnpSubscriber<Chatter>(
      talker.topic("CHATTER"),
      [&](const Chatter::Reader &msg) {
        ReceivedMessage entry;
        entry.timestamp = msg.getTimestamp();
        entry.sender = msg.getSender().cStr();
        entry.message = msg.getMessage().cStr();
        {
          std::lock_guard<std::mutex> lock(mutex);
          received.push_back(std::move(entry));
        }
        cv.notify_all();
      });
  (void)sub;

  auto pub = talker.registerCapnpPublisher<Chatter>(talker.topic("CHATTER"));
  uint64_t count = 0;
  auto repeater = talker.registerRepeater(50, [&, pub]() -> bool {
    count++;
    auto builder = talker.newCapnpMessageBuilder();
    auto msg = builder.initRoot<Chatter>();
    msg.setTimestamp(count);
    msg.setSender("TALKER");
    msg.setMessage(("Hello world #" + std::to_string(count)).c_str());
    pub->send(builder);
    return true;
  });
  (void)repeater;

  talker.start();
  listener.start();

  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(10),
                [&] { return received.size() >= kExpectedMessages; });
  }

  talker.stop();
  listener.stop();

  std::vector<ReceivedMessage> snapshot;
  {
    std::lock_guard<std::mutex> lock(mutex);
    snapshot = received;
  }

  REQUIRE(snapshot.size() >= kExpectedMessages);
  for (size_t i = 0; i < kExpectedMessages; i++) {
    REQUIRE(snapshot[i].sender == "TALKER");
    REQUIRE(snapshot[i].message == "Hello world #" + std::to_string(i + 1));
  }
  REQUIRE(snapshot[0].timestamp == 1);
}

}  // namespace test
}  // namespace dispatch_nodes
}  // namespace a17