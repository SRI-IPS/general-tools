#pragma once

#include "defs.h"

namespace a17 {
namespace dispatch {

class Socket;

// listener socket (for receiving messages)
class Listener {
 private:
  Socket &socket_;
  SmartMessageHandler listener_handler_;
  SmartMessageHandler wrapped_handler_;
  ErrorHandler error_handler_;

 public:
  Listener(Socket &socket, SmartMessageHandler handler, ErrorHandler error);

  Listener(Listener &&other)
      : socket_(other.socket_),
        listener_handler_(std::move(other.listener_handler_)),
        wrapped_handler_(std::move(other.wrapped_handler_)),
        error_handler_(std::move(other.error_handler_)) {};

  virtual void onMessage(azmq::message_vector &message);

  // Return the previous handler and replace it with the new handler.
  void setMessageHandler(SmartMessageHandler handler);

  // Construct a handler that calls a list of handlers with the same message.
  static SmartMessageHandler all(const std::vector<SmartMessageHandler> &handlers);
};

}  // namespace dispatch
}  // namespace a17
