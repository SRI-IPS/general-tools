#pragma once

#include "azmq/socket.hpp"
#include "spdlog/logger.h"

#include "defs.h"
#include "handlers.h"

namespace a17 {
namespace dispatch {

class Socket {
  friend class Listener;

 public:
  Socket(boost::asio::io_service &ios, int type, const std::string &class_name);

  Socket(Socket &&other)
      : azmqsocket_(std::move(other.azmqsocket_)),
        received_message_(std::move(other.received_message_)),
        smart_message_handler_(other.smart_message_handler_),
        receive_handler_(bind3(&Socket::onReceive)),
        log_name_(std::move(other.log_name_)),
        logger_(std::move(other.logger_)) {}

  ~Socket() {
    if (logger_) logger_->info("{0} destroyed", log_name_);
  }

  inline azmq::socket &socket() { return azmqsocket_; }
  inline const std::string &logName() const { return log_name_; }

  void receive(SmartMessageHandler handler, ErrorHandler errorHandler = ErrorHandler());

  virtual size_t send(const azmq::message_vector &message, boost::system::error_code &ec);
  boost::system::error_code send(const azmq::message_vector &message);

  inline void sendHighWaterMark(uint32_t hwm) {
    azmqsocket_.set_option(azmq::socket::snd_hwm(hwm));
  }

 protected:
  void onReceive(boost::system::error_code &ec, azmq::message &msg, size_t bytes);

  azmq::socket azmqsocket_;
  azmq::message_vector received_message_;
  SmartMessageHandler smart_message_handler_;
  ErrorHandler error_handler_;
  const std::function<void(boost::system::error_code &ec, azmq::message &, size_t)>
      receive_handler_;

  // logging
  std::string log_name_;
  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace dispatch
}  // namespace a17
