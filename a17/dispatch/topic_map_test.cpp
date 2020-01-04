#include "catch.hpp"

#include "directory.h"

namespace a17 {
namespace dispatch {
namespace test {

TEST_CASE("Topic Map Test", "dispatch") {
  constexpr auto TIMESTAMP = static_cast<long int>(10);

  const auto topic1 = a17::dispatch::DirectoryTopic{"TEST/1",
                                                    TIMESTAMP,
                                                    ZMQ_REP,
                                                    "addr://test1",
                                                    std::set<a17::dispatch::message_type>{"x"},
                                                    std::set<a17::dispatch::message_type>{"z"},
                                                    "1001"};

  const auto topic2 = a17::dispatch::DirectoryTopic{"TEST/2",
                                                    TIMESTAMP,
                                                    ZMQ_PUB,
                                                    "addr://test2",
                                                    std::set<a17::dispatch::message_type>{},
                                                    std::set<a17::dispatch::message_type>{"w"},
                                                    "1002"};

  const auto topic2a = a17::dispatch::DirectoryTopic{"TEST/2",
                                                     TIMESTAMP,
                                                     ZMQ_PULL,
                                                     "addr://test2a",
                                                     std::set<a17::dispatch::message_type>{"x"},
                                                     std::set<a17::dispatch::message_type>{},
                                                     "1002a"};

  const auto topic2alt = a17::dispatch::DirectoryTopic{"TEST/2",
                                                       TIMESTAMP,
                                                       ZMQ_PUB,
                                                       "addr://test2alt",
                                                       std::set<a17::dispatch::message_type>{},
                                                       std::set<a17::dispatch::message_type>{"w"},
                                                       "1002"};

  const auto topic3 = a17::dispatch::DirectoryTopic{"TEST/3",
                                                    TIMESTAMP,
                                                    ZMQ_REP,
                                                    "addr://test1",
                                                    std::set<a17::dispatch::message_type>{"x"},
                                                    std::set<a17::dispatch::message_type>{"z"},
                                                    "1001"};

  SECTION("Local topic") {
    a17::dispatch::DirectoryTopicStore topic_store(topic1.guid);

    topic_store.add(topic1);
    topic_store.add(topic3);

    REQUIRE(topic_store.hasTopic(topic1.name));
    REQUIRE(topic_store.hasTopic(topic3.name));
    REQUIRE(!topic_store.hasTopic(topic2.name));
    REQUIRE(topic_store.size() == 2);
    REQUIRE(topic_store.localSize() == 2);
    REQUIRE(topic_store.beginLocal() != topic_store.endLocal());

    // Shouldn't remove since guid doesn't match.
    topic_store.remove(topic2.guid, topic1.name);
    REQUIRE(topic_store.hasTopic(topic1.name));

    // Shouldn't remove since name doesn't match.
    topic_store.remove(topic1.guid, topic2.name);
    REQUIRE(topic_store.hasTopic(topic1.name));

    // This should remove.
    topic_store.remove(topic1.guid, topic1.name);
    topic_store.remove(topic3.guid, topic3.name);
    REQUIRE(!topic_store.hasTopic(topic1.name));
    REQUIRE(topic_store.size() == 0);
    REQUIRE(topic_store.beginLocal() == topic_store.endLocal());
  }

  SECTION("Network topic") {
    a17::dispatch::DirectoryTopicStore topic_store(topic1.guid);
    auto expected_guid_topic_map = a17::dispatch::GuidTopicMap{};
    auto events = std::vector<std::pair<std::string, a17::dispatch::GuidTopicMap>>{};
    auto callback = [&events](const std::string &topic_name,
                              const a17::dispatch::GuidTopicMap &guid_topic_map) {
      events.push_back({topic_name, guid_topic_map});
    };

    topic_store.observe(topic2.name, callback);

    topic_store.add(topic2);
    expected_guid_topic_map.insert({topic2.guid, topic2});
    REQUIRE(events.size() == 1);
    REQUIRE(events[0].first == topic2.name);
    REQUIRE(events[0].second == expected_guid_topic_map);
    REQUIRE(topic_store.beginLocal() == topic_store.endLocal());

    topic_store.add(topic1);
    topic_store.add(topic2a);
    expected_guid_topic_map.insert({topic2a.guid, topic2a});
    REQUIRE(events.size() == 2);
    REQUIRE(events[1].first == topic2.name);
    REQUIRE(events[1].second == expected_guid_topic_map);
    REQUIRE(topic_store.hasTopic(topic1.name));
    REQUIRE(topic_store.hasTopic(topic2.name));
    REQUIRE(topic_store.size() == 2);

    // topic2a and topic2 share the same topic name
    topic_store.remove(topic2a.guid, topic2a.name);
    expected_guid_topic_map.clear();
    expected_guid_topic_map.insert({topic2.guid, topic2});
    REQUIRE(events.size() == 3);
    REQUIRE(events[2].first == topic2.name);
    REQUIRE(events[2].second == expected_guid_topic_map);
    REQUIRE(topic_store.hasTopic(topic1.name));
    REQUIRE(topic_store.hasTopic(topic2.name));
    REQUIRE(topic_store.size() == 2);

    // should replace existing entry for topic2.name
    topic_store.add(topic2alt);
    expected_guid_topic_map.clear();
    expected_guid_topic_map.insert({topic2alt.guid, topic2alt});
    REQUIRE(events.size() == 4);
    REQUIRE(events[3].first == topic2.name);
    REQUIRE(events[3].second == expected_guid_topic_map);
    REQUIRE(topic_store.size() == 2);
  }

  SECTION("Evict") {
    a17::dispatch::DirectoryTopicStore topic_store(topic2.guid);
    auto expected_guid_topic_map = a17::dispatch::GuidTopicMap{};
    auto events = std::vector<std::pair<std::string, a17::dispatch::GuidTopicMap>>{};
    auto callback = [&events](const std::string &topic_name,
                              const a17::dispatch::GuidTopicMap &guid_topic_map) {
      events.push_back({topic_name, guid_topic_map});
    };
    topic_store.observe(topic1.name, callback);
    topic_store.observe(topic3.name, callback);

    topic_store.add(topic1);
    topic_store.add(topic2);
    topic_store.add(topic2a);
    topic_store.add(topic3);
    REQUIRE(events.size() == 2);
    REQUIRE(events[0].first == topic1.name);
    expected_guid_topic_map.clear();
    expected_guid_topic_map.insert({topic1.guid, topic1});
    REQUIRE(events[0].second == expected_guid_topic_map);
    REQUIRE(events[1].first == topic3.name);
    expected_guid_topic_map.clear();
    expected_guid_topic_map.insert({topic3.guid, topic3});
    REQUIRE(events[1].second == expected_guid_topic_map);
    REQUIRE(topic_store.size() == 3);

    // topic1 and topic3 have the same guid, so they should both be evicted.
    topic_store.evict(topic1.guid);
    expected_guid_topic_map.clear();
    REQUIRE(topic_store.size() == 1);
    REQUIRE(events.size() == 4);
    REQUIRE(events[2].first == topic1.name);
    REQUIRE(events[2].second == expected_guid_topic_map);
    REQUIRE(events[3].first == topic3.name);
    REQUIRE(events[3].second == expected_guid_topic_map);
  }

  SECTION("Unobserve") {
    a17::dispatch::DirectoryTopicStore topic_store(topic1.guid);
    auto expected_guid_topic_map = a17::dispatch::GuidTopicMap{};
    auto events = std::vector<std::pair<std::string, a17::dispatch::GuidTopicMap>>{};
    auto callback = [&events](const std::string &topic_name,
                              const a17::dispatch::GuidTopicMap &guid_topic_map) {
      events.push_back({topic_name, guid_topic_map});
    };
    auto ref = topic_store.observe(topic2.name, callback);

    // Make sure add/remove/add calls callback.
    topic_store.add(topic2);
    REQUIRE(events.size() == 1);
    REQUIRE(events[0].first == topic2.name);
    expected_guid_topic_map.insert({topic2.guid, topic2});
    REQUIRE(events[0].second == expected_guid_topic_map);

    // Shouldn't do anything.
    topic_store.unobserve(topic2.name, "notourref");

    topic_store.remove(topic2.guid, topic2.name);
    REQUIRE(events.size() == 2);
    REQUIRE(events[1].first == topic2.name);
    expected_guid_topic_map.clear();
    REQUIRE(events[1].second == expected_guid_topic_map);

    topic_store.add(topic2);
    REQUIRE(events.size() == 3);
    REQUIRE(events[2].first == topic2.name);
    expected_guid_topic_map.insert({topic2.guid, topic2});
    REQUIRE(events[2].second == expected_guid_topic_map);

    // Now test unobserve.
    topic_store.unobserve(topic2.name, ref);

    // Shouldn't do anything since we unobserved the topic.
    topic_store.remove(topic2.guid, topic2.name);
    REQUIRE(events.size() == 3);
  }

  SECTION("Look up missing") {
    a17::dispatch::DirectoryTopicStore topic_store(topic1.guid);
    auto expected_guid_topic_map = a17::dispatch::GuidTopicMap{};
    auto callback = [](const std::string &topic_name,
                       const a17::dispatch::GuidTopicMap &guid_topic_map) {};
    topic_store.observe(topic2.name, callback);
    topic_store.observe(topic3.name, callback);

    std::set<std::string> missing = topic_store.getMissing();
    REQUIRE(missing.size() == 2);
    REQUIRE(missing.count(topic2.name) == 1);
    REQUIRE(missing.count(topic3.name) == 1);

    topic_store.add(topic2a);

    missing = topic_store.getMissing();
    REQUIRE(missing.size() == 1);
    REQUIRE(missing.count(topic2.name) == 0);
    REQUIRE(missing.count(topic3.name) == 1);
  }

  SECTION("Iterate") {
    a17::dispatch::DirectoryTopicStore topic_store(topic1.guid);

    topic_store.add(topic1);
    topic_store.add(topic2);
    topic_store.add(topic2a);
    topic_store.add(topic3);

    int count = 0;
    for (auto iter = topic_store.beginLocal(); iter != topic_store.endLocal(); iter++) {
      REQUIRE(topic2.name != iter->first);
      count++;
    }

    REQUIRE(count == 2);
  }
}

}  // namespace test
}  // namespace dispatch
}  // namespace a17
