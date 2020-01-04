#pragma once

#include "handlers.h"
#include "listener.h"
#include "server.h"

namespace a17 {
namespace dispatch {

// treats zmq subscriber as the server (use with pubclient)
class SubServer : public Server, public Listener {
 private:
  std::set<message_type> filters_;

 public:
 // Discovery constructor
  SubServer(boost::asio::io_service &ios, Directory &directory, const std::string &topic,
            const std::set<message_type> &inputTypes, SmartMessageHandler handler,
            ErrorHandler error = ErrorHandler(), const std::set<message_type> &filters = {""},
            Address address = Address())
      : Server(ios, ZMQ_SUB, "SubServer", directory, topic, inputTypes, std::set<message_type>{},
               address),
        Listener(*this, handler, error),
        filters_(filters) {
    applyFilters();
  }

  // Non-discovery constructor
  SubServer(boost::asio::io_service &ios, const std::string &address, SmartMessageHandler handler,
            ErrorHandler error = ErrorHandler(), const std::set<message_type> &filters = {""})
      : Server(ios, ZMQ_SUB, "SubServer", address),
        Listener(*this, handler, error),
        filters_(filters) {
    applyFilters();
  }

 private:
  void applyFilters();
};

}  // namespace dispatch
}  // namespace a17
