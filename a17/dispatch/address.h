#pragma once

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#include <string>
#include <stdint.h>

namespace a17 {
namespace dispatch {

const size_t IP_STRING_BUF = 200;

// Determines the address of the local interface to use for sockets.
// checks for DISPATCH_IP if set as env var

class OwnAddress {
 public:
  OwnAddress();
  inline std::string address() const { return address_; }
  inline std::string broadcast() const { return broadcast_; }
  inline std::string netmask() const { return netmask_; }
  inline operator std::string() { return address_; }
  inline std::string interface() const { return interface_; }

  static OwnAddress &instance() {
    static OwnAddress instance;
    return instance;
  }

  bool onNetwork(const std::string &address);

 private:
  std::string address_;
  std::string broadcast_;
  std::string netmask_;
  in_addr_t network_;
  in_addr_t mask_;
  std::string interface_;
};

class Address {
 public:
  Address(uint16_t port = 0)
      : protocol_("tcp"),
        ip_(OwnAddress::instance().address()),
        port_(port),
        interface_(OwnAddress::instance().interface()) {}
  Address(const std::string &ip, uint16_t port = 0) : protocol_("tcp"), ip_(ip), port_(port) {}
  Address(const std::string &protocol, const std::string &ip, uint16_t port = 0)
      : protocol_(protocol), ip_(ip), port_(port) {}
  Address(const Address &other) : protocol_(other.protocol_), ip_(other.ip_), port_(other.port_) {}
  Address(Address &&other)
      : protocol_(std::move(other.protocol_)), ip_(std::move(other.ip_)), port_(other.port_) {}
  static Address parse(const std::string &address);

  inline const std::string &protocol() const { return protocol_; }
  inline const std::string &ip() const { return ip_; }
  inline const std::string &interface() const { return interface_; }
  inline uint16_t port() const { return port_; }
  inline std::string address() const {
    return protocol_ + "://" + ip_ + ":" + std::to_string(port_);
  }
  inline operator std::string() const { return address(); }
  inline bool isLocalhost() const { return ip_ == "127.0.0.1" || ip_ == "localhost"; }

  void setPort(uint16_t port) { port_ = port; }

 private:
  std::string protocol_;
  std::string ip_;
  uint16_t port_;
  std::string interface_;
};

}  // namespace dispatch
}  // namespace a17
