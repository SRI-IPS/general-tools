#include "client.h"

#include "spdlog/spdlog.h"

#include "directory.h"

namespace a17 {
namespace dispatch {

Client::Client(boost::asio::io_service &ios, int socketType, const std::string &class_name,
               Directory &directory, const std::string &topic_name, ConnectionHandler on_connect,
               ConnectionHandler on_disconnect)
    : Socket(ios, socketType, class_name),
      on_connect_(on_connect),
      on_disconnect_(on_disconnect),
      topic_name_(topic_name),
      class_name_(class_name) {
  log_name_ += " [" + topic_name + "]";
  monitor_ref_ = directory.observe(topic_name, bind2(&Client::onDirectoryTopicsChanged));
  on_destroy_ = [this, &directory]() { directory.unobserve(topic_name_, monitor_ref_); };
}

Client::Client(boost::asio::io_service &ios, int socketType, const std::string &class_name,
               const std::string &address)
    : Socket(ios, socketType, class_name), class_name_(class_name) {
  if (!address.empty()) connect(address);
}

Client::Client(boost::asio::io_service &ios, int socketType, const std::string &class_name,
               const std::set<std::string> &addresses)
    : Socket(ios, socketType, class_name), class_name_(class_name) {
  if (addresses.empty()) {
    throw std::runtime_error("No addresses to connect to");
  }

  for (auto iter = addresses.cbegin(); iter != addresses.cend(); iter++) {
    connect(*iter);
  }

  log_name_ = class_name + " (" + *(addresses_.begin()) + "+)";
}

Client::~Client() {
  if (on_destroy_) on_destroy_();
}

void Client::onDirectoryTopicsChanged(const std::string &topic_name,
                                      const GuidTopicMap &guid_topic_map) {
  auto new_addresses = std::vector<std::string>{};
  for (const auto &item : guid_topic_map) {
    new_addresses.push_back(item.second.address);
  }
  // Connect to any new addresses that we aren't already connected to.
  for (const auto &new_address : new_addresses) {
    if (std::find(addresses_.begin(), addresses_.end(), new_address) == addresses_.end()) {
      connect(new_address);
    }
  }
  // Disconnect from any addresses that we are currently connected to, but are no longer in the list
  // of directory topics. This currently will happen when a node disconnects gracefully.
  for (const auto &address : addresses_) {
    if (std::find(new_addresses.begin(), new_addresses.end(), address) == new_addresses.end()) {
      disconnect(address);
    }
  }
}

void Client::connect(const std::string &address) {
  if (addresses_.count(address) == 0) {
    auto logger = spdlog::get("Socket");
    if (logger) logger->info("{0} @ {1}", log_name_, address);
    addresses_.insert(address);
    azmqsocket_.connect(address);
    if (log_name_ == class_name_) log_name_ += "(" + address + ")";
    if (on_connect_) on_connect_(topic_name_);
  }
}

void Client::disconnect(const std::string &address) {
  auto logger = spdlog::get("Socket");
  if (addresses_.count(address)) {
    if (logger) logger->info("{0} !@ {1}", log_name_, address);
    addresses_.erase(address);
    azmqsocket_.disconnect(address);
    if (on_disconnect_) on_disconnect_(topic_name_);
  } else {
    if (logger) logger->info("{0} already disconnected from {1}", log_name_, address);
  }
}

void Client::setConnectionHandlers(ConnectionHandler on_connect, ConnectionHandler on_disconnect) {
  on_connect_ = on_connect;
  on_disconnect_ = on_disconnect;
}

}  // namespace dispatch
}  // namespace a17
