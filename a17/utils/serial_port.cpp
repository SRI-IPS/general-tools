#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include "serial_port.h"

namespace a17 {
namespace utils {

SerialPort::SerialPort(boost::asio::io_service &ios, const std::string &device, uint bitrate,
                       CharacterReceiveHandler receiveHandler, ErrorHandler error)
    : CharacterDevice("serialport", device, receiveHandler, error) {
  try {
    port_.reset(new boost::asio::serial_port(ios, device));
  } catch (...) {
    std::cerr << "Serial port failed to open." << std::endl;
    return;
  }

  port_->set_option(boost::asio::serial_port_base::baud_rate(bitrate));
  asyncReceive();
}

void SerialPort::asyncReceive() { port_->async_read_some(asioReceiveBuffer_, readHandler_); }

void SerialPort::asyncSend(const SendBuffer &buffer) {
  port_->async_write_some(buffer.cbuffer(), sendHandler_);
}

bool SerialPort::isOpen() {
  if (port_) {
    if (port_->is_open()) return true;
  }
  return false;
}

}  // namespace utils
}  // namespace a17
