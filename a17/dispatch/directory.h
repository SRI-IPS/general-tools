#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "boost/asio.hpp"
#include "boost/asio/ip/multicast.hpp"
#include "boost/asio/ip/udp.hpp"
#include "spdlog/logger.h"

#include "a17/utils/buffer_pool.h"

#include "address.h"
#include "defs.h"
#include "directory_topic.h"
#include "socket_types.h"

namespace a17 {
namespace dispatch {

class Directory;

const uint16_t DEFAULT_DIRECTORY_PORT =
    std::getenv("DISPATCH_PORT") ? std::atoi(std::getenv("DISPATCH_PORT")) : 8888;
const std::string DEFAULT_DIRECTORY_MULTICAST =
    std::getenv("DISPATCH_MULTICAST") ? std::getenv("DISPATCH_MULTICAST") : "224.0.88.1";

const char DISCOVERY_EXIT = 'X';
const char DISCOVERY_AVAILABLE = 'A';
const char DISCOVERY_REMOVE = 'R';
const char DISCOVERY_SEARCH = 'S';
const size_t EVENT_BUFFER_SIZE = 1500;

using GuidTopicMap = std::map<std::string, DirectoryTopic>;
using DirectoryTopicEventHandler =
    std::function<void(const std::string &topic_name, const GuidTopicMap &guid_topic_map)>;

struct DirectoryObserver {
  DirectoryTopicEventHandler handler;
  std::string ref;
};

class DirectoryTopicStore {
 public:
  DirectoryTopicStore(const std::string &my_guid) : my_guid_(my_guid) {}
  bool add(const DirectoryTopic &topic);
  bool remove(const std::string &guid, const std::string &topic_name);
  void evict(const std::string &guid);
  std::string observe(const std::string &topic_name, DirectoryTopicEventHandler handler);
  void callImmediate(const std::string &topic_name, DirectoryTopicEventHandler event);
  void unobserve(const std::string &topic_name, const std::string &ref);

  size_t size() const { return network_topics_.size(); }
  bool hasTopic(const std::string &topic_name) const {
    return network_topics_.count(topic_name) > 0;
  }
  std::set<std::string> getMissing();

  size_t localSize() const { return local_topics_.size(); }
  std::map<std::string, DirectoryTopic>::const_iterator findLocal(const std::string &topic_name);
  std::map<std::string, DirectoryTopic>::const_iterator beginLocal() {
    return local_topics_.cbegin();
  }
  std::map<std::string, DirectoryTopic>::const_iterator endLocal() { return local_topics_.cend(); }

 private:
  std::string my_guid_;
  std::map<std::string, DirectoryTopic> local_topics_;
  std::map<std::string, GuidTopicMap> network_topics_;
  std::multimap<std::string, DirectoryObserver> observers;

  void callObservers(const std::string &topic_name, const GuidTopicMap &guid_topic_map);
};

/**
 * Transport and mapping of topics to sockets hosted by all nodes on the network. Utilizes
 * multi-cast UDP mechanism for allowing distributed nodes to advetise/query for specific mappings.
 *
 * DISCOVERY_EXIT - The node is shutting down and all socket/topics are being removed
 * DISCOVERY_AVAILABLE - Inform other nodes that a specific topic/socket is available
 * DISCOVERY_SEARCH - Ask other nodes if a specific topic/socket is available
 * DISCOVERY_REMOVE - Inform all nodes to remove a specific topic/socket
 *
 * When a client needs to connect to a particular topic, it looks in the local list.
 * If not present, the directory sends a DISCOVERY_SEARCH on a repeater until a
 * DISCOVERY_AVAILABLE with the topic info is received from another node.
 */
class Directory {
 public:
 public:
  Directory(boost::asio::io_service &ios, const std::string &name,
            uint16_t port = DEFAULT_DIRECTORY_PORT,
            const std::string &multicastAddress = DEFAULT_DIRECTORY_MULTICAST);

  ~Directory();
  Directory(const Directory &) = delete;
  Directory(Directory &&) = delete;

  inline const std::string &guid() const { return my_guid_; }
  inline const DirectoryTopicStore &topics() const { return topics_; }
  bool add(const std::string &topic_name, int socketType, const std::string &address,
           const std::set<message_type> &inputTypes, const std::set<message_type> &outputTypes,
           const std::string &guid);
  void remove(const std::string &topic_name);
  std::string observe(const std::string &topic_name, DirectoryTopicEventHandler handler);
  void unobserve(const std::string &topic_name, const std::string &ref);

  void handleEvent(char *event);
  void discoveryExit(const char *guid, char *data);
  void discoveryAvailable(const char *guid, char *data);
  void discoveryRemove(const char *guid, char *data);
  void discoverySearch(const char *guid, char *data);

  // Send message to other directories
  inline void broadcast(const char command, const std::string &args = "") {
    send(multicastEndpoint_, command, args);
  }

  inline void reply(const char command, const std::string &args = "") { broadcast(command, args); }

  inline uint16_t nextServerPort() { return nextServerPort_ ? nextServerPort_++ : 0; }

  std::string buildTopicInfo(const std::string &name, int socketType, const std::string &address,
                             const std::set<message_type> &inputTypes,
                             const std::set<message_type> &outputTypes);

 protected:
  void receive();
  virtual void send(const boost::asio::ip::udp::endpoint &endpoint, const char command,
                    const std::string &args = "");

  void event(const boost::system::error_code &error, size_t bytes);

  void startQueryTimer();
  void queryMissingTopics(const boost::system::error_code &ec);

  void writeTypes(std::ostream &, const std::set<message_type> &types);
  void parseTypes(std::set<message_type> &types, const std::string &str);

 private:
  boost::asio::io_service &ios_;
  std::string name_;

  const std::string my_guid_;
  boost::asio::ip::udp::socket multicastSocket_;
  boost::asio::ip::udp::endpoint multicastEndpoint_;
  boost::asio::ip::udp::endpoint lastReceivedEndpoint_;
  std::shared_ptr<spdlog::logger> logger_;

  a17::utils::BufferPool sendPool_;
  char receiveBuffer_[EVENT_BUFFER_SIZE + 1];
  boost::asio::mutable_buffers_1 asioReceiveBuffer_;

  DirectoryTopicStore topics_;
  boost::asio::deadline_timer queryTimer_;
  boost::posix_time::time_duration queryInterval_;
  bool queryTimerActive_ = false;
  std::function<void(const boost::system::error_code &ec)> queryFn_;

  uint16_t nextServerPort_ = 0;

  void _send(const boost::asio::ip::udp::endpoint &endpoint, char *buf, size_t pos, size_t len);
  size_t nextToken(char *event, size_t start);
};

}  // namespace dispatch
}  // namespace a17
