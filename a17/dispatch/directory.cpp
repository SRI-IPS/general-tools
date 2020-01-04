#include <chrono>
#include <iostream>
#include <sstream>

#include "boost/bind/bind.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"

#include "directory.h"
#include "publisher.h"

#define FROM_CAP_HEX(c) ((isdigit(c) ? (c) - '0' : (c) + 10 - 'A'))

namespace a17 {
namespace dispatch {

namespace {

using a17::utils::DEFAULT_POOL_BUFFER_COUNT;

const std::string DISPATCH = "DISPATCH ";
const std::string NO_TYPES = ".";
const char SEP = '|';

const boost::posix_time::time_duration QUERY_INTERVAL = boost::posix_time::seconds(1);
const boost::posix_time::time_duration QUERY_INTERVAL_MAX = boost::posix_time::seconds(16);
const boost::posix_time::time_duration QUERY_ZERO = boost::posix_time::milliseconds(50);
const int QUERY_INTERVAL_MULT = 2;

static std::string GenerateUuid() {
  static boost::uuids::random_generator uuid_generator;
  static std::mutex mtx;

  // The lock prevents generation of duplicate uuids we're not calling this often, so a lock here is
  // not a big deal
  std::unique_lock<std::mutex> lock(mtx);
  return boost::uuids::to_string(uuid_generator());
}

// Find next port number a socket should bind to. Sockets always bind to the same port number when
// the process starts. Each Directory starts with a port number 50 higher if multiple directories in
// same process.
static std::unique_ptr<std::atomic<uint16_t>> next_directory_port;

}  // namespace

// Directory
Directory::Directory(boost::asio::io_service &ios, const std::string &name, uint16_t port,
                     const std::string &multicastAddress)
    : ios_(ios),
      name_(name),
      my_guid_(name + "_" + OwnAddress::instance().address() + "_" + GenerateUuid()),
      multicastSocket_(ios),
      multicastEndpoint_(boost::asio::ip::address::from_string(multicastAddress), port),
      sendPool_(EVENT_BUFFER_SIZE, DEFAULT_POOL_BUFFER_COUNT, true),
      asioReceiveBuffer_(boost::asio::buffer(receiveBuffer_, EVENT_BUFFER_SIZE)),
      topics_(my_guid_),
      queryTimer_(ios),
      queryFn_(bind1(&Directory::queryMissingTopics)) {
  logger_ = spdlog::get("Directory|" + name_);
  if (logger_ == nullptr) {
    try {
      logger_ = spdlog::stdout_color_mt("Directory|" + name_);
    } catch (...) {
      logger_ = spdlog::get("Directory|" + name_);
      if (!logger_) {
        throw std::runtime_error("Directory logger get() failed twice.");
      }
    }
  }

  if (logger_) {
    if (logger_) logger_->set_pattern("[%Y-%m-%d %T.%e] [%n](%l) %v");
    logger_->info("listening on {}:{}", multicastAddress, port);
  }

  try {
    auto listen_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port);
    multicastSocket_.open(listen_endpoint.protocol());
    multicastSocket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    multicastSocket_.set_option(boost::asio::ip::multicast::enable_loopback(true));
    multicastSocket_.set_option(boost::asio::ip::multicast::hops(1));
    multicastSocket_.bind(listen_endpoint);
  } catch (const std::exception &e) {
    if (logger_) logger_->error("Could not open multicast socket, is network valid?");
    throw std::runtime_error("Could not open multicast socket");
  }

  try {
    multicastSocket_.set_option(
        boost::asio::ip::multicast::join_group(multicastEndpoint_.address()));
  } catch (const std::exception &e) {
    if (logger_) logger_->error("Error joining multicast group: {}", e.what());
    throw std::runtime_error("Error joining multicast group");
  }

  if (next_directory_port) {
    nextServerPort_ = next_directory_port->fetch_add(50);
  }

  receive();
}

Directory::~Directory() {
  if (logger_) logger_->info("exiting");

  // synchronous send of BYE, since we're probably no longer in the asio loop
  std::ostringstream os;
  os << DISPATCH << my_guid_ << ' ' << DISCOVERY_EXIT;
  multicastSocket_.send_to(boost::asio::buffer(os.str()), multicastEndpoint_);
}

// topics
bool Directory::add(const std::string &topic_name, int socketType, const std::string &address,
                    const std::set<message_type> &inputTypes,
                    const std::set<message_type> &outputTypes, const std::string &guid) {
  auto now = static_cast<long int>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count());
  DirectoryTopic topic{
      topic_name, now,
      socketType, address,
      inputTypes, outputTypes,
      guid,       buildTopicInfo(topic_name, socketType, address, inputTypes, outputTypes)};

  if (topics_.add(topic)) {
    assert(topics_.beginLocal() != topics_.endLocal());
    broadcast(DISCOVERY_AVAILABLE, topic.info);

    if (logger_) {
      logger_->debug("registered {} topic {} @ {}",
                     SocketTypes::instance.fromType(topic.socketType), topic.name, topic.address);
    }

    return true;
  } else {
    if (logger_) {
      logger_->debug("added {} topic {} @ {} from {}",
                     SocketTypes::instance.fromType(topic.socketType), topic.name, topic.address,
                     topic.guid);
    }

    return false;
  }
}

void Directory::remove(const std::string &topic_name) {
  if (topics_.remove(my_guid_, topic_name)) {
    broadcast(DISCOVERY_REMOVE, topic_name);
    if (logger_) logger_->debug("removed topic {}", topic_name);
  }
}

std::string Directory::observe(const std::string &topic_name, DirectoryTopicEventHandler handler) {
  std::string ref = topics_.observe(topic_name, handler);

  // call handler immediately for all matching topics, but only after we exit
  // back to event loop
  ios_.post([this, topic_name, handler]() { topics_.callImmediate(topic_name, handler); });

  startQueryTimer();

  return ref;
}

void Directory::unobserve(const std::string &topic_name, const std::string &ref) {
  topics_.unobserve(topic_name, ref);
  startQueryTimer();
}

void Directory::startQueryTimer() {
  if (queryTimerActive_) queryTimer_.cancel();
  queryTimerActive_ = true;
  queryInterval_ = QUERY_INTERVAL;
  queryTimer_.expires_from_now(QUERY_ZERO);
  queryTimer_.async_wait(queryFn_);
}

void Directory::queryMissingTopics(const boost::system::error_code &ec) {
  if (ec) return;

  std::set<std::string> requests = topics_.getMissing();
  if (!requests.empty()) {
    queryTimer_.expires_from_now(queryInterval_);
    queryTimer_.async_wait(queryFn_);
    if (queryInterval_ < QUERY_INTERVAL_MAX) queryInterval_ *= QUERY_INTERVAL_MULT;
    if (queryInterval_ > QUERY_INTERVAL_MAX) queryInterval_ = QUERY_INTERVAL_MAX;

    char *buf = nullptr;
    int count = 0;
    size_t pos = 0;
    size_t start = 0;
    size_t startLen = DISPATCH.size() + my_guid_.size() + 2;

    for (const std::string &topic_name : requests) {
      // max number of topics to group in a single query
      if (!buf || pos + 1 + topic_name.size() >= EVENT_BUFFER_SIZE || count >= 4) {
        if (buf) _send(multicastEndpoint_, buf, 0, pos);
        buf = static_cast<char *>(sendPool_.malloc());
        sprintf(buf, "%s%s %c", DISPATCH.c_str(), my_guid_.c_str(), DISCOVERY_SEARCH);
        start = startLen;
        pos = start;
        count = 0;
      }

      buf[pos++] = ' ';
      strcpy(&buf[pos], topic_name.c_str());
      pos += topic_name.size();
      count++;
    }

    if (pos > start) _send(multicastEndpoint_, buf, 0, pos);
  } else {
    queryTimerActive_ = false;
  }
}

// I/O
void Directory::receive() {
  multicastSocket_.async_receive_from(asioReceiveBuffer_, lastReceivedEndpoint_,
                                      bind2(&Directory::event));
}

void Directory::send(const boost::asio::ip::udp::endpoint &endpoint, const char command,
                     const std::string &args) {
  char *buf = static_cast<char *>(sendPool_.malloc());
  int len;

  if (!args.empty()) {
    len = sprintf(buf, "%s%s %c %s", DISPATCH.c_str(), my_guid_.c_str(), command, args.c_str());
  } else {
    len = sprintf(buf, "%s%s %c", DISPATCH.c_str(), my_guid_.c_str(), command);
  }

  _send(endpoint, buf, 0, len);
}

void Directory::_send(const boost::asio::ip::udp::endpoint &endpoint, char *buf, size_t pos,
                      size_t len) {
  multicastSocket_.async_send_to(
      boost::asio::buffer(buf + pos, len), endpoint,
      [this, &endpoint, buf, pos, len](const boost::system::error_code &ec, size_t bytes) {
        if (ec) {
          if (logger_) {
            logger_->error("couldn't send to {}: {}", endpoint.address().to_string(),
                           strerror(ec.value()));
          }
        } else if (bytes < len) {
          _send(endpoint, buf, pos + bytes, len - bytes);
        } else {
          sendPool_.free(buf);
        }
      });

  if (logger_) logger_->debug("sending \"{}\"", buf);
}

// events
void Directory::event(const boost::system::error_code &ec, size_t bytes) {
  if (ec) {
    if (logger_) logger_->error("receive error: {}", strerror(ec.value()));
    receive();
    return;
  }

  bool badChar = false;
  for (int i = 0; i < static_cast<int>(bytes); i++) {
    if (!isprint(receiveBuffer_[i])) {
      if (logger_) logger_->warn("non-printable character: {}", (uint16_t)receiveBuffer_[i]);
      badChar = true;
      break;
    }
  }

  if (!badChar) {
    receiveBuffer_[bytes] = 0;
    if (logger_) logger_->trace("received \"{}\"", receiveBuffer_);
    handleEvent(receiveBuffer_);
  }

  receive();
}

void Directory::handleEvent(char *event) {
  if (strncmp(event, &DISPATCH[0], DISPATCH.size())) {
    if (logger_) logger_->trace("ignoring event missing DISPATCH tag: {}", event);
    return;
  }

  size_t pos = DISPATCH.size();
  if (logger_) logger_->trace("event: {}", &event[pos]);
  size_t next = nextToken(event, pos);

  if (strcmp(&event[pos], &my_guid_[0])) {
    const char *guid = &event[pos];
    pos = next;
    next = nextToken(event, pos);
    char *data = &event[next];

    switch (event[pos]) {
      case DISCOVERY_AVAILABLE:
        discoveryAvailable(guid, data);
        break;
      case DISCOVERY_SEARCH:
        discoverySearch(guid, data);
        break;
      case DISCOVERY_EXIT:
        discoveryExit(guid, data);
        break;
      case DISCOVERY_REMOVE:
        discoveryRemove(guid, data);
        break;
      default:
        if (logger_) logger_->trace("ignoring event, unknown type: {}", event[pos]);
        break;
    }
  }
}

void Directory::discoveryAvailable(const char *guid, char *data) {
  size_t pos = 0;
  size_t next = nextToken(data, pos);
  if (next == pos) return;
  const char *name = &data[pos];
  if (!*name) return;

  pos = next;
  next = nextToken(data, pos);
  if (next == pos) return;
  const char *type = &data[pos];
  if (!*type) return;

  pos = next;
  next = nextToken(data, pos);
  if (next == pos) return;
  const char *address = &data[pos];
  if (!*address) return;

  pos = next;
  next = nextToken(data, pos);
  if (next == pos) return;
  const char *ins = &data[pos];

  pos = next;
  next = nextToken(data, pos);
  if (next == pos) return;
  const char *outs = &data[pos];

  Address addressParts = Address::parse(address);
  if (!OwnAddress::instance().onNetwork(addressParts.ip())) {
    if (logger_) logger_->info("ignoring topic from other network: {} {}", name, address);
    return;
  }

  int socketType = SocketTypes::instance.fromName(type);
  if (socketType < 0) {
    if (logger_) logger_->debug("unknown socket type: {}", type);
    return;
  }

  std::set<std::string> inputTypes;
  parseTypes(inputTypes, ins);
  std::set<std::string> outputTypes;
  parseTypes(outputTypes, outs);

  add(name, socketType, address, inputTypes, outputTypes, guid);
}

void Directory::discoveryRemove(const char *guid, char *data) {
  nextToken(data, 0);
  const char *name = data;
  topics_.remove(guid, name);
  startQueryTimer();
}

void Directory::discoveryExit(const char *guid, char *data) {
  topics_.evict(guid);
  startQueryTimer();

  if (logger_) logger_->debug("topics remaining: {}", topics_.size());
}

void Directory::discoverySearch(const char *guid, char *data) {
  size_t pos = 0;
  while (data[pos]) {
    size_t next = nextToken(data, pos);
    const char *name = &data[pos];
    if (*name) {
      // respond to wildcard query with all topics
      if (name[0] == '*' && name[1] == 0) {
        for (auto iter = topics_.beginLocal(); iter != topics_.endLocal(); iter++) {
          reply(DISCOVERY_AVAILABLE, iter->second.info);
        }
      } else {
        auto iter = topics_.findLocal(name);
        if (iter != topics_.endLocal()) {
          reply(DISCOVERY_AVAILABLE, iter->second.info);
        }
      }
    }

    pos = next;
  }
}

std::string Directory::buildTopicInfo(const std::string &name, int socketType,
                                      const std::string &address,
                                      const std::set<message_type> &inputTypes,
                                      const std::set<message_type> &outputTypes) {
  std::ostringstream os;
  os << name << ' ' << SocketTypes::instance.fromType(socketType) << ' ' << address << ' ';
  writeTypes(os, inputTypes);
  os << ' ';
  writeTypes(os, outputTypes);
  return os.str();
}

void Directory::writeTypes(std::ostream &os, const std::set<message_type> &types) {
  if (types.empty()) {
    os << NO_TYPES;
    return;
  }
  // Large enough for example: uint64_t,uint64_t,uint64_t.
  char buf[types.size() * sizeof(uint64_t) + types.size() - 1];
  *buf = 0;
  for (auto &type : types) {
    if (*buf) os << SEP;
    for (auto id : type) {
      sprintf(buf, "%c", id);
      os << buf;
    }
  }
};

void Directory::parseTypes(std::set<message_type> &types, const std::string &str) {
  if (str.empty() || str == NO_TYPES) {
    return;
  }
  std::istringstream is(str);
  while (!is.eof()) {
    std::string raw;
    std::getline(is, raw, SEP);
    types.insert(raw);
  }
}

size_t Directory::nextToken(char *event, size_t start) {
  if (start >= EVENT_BUFFER_SIZE) return start;
  auto end = start;
  while (end < EVENT_BUFFER_SIZE && event[end] != ' ' && event[end] != 0) {
    ++end;
  }
  if (end == EVENT_BUFFER_SIZE || event[end] == 0) {
    return end;
  }
  event[end] = 0;
  return end + 1;
}

bool DirectoryTopicStore::add(const DirectoryTopic &topic) {
  bool added_mine = topic.guid == my_guid_;
  if (added_mine) {
    if (local_topics_.count(topic.name)) {
      throw std::runtime_error("Duplicate topic: " + topic.name);
    }
    local_topics_[topic.name] = topic;
  }

  if (network_topics_.count(topic.name)) {
    GuidTopicMap &guid_topic_map = network_topics_[topic.name];
    if (!guid_topic_map.count(topic.guid)) {
      guid_topic_map[topic.guid] = topic;
      callObservers(topic.name, guid_topic_map);
    } else if (guid_topic_map[topic.guid].address != topic.address) {
      guid_topic_map[topic.guid] = topic;
      callObservers(topic.name, guid_topic_map);
    }
  } else {
    GuidTopicMap guid_topic_map;
    guid_topic_map[topic.guid] = topic;
    network_topics_[topic.name] = guid_topic_map;
    callObservers(topic.name, guid_topic_map);
  }
  return added_mine;
}

bool DirectoryTopicStore::remove(const std::string &guid, const std::string &topic_name) {
  bool removed_mine = false;
  if (guid == my_guid_) {
    removed_mine = local_topics_.erase(topic_name) > 0;
  }
  if (network_topics_.count(topic_name)) {
    auto &guid_topic_map = network_topics_[topic_name];
    if (guid_topic_map.erase(guid) > 0) {
      ;
      callObservers(topic_name, guid_topic_map);
    }
    if (guid_topic_map.empty()) {
      network_topics_.erase(topic_name);
    }
  }
  return removed_mine;
}

void DirectoryTopicStore::evict(const std::string &guid) {
  for (auto iter = network_topics_.begin(); iter != network_topics_.end();) {
    const auto topic_name = iter->first;
    auto &guid_topic_map = iter->second;
    if (guid_topic_map.erase(guid) > 0) {
      std::cout << "CALLING CALLBACK for topic_name: " << topic_name << " guid: " << guid
                << std::endl;
      callObservers(topic_name, guid_topic_map);
    }
    if (guid_topic_map.empty()) {
      iter = network_topics_.erase(iter);
    } else {
      ++iter;
    }
  }
}

std::string DirectoryTopicStore::observe(const std::string &topic_name,
                                         DirectoryTopicEventHandler handler) {
  auto ref = GenerateUuid();
  observers.emplace(topic_name, DirectoryObserver{handler, ref});
  return ref;
}

void DirectoryTopicStore::callImmediate(const std::string &topic_name,
                                        DirectoryTopicEventHandler handler) {
  auto iter = network_topics_.find(topic_name);
  if (iter != network_topics_.cend()) {
    handler(topic_name, iter->second);
  }
}

void DirectoryTopicStore::unobserve(const std::string &topic_name, const std::string &ref) {
  auto range = observers.equal_range(topic_name);
  auto iter = range.first;
  while (iter != range.second) {
    if (iter->second.ref == ref) {
      iter = observers.erase(iter);
    } else {
      iter++;
    }
  }
}

void DirectoryTopicStore::callObservers(const std::string &topic_name,
                                        const GuidTopicMap &guid_topic_map) {
  // Save handlers to execute outside loops.
  std::vector<DirectoryTopicEventHandler> handlers;

  // Observers seeking updates to this topic.
  auto range = observers.equal_range(topic_name);
  for (auto iter = range.first; iter != range.second; ++iter) {
    handlers.push_back(iter->second.handler);
  }

  // Observers seeking all updates.
  auto range2 = observers.equal_range("");
  for (auto iter2 = range2.first; iter2 != range2.second; ++iter2) {
    handlers.push_back(iter2->second.handler);
  }

  for (auto handler : handlers) {
    handler(topic_name, guid_topic_map);
  }
}

std::set<std::string> DirectoryTopicStore::getMissing() {
  std::set<std::string> topics;

  for (auto &entry : observers) {
    const std::string &topic_name = entry.first;
    if (network_topics_.count(topic_name) == 0) {
      topics.insert(topic_name);
    }
  }

  return topics;
}

std::map<std::string, DirectoryTopic>::const_iterator DirectoryTopicStore::findLocal(
    const std::string &topic_name) {
  return local_topics_.find(topic_name);
}

}  // namespace dispatch
}  // namespace a17
