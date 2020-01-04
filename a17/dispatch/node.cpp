#include "node.h"

#include "defs.h"

namespace a17 {
namespace dispatch {

Node::Node(const std::string &name /* ="Node" */)
    : name_(!name.empty() ? name : "Node"),
      directory_(ios_, name_),
      signals_(ios_, SIGINT, SIGTERM, SIGHUP) {
  if (name_.find_first_of(' ') != std::string::npos) {
    throw std::runtime_error("Process name must not contain spaces");
  }

  logger_ = spdlog::get(name_);
  if(!logger_) {
    logger_ = spdlog::stdout_color_mt(name_);
  }

  if (logger_) {
    logger_->set_pattern("[%Y-%m-%d %T.%e] [%n](%l) %v");
    logger_->info("Node {} starting up", name);
  }
  signals_.async_wait(bind2(&Node::signal));
}

Node::~Node() {
  stop();
  if (future_.valid()) {
    future_.wait();
  }
}

Topic Node::topic(const std::string &topic) {
  return Topic{DeviceName(), name(), topic};
}

void Node::start() {
  future_ = std::async(std::launch::async, [this]() { this->run(); });
}

void Node::stop() {
  signaled_shutdown_ = true;
  ios_.stop();
}

void Node::signal(const boost::system::error_code &ec, int signalNumber) {
  if (!ec) {
    if (logger_) logger_->info("Node shutting down");
    stop();
  }
}

}  // namespace dispatch
}  // namespace a17
