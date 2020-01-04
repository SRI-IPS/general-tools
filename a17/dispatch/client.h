#pragma once

#include "azmq/socket.hpp"

#include "directory.h"
#include "handlers.h"
#include "socket.h"

namespace a17 {
namespace dispatch {

// Base class for client sockets that connect to a remote port.
class Client : public Socket {
 private:
  std::set<std::string> addresses_;
  ConnectionHandler on_connect_;
  ConnectionHandler on_disconnect_;

  const std::string topic_name_;
  const std::string class_name_;
  std::string monitor_ref_;
  std::function<void()> on_destroy_;

 public:
  // Connect client to a specific topic in the directory.
  // Declared socket type must be compatible with the topic's socket type (sub->pub, etc.)

  // If multiple nodes on the network advertise the same topic, the client connects to all of them.
  // If the topic isn't yet known by the directory, the directory broadcasts a periodic request for
  // the topic until another node on the network replies, after which the connection is made.
  Client(boost::asio::io_service &ios, int socketType, const std::string &class_name,
         Directory &directory, const std::string &topic_name,
         ConnectionHandler on_connect = ConnectionHandler(),
         ConnectionHandler on_disconnect = ConnectionHandler());

  // Connect client to a socket at a specific address. If empty address is provided, no connection
  // will be made, allowing the caller to call connect() at a later time.
  Client(boost::asio::io_service &ios, int socketType, const std::string &class_name,
         const std::string &address = "");

  Client(boost::asio::io_service &ios, int socketType, const std::string &class_name,
         const std::set<std::string> &addresses);

  Client(Client &&other) : Socket(std::move(other)), addresses_(std::move(other.addresses_)) {}

  ~Client();

  void onDirectoryTopicsChanged(const std::string &topic_name, const GuidTopicMap &guid_topic_map);
  virtual void connect(const std::string &address);
  void disconnect(const std::string &address);

  inline const std::string &topic() const { return topic_name_; }
  inline const std::set<std::string> &addresses() const { return addresses_; }
  inline bool isConnected(unsigned long count = 1) { return addresses_.size() >= count; }
  void setConnectionHandlers(ConnectionHandler on_connect,
                             ConnectionHandler on_disconnect = ConnectionHandler());
};

}  // namespace dispatch
}  // namespace a17
