#include "reply_server.h"

namespace a17 {
namespace dispatch {

void ReplyServer::onRequest(azmq::message_vector &message) {
  if (logger_) logger_->trace("{0} onMessage", log_name_);
  // to ensure that the entire reply server socket does not go down if we catch an exception in the
  // application layer, send a blank reply to free the request client and reset the reply server for
  // another message to be received
  // NOTE: this seems to work ok in practice, but does not protect against hard crashes of the reply
  // server, which would leave the request client in a non-recoverable state if the reply server
  // crashes without sending a reply and then restarts
  try {
    reply_handler_(message, reply_sender_);
  } catch (const std::exception &e) {
    if (logger_) logger_->warn("Caught exception in {} onRequest()\n{}", log_name_, e.what());
    // ensure that a reply is always sent to maintain the socket!
    send(azmq::message_vector());
    // stage the next receive despite not being able to process the failed one
    receive(request_handler_, error_handler_);
  }
}

void ReplyServer::sendReply(const azmq::message_vector &reply) {
  boost::system::error_code ec;
  send(reply, ec);
  if (ec) {
    if (logger_) logger_->error("{0} reply error: {1}", log_name_, ec.message());
  }

  receive(request_handler_, error_handler_);
}

}  // namespace dispatch
}  // namespace a17
