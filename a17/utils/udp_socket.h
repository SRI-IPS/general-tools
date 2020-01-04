#ifndef A17_UTILS_UDP_SOCKET_H_
#define A17_UTILS_UDP_SOCKET_H_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include "bind.h"

namespace a17 {
namespace utils {

class GenericUdp {
 private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint target_;
  double receiveBuffer_[1024];
  boost::asio::deadline_timer recoveryTimer_;
  std::shared_ptr<spdlog::logger> logger_;
  std::function<void(std::vector<double>)> handler_;
  bool firstMessage_;

  void onReceive(const boost::system::error_code &ec, size_t bytes);
  void receive();
  void parser(size_t bytes);
  void onSend(const boost::system::error_code &error, std::size_t bytes);

 public:
  GenericUdp(boost::asio::io_service &ios, uint16_t port,
             std::function<void(std::vector<double>)> handler);
  void send(const std::vector<double> &v);
  bool connected();
};

}  // namespace utils
}  // namespace a17

#endif // A17_UTILS_UDP_SOCKET_H_
