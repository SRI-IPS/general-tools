#pragma once

#include <azmq/socket.hpp>
#include "socket.h"
#include "address.h"
#include "a17/utils/buffer_pool.h"

namespace a17 {
namespace dispatch {

class Directory;

// Base class for server sockets that bind to a local port.
// If address is not specified, socket binds to the preferred interface on a random free port. The
// selected port is updated in the address field after the bind succeeds. If directory is specified,
// the socket is advertised on the directory multicast channel with its address and capabilities.
class Server : public Socket {
 public:
  Server(boost::asio::io_service &ios, int socketType, const std::string &className,
         Directory &directory, const std::string &topicName,
         const std::set<message_type> &inputTypes, const std::set<message_type> &outputTypes,
         Address address = Address());

  // allows for bind to be called later
  Server(boost::asio::io_service &ios, int socketType, const std::string &className,
         const std::string &address);

  Server(Server &&other) : Socket(std::move(other)), address_(std::move(other.address_)) {}

  ~Server();

  void bind(const std::string &address);
  void unbind();

  inline const std::string &address() const { return address_; }
  inline bool isBound() const { return !address_.empty(); }

 protected:
  std::string topic_name_;
  std::string address_;
  std::function<void()> on_destroy_;

 private:
  a17::utils::BufferPool pool_;
  std::unique_ptr<Server> messageLog_;
  std::unique_ptr<azmq::socket> fileLog_;
};

namespace detail {

template <typename K, typename V>
typename std::set<K> keys(const typename std::map<K, V> &map) {
  typename std::set<K> keys;
  for (typename std::map<K, V>::const_iterator iter = map.cbegin(); iter != map.cend(); iter++) {
    keys.insert(iter->first);
  }

  return keys;
}

}

}  // namespace dispatch
}  // namespace a17
