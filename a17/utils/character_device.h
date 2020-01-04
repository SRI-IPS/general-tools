// Based on https://github.com/KumarRobotics/asio_serial_device

#ifndef A17_UTILS_CHARACTER_DEVICE_H_
#define A17_UTILS_CHARACTER_DEVICE_H_

#include <deque>
#include <spdlog/logger.h>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>
// #include "defs.hpp"
#include "buffer_pool.h"

namespace a17 {
namespace utils {

namespace {

const int kHighWaterMark = 1024;
typedef std::function<void(const void *buf, size_t bytes)> CharacterReceiveHandler;
typedef std::function<void(const boost::system::error_code &)> ErrorHandler;

} // namespace

class SendBuffer {
 public:
  SendBuffer(size_t maxSize)
      : buffer_(static_cast<uint8_t *>(::malloc(maxSize))), maxSize_(maxSize) {
    free_ = [](uint8_t *buf) { ::free(buf); };
  }

  SendBuffer(BufferPool &pool)
      : buffer_(static_cast<uint8_t *>(pool.malloc())), maxSize_(pool.bufferSize()) {
    free_ = [&pool](uint8_t *buf) { pool.free(buf); };
  }

  SendBuffer(SendBuffer &&other)
      : buffer_(other.buffer_),
        maxSize_(other.maxSize_),
        size_(other.size_),
        pos_(other.pos_),
        free_(other.free_) {
    other.buffer_ = nullptr;
  }

  ~SendBuffer() {
    if (buffer_) free_(buffer_);
  }

  inline uint8_t *data() const { return buffer_; }
  inline uint8_t *dataPos() const { return buffer_ + pos_; }
  inline size_t size() const { return size_; }
  inline size_t maxSize() const { return maxSize_; }
  inline size_t pos() const { return pos_; }
  inline uint8_t &operator[](size_t pos) const { return buffer_[pos]; }

  inline void reset(size_t size) {
    size_ = size;
    pos_ = 0;
  }
  inline bool advance(size_t bytes) {
    pos_ += bytes;
    return isSent();
  }
  inline bool isSent() const { return pos_ >= size_; }

  inline boost::asio::const_buffers_1 cbuffer() const {
    return boost::asio::const_buffers_1(buffer_ + pos_, size_ - pos_);
  }

 private:
  uint8_t *buffer_;
  size_t maxSize_;
  size_t size_ = 0;
  size_t pos_ = 0;
  std::function<void(uint8_t *)> free_;
};

class CharacterDevice {
 public:
  CharacterDevice(const std::string &logger, const std::string &logName,
                  CharacterReceiveHandler handler = CharacterReceiveHandler(),
                  ErrorHandler error = ErrorHandler());

  CharacterDevice(CharacterDevice &&other);

  //! Send the contents of the specified buffer
  void send(const uint8_t *buffer, size_t bytes);
  //! Pass your own function that copies your data directly into the internal
  //! send buffer
  void send(std::function<size_t(void *buf, size_t max)> copy);

  void setReceiveHandler(CharacterReceiveHandler handler) { receiveHandler_ = handler; }

  inline void sendHighWaterMark(uint32_t hwm) { highWaterMark_ = hwm; }

 protected:
  virtual void asyncSend(const SendBuffer &buffer) = 0;
  virtual void asyncReceive() = 0;
  void onSend(const boost::system::error_code &ec, size_t bytes);
  void onReceive(const boost::system::error_code &ec, size_t bytes);
  bool exceededHighWaterMark();

  BufferPool pool_;
  CharacterReceiveHandler receiveHandler_;
  ErrorHandler errorHandler_;
  std::function<void(const boost::system::error_code &ec, size_t)> readHandler_;
  std::function<void(const boost::system::error_code &ec, size_t)> sendHandler_;

  std::deque<SendBuffer> sendBuffers_;
  std::array<uint8_t, 1024> receiveBuffer_;
  boost::asio::mutable_buffers_1 asioReceiveBuffer_;
  uint32_t highWaterMark_ = kHighWaterMark;

  std::shared_ptr<spdlog::logger> logger_;
  std::string logName_;
};

}  // namespace utils
}  // namespace a17

#endif  // A17_UTILS_CHARACTER_DEVICE_H_
