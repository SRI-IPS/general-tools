#include <sstream>
#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#if SPDLOG_VERSION >= 10000
  #include <spdlog/sinks/stdout_color_sinks.h>
#endif
#include "publisher.h"
#include "socket.h"
#include "message_helpers.h"
#include "smart_capnp_builder.h"
#include "smart_capnp_reader.h"
#include "smart_message_reader.h"

namespace a17 {
namespace dispatch {

Socket::Socket(boost::asio::io_service &ios, int type, const std::string &class_name)
    : azmqsocket_(ios, type, true),
      receive_handler_(bind3(&Socket::onReceive)),
      log_name_(class_name) {
  logger_ = spdlog::get("Socket");
  if (!logger_) {
    try {
      logger_ = spdlog::stdout_color_mt("Socket");
    } catch (...) {
      logger_ = spdlog::get("Socket");
      if (!logger_) {
        throw std::runtime_error("Socket logger get() failed twice.");
      }
    }
  }

  received_message_.reserve(32);
  sendHighWaterMark(10);
  if(logger_) logger_->set_pattern("[%Y-%m-%d %T.%e] [%n](%l) %v");
}

// Start the process of receiving a multipart message.
void Socket::receive(SmartMessageHandler handler, ErrorHandler error_handler) {
  smart_message_handler_ = std::move(handler);
  error_handler_ = std::move(error_handler);
  // TODO(pickledgator): handle timeout logic here
  azmqsocket_.async_receive(receive_handler_);
}

// Accumulates a complete multipart message over possibly several events. When the message is
// complete, calls the handler with the message.
void Socket::onReceive(boost::system::error_code &ec, azmq::message &msg, size_t bytes) {
  if (ec) {
    if (logger_) logger_->error("{0} receive error: {1}", log_name_, strerror(ec.value()));
    if (error_handler_) error_handler_(ec);
    return;
  }

  bool more = msg.more();
  if (msg.size() > 0) {
    received_message_.push_back(std::move(msg));
  }

  if (more) {
    azmqsocket_.async_receive(receive_handler_);
  } else {
    if (logger_) {
      std::ostringstream message_ostream;
      message_ostream << received_message_;
      logger_->trace("{} received {}", log_name_, message_ostream.str());
    }

    smart_message_handler_(received_message_);

    // TODO(pickledgator): this may be dangerous if the handler doesn't make a copy!
    received_message_.clear();
  }
}

size_t Socket::send(const azmq::message_vector &message_vector, boost::system::error_code &ec) {
  if (logger_) {
    std::ostringstream message_ostream;
    message_ostream << message_vector;
    logger_->trace("{} sending {}", log_name_, message_ostream.str());
  }

  size_t size = 0;

  if (message_vector.empty()) {
    size = azmqsocket_.send(azmq::message(), ZMQ_DONTWAIT, ec);
  } else {
    for (unsigned long i = 0; i < message_vector.size() - 1 && !ec; i++) {
      size += azmqsocket_.send(message_vector[i], ZMQ_SNDMORE | ZMQ_DONTWAIT, ec);
    }

    if (!ec) size += azmqsocket_.send(message_vector[message_vector.size() - 1], ZMQ_DONTWAIT, ec);
  }

  if (ec) {
    if (logger_) logger_->error("{0} send error: {1}", log_name_, strerror(ec.value()));
  }

  return size;
}

boost::system::error_code Socket::send(const azmq::message_vector &message_vector) {
  boost::system::error_code ec;
  send(message_vector, ec);
  return ec;
}

}  // namespace dispatch
}  // namespace a17
