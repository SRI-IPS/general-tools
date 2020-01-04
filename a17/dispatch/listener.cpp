#include <functional>

#include "handlers.h"
#include "listener.h"
#include "socket.h"

namespace a17 {
namespace dispatch {

Listener::Listener(Socket &socket, SmartMessageHandler handler, ErrorHandler error)
    : socket_(socket),
      listener_handler_(bind1(&Listener::onMessage)),
      wrapped_handler_(std::move(handler)),
      error_handler_(error) {
  socket_.receive(listener_handler_, error_handler_);
}

void Listener::onMessage(azmq::message_vector &message) {
  if (socket_.logger_) socket_.logger_->trace("{0} listener onMessage", socket_.log_name_);
  wrapped_handler_(message);
  socket_.receive(listener_handler_, error_handler_);
}

SmartMessageHandler Listener::all(const std::vector<SmartMessageHandler> &handlers) {
  return [handlers](azmq::message_vector &message) {
    for (auto handler = handlers.cbegin(); handler != handlers.cend(); handler++) {
      (*handler)(message);
    }
  };
}

void Listener::setMessageHandler(SmartMessageHandler handler) { wrapped_handler_ = handler; }

}  // namespace dispatch
}  // namespace a17
