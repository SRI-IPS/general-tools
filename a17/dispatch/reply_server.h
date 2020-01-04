#pragma once

#include <stack>

#include "handlers.h"
#include "server.h"

namespace a17 {
namespace dispatch {

// client sends messages, server replies with message

class ReplyServer : public Server {
 public:
  ReplyServer(boost::asio::io_service &ios, Directory &directory, const std::string &topic,
              const std::set<message_type> &inputTypes, const std::set<message_type> &outputTypes,
              ReplySmartMessageHandler handler, ErrorHandler error = ErrorHandler(),
              Address address = Address())
      : Server(ios, ZMQ_REP, "ReplyServer", directory, topic, inputTypes, outputTypes, address),
        request_handler_(bind1(&ReplyServer::onRequest)),
        reply_sender_(bind1(&ReplyServer::sendReply)),
        reply_handler_(handler),
        error_handler_(error) {
    receive(request_handler_, error_handler_);
  }

  ReplyServer(boost::asio::io_service &ios, const std::string &address,
              ReplySmartMessageHandler handler, ErrorHandler error = ErrorHandler())
      : Server(ios, ZMQ_REP, "ReplyServer", address),
        request_handler_(bind1(&ReplyServer::onRequest)),
        reply_sender_(bind1(&ReplyServer::sendReply)),
        reply_handler_(handler),
        error_handler_(error) {
    receive(request_handler_, error_handler_);
  }

  //! Return the current handler and replace it with the new handler.
  inline void setReplyHandler(ReplySmartMessageHandler handler) { reply_handler_ = handler; }

 protected:
  void onRequest(azmq::message_vector &message);
  void sendReply(const azmq::message_vector &reply);

 private:
  SmartMessageHandler request_handler_;
  ReplySender reply_sender_;
  ReplySmartMessageHandler reply_handler_;
  ErrorHandler error_handler_;
};

}  // namespace dispatch
}  // namespace a17
