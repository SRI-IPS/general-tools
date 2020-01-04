#pragma once

#include "server.h"

namespace a17 {
namespace dispatch {

// Every subscriber receives the same messages.
class Publisher : public Server {
 public:
  Publisher(boost::asio::io_service &ios, Directory &directory, const std::string &topic,
            const std::set<message_type> &outputTypes, Address address = Address())
      : Server(ios, ZMQ_PUB, "Publisher", directory, topic, std::set<message_type>{}, outputTypes,
               address) {}

  Publisher(boost::asio::io_service &ios, const std::string &address = "")
      : Server(ios, ZMQ_PUB, "Publisher", address) {}
};

}  // namespace dispatch
}  // namespace a17
