#pragma once

#include "client.h"
#include "listener.h"

namespace a17 {
namespace dispatch {

// Every subscriber receives the same messages.
class Subscriber : public Client, public Listener {
 private:
  std::set<message_type> filters_;

 public:
  Subscriber(boost::asio::io_service &ios, Directory &directory, const std::string &publisherTopic,
             SmartMessageHandler handler, ErrorHandler error = ErrorHandler(),
             ConnectionHandler connectHandler = ConnectionHandler(),
             ConnectionHandler disconnectHandler = ConnectionHandler(),
             const std::set<message_type> &filters = {""})
      : Client(ios, ZMQ_SUB, "Subscriber", directory, publisherTopic),
        Listener(*this, handler, error),
        filters_(filters) {
    applyFilters();
  }

  Subscriber(boost::asio::io_service &ios, const std::string &publisherAddress,
             SmartMessageHandler handler, ErrorHandler error = ErrorHandler(),
             const std::set<message_type> &filters = {""})
      : Client(ios, ZMQ_SUB, "Subscriber", publisherAddress),
        Listener(*this, handler, error),
        filters_(filters) {
    applyFilters();
  }

  inline void setMessageHandler(SmartMessageHandler handler) {
    Listener::setMessageHandler(handler);
  }

 private:
  void applyFilters();
};

}  // namespace dispatch
}  // namespace a17
