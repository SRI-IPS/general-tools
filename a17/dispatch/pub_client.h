#pragma once

#include "client.h"

namespace a17 {
namespace dispatch {

// treats zmq publisher as the client (use with subserver)
class PubClient : public Client {
 public:
  PubClient(boost::asio::io_service &ios, Directory &directory, const std::string &subServerTopic,
            ConnectionHandler connectHandler = ConnectionHandler(),
            ConnectionHandler disconnectHandler = ConnectionHandler())
      : Client(ios, ZMQ_PUB, "PubClient", directory, subServerTopic, connectHandler,
               disconnectHandler) {}

  PubClient(boost::asio::io_service &ios, const std::string &address = "")
      : Client(ios, ZMQ_PUB, "PubClient", address) {}
};

}  // namespace dispatch
}  // namespace a17
