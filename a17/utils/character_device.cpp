#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <spdlog/spdlog.h>
#include "character_device.h"
// #include "messages.hpp"

namespace a17 {
namespace utils {

CharacterDevice::CharacterDevice(const std::string &logger, const std::string &logName,
                                 CharacterReceiveHandler receiveHandler, ErrorHandler error)
    : receiveHandler_(receiveHandler),
      errorHandler_(error),
      readHandler_(boost::bind(&CharacterDevice::onReceive, this, boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred)),
      sendHandler_(boost::bind(&CharacterDevice::onSend, this, boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred)),
      asioReceiveBuffer_(boost::asio::buffer(&receiveBuffer_[0], receiveBuffer_.size())),
      logger_(spdlog::get(logger)),
      logName_(logName) {}

CharacterDevice::CharacterDevice(CharacterDevice &&other)
    : receiveHandler_(std::move(other.receiveHandler_)),
      errorHandler_(std::move(other.errorHandler_)),
      readHandler_(boost::bind(&CharacterDevice::onReceive, this, boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred)),
      sendHandler_(boost::bind(&CharacterDevice::onSend, this, boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred)),
      asioReceiveBuffer_(boost::asio::buffer(&receiveBuffer_[0], receiveBuffer_.size())),
      highWaterMark_(other.highWaterMark_) {}

void CharacterDevice::onReceive(const boost::system::error_code &ec, size_t bytes) {
  if (!ec) {
    if (logger_) {
      logger_->trace("{} received, bytes: {}", logName_, bytes);
    }

    if (receiveHandler_) {
      receiveHandler_(&receiveBuffer_[0], bytes);
    }
  } else {
    if (logger_) logger_->error("{0} receive error: {1}", logName_, strerror(ec.value()));
    if (errorHandler_) errorHandler_(ec);
  }

  asyncReceive();
}

void CharacterDevice::send(const uint8_t *buffer, size_t bytes) {
  if (exceededHighWaterMark()) return;

  for (size_t i = 0; i < bytes; i += pool_.bufferSize()) {
    send([&](void *buf, size_t max) {
      size_t remain = bytes - i;
      size_t current = remain > max ? max : remain;
      memcpy(buf, &buffer[i], current);
      return current;
    });
  }
}

void CharacterDevice::send(std::function<size_t(void *buf, size_t max)> copy) {
  if (exceededHighWaterMark()) return;

  SendBuffer buffer(pool_);
  buffer.reset(copy(buffer.data(), buffer.maxSize()));
  bool wasEmpty = sendBuffers_.empty();
  sendBuffers_.push_back(std::move(buffer));
  if (wasEmpty) asyncSend(sendBuffers_.front());
}

void CharacterDevice::onSend(const boost::system::error_code &ec, size_t bytes) {
  if (!ec) {
    SendBuffer &buffer = sendBuffers_.front();

    if (logger_) {
      logger_->trace("{} sent msg, bytes: {}", logName_, bytes);
    }

    if (buffer.advance(bytes)) {
      sendBuffers_.pop_front();
    } else {
      if (logger_) logger_->debug("{0} sending more", logName_);
    }

    if (!sendBuffers_.empty()) {
      asyncSend(sendBuffers_.front());
    }
  } else {
    if (logger_) logger_->error("{0} send error: {1}", logName_, strerror(ec.value()));
  }
}

bool CharacterDevice::exceededHighWaterMark() {
  if (sendBuffers_.size() >= highWaterMark_) {
    if (logger_) logger_->warn("{0} exceeded high water mark; send dropped", logName_);
    return true;
  }

  return false;
}

}  // namespace utils
}  // namespace a17
