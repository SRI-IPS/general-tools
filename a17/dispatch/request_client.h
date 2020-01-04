#pragma once

#include <string>

#include "boost/asio/deadline_timer.hpp"
#include "boost/asio/io_service.hpp"
#include "spdlog/spdlog.h"

#include "directory.h"
#include "handlers.h"
#include "socket.h"

namespace a17 {
namespace dispatch {

class RequestClient {
 public:
  RequestClient(boost::asio::io_service &ios, Directory &directory, const std::string &topic_name,
                ConnectionHandler connect_handler = ConnectionHandler(),
                ConnectionHandler disconnect_handler = ConnectionHandler());
  ~RequestClient();

  inline bool isConnected() { return socket_ != nullptr; }

  // TODO(kgreenek): Deprecate this and add a new request method that returns a future, takes a
  // capnp builder as an argument, and fulfills the promise with a capnp type reply.
  boost::system::error_code request(const azmq::message_vector &message,
                                    SmartMessageHandler reply_handler,
                                    ErrorHandler error_handler = ErrorHandler(),
                                    float timeout = -1.0);

 private:
  void connect(const DirectoryTopic &topic);
  void disconnect();
  void reconnect();
  void sendReplyToHandler(azmq::message_vector &message);
  void onDirectoryTopicsChanged(const std::string &topic_name, const GuidTopicMap &guid_topic_map);
  void cleanupRequest();

  std::shared_ptr<spdlog::logger> logger_ = std::make_shared<spdlog::logger>(
      "request_client", std::make_shared<spdlog::sinks::stdout_sink_mt>());

  boost::asio::io_service *ios_;
  a17::dispatch::Directory *directory_;
  std::string topic_name_;
  ConnectionHandler on_connect_;
  ConnectionHandler on_disconnect_;
  boost::asio::deadline_timer timeout_timer_;

  std::unique_ptr<Socket> socket_ = nullptr;
  SmartMessageHandler reply_handler_ = nullptr;
  bool waiting_for_receive_ = false;
  DirectoryTopic connected_topic_;
  std::string topic_observer_ref_;
};

}  // namespace dispatch
}  // namespace a17
