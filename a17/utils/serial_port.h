// Based on https://github.com/KumarRobotics/asio_serial_device

#ifndef A17_UTILS_SERIAL_PORT_H_
#define A17_UTILS_SERIAL_PORT_H_

#include "character_device.h"
#include <boost/asio/serial_port.hpp>

namespace a17 {
namespace utils {

class SerialPort : public CharacterDevice {
 public:
  SerialPort(boost::asio::io_service &ios, const std::string &device, uint bitrate,
             CharacterReceiveHandler handler = CharacterReceiveHandler(),
             ErrorHandler error = ErrorHandler());

  bool isOpen();

 protected:
  virtual void asyncSend(const SendBuffer &buffer);
  virtual void asyncReceive();

  std::unique_ptr<boost::asio::serial_port> port_;
};

}  // namespace utils
}  // namespace a17

#endif  // A17_UTILS_SERIAL_PORT_H_
