#include "udp_socket.h"
#include <sstream>

namespace a17 {
namespace utils {

GenericUdp::GenericUdp(boost::asio::io_service &ios, uint16_t port,
                       std::function<void(std::vector<double>)> handler)
    : socket_(ios),
      target_(boost::asio::ip::udp::v4(), 0),
      recoveryTimer_(ios),
      logger_(spdlog::get("Socket")),
      handler_(handler) {
  auto endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port);
  socket_.open(endpoint.protocol());
  socket_.bind(endpoint);
  firstMessage_ = false;

  if (logger_) logger_->info("Waiting for first message from target...");
  receive();
}

void GenericUdp::receive() {
  socket_.async_receive_from(boost::asio::buffer(receiveBuffer_, 1024), target_,
                             bind2(&GenericUdp::onReceive));
}

void GenericUdp::onReceive(const boost::system::error_code &ec, size_t bytes) {
  if (!firstMessage_) {
    if (logger_) {
      std::ostringstream address_ostream;
      address_ostream << target_.address();
      logger_->info("Got a message from target: {}", address_ostream.str());
    }
    firstMessage_ = true;
  }

  if (!ec) {
    parser(bytes);
    receive();
  } else {
    if (logger_) logger_->error("{0} receive error: {1}", "Socket", strerror(ec.value()));
    recoveryTimer_.expires_from_now(boost::posix_time::seconds(1));
    recoveryTimer_.async_wait([this](const boost::system::error_code &ec) { receive(); });
  }
}

void GenericUdp::send(
    const std::vector<double> &v) {  // TODO make sure v doesnt go out of scope before send
  auto buf = boost::asio::buffer(v.data(), v.size());
  if (firstMessage_) {
    socket_.async_send_to(buf, target_,
                          boost::bind(&GenericUdp::onSend, this, boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
  } else {
    if (logger_) logger_->debug("Not sending, have not received target infomation yet");
    onSend(boost::system::error_code(), boost::asio::buffer_size(buf));
  }
}

void GenericUdp::onSend(const boost::system::error_code &error, std::size_t bytes) {}

void GenericUdp::parser(size_t bytes) {
  if (bytes < 8) {
    if (logger_) logger_->error("parser received less than 8 bytes ({}), ignoring.", bytes);
    return;
  }
  std::vector<double> out;
  for (int i = 0; i < bytes / 8; i++) {
    out.push_back(receiveBuffer_[i]);
  }
  handler_(out);
}

bool GenericUdp::connected() { return firstMessage_; }

}  // namespace utils
}  // namespace a17
