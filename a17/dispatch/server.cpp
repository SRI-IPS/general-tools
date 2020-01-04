#include <spdlog/spdlog.h>
#include "server.h"
#include "directory.h"

namespace a17 {
namespace dispatch {

Server::Server(boost::asio::io_service &ios, int socketType, const std::string &className,
               Directory &directory, const std::string &topicName,
               const std::set<message_type> &inputTypes, const std::set<message_type> &outputTypes,
               Address address)
    : Socket(ios, socketType, className), topic_name_(topicName) {
  log_name_ += " [" + topicName + "]";

  uint16_t port = directory.nextServerPort();
  if (port) address.setPort(port);

  try {
    bind(address);
  } catch (boost::system::system_error err) {
    if (port) {
      if (logger_) logger_->debug("Bind error in {}, retrying with random port", log_name_);
      address.setPort(0);
      bind(address);
    } else {
      throw err;
    }
  }

  directory.add(topicName, socketType, address_, inputTypes, outputTypes, directory.guid());

  on_destroy_ = [&]() { directory.remove(topic_name_); };
}

Server::Server(boost::asio::io_service &ios, int socketType, const std::string &className,
               const std::string &address)
    : Socket(ios, socketType, className) {
  bind(address);
  log_name_ += "(" + address_ + ")";
}

Server::~Server() {
  if (on_destroy_) on_destroy_();
}

void Server::bind(const std::string &address) {
  auto logger = spdlog::get("Socket");
  if (logger) logger->debug("{0} binding to {1}", log_name_, address);

  azmqsocket_.bind(address);
  address_ = address;

  char addr[256];
  azmq::socket::last_endpoint endpoint(addr, sizeof(addr));
  boost::system::error_code ec;
  azmqsocket_.get_option(endpoint, ec);

  if (addr != address) {
    address_.assign(addr);
    if (logger) logger->info("{0} @ {1}", log_name_, address_);
  }
};

void Server::unbind() {
  if (!address_.empty()) {
    azmqsocket_.unbind(address_);
    auto logger = spdlog::get("Socket");
    if (logger) logger->info("{0} !@ {1}", log_name_, address_);
  }
}

}  // namespace dispatch
}  // namespace a17
