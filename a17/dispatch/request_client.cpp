#include "request_client.h"

namespace a17 {
namespace dispatch {

namespace {

/// @returns The newest topic within guid_topic_map.
/// @throws std::runtime_error If guid_topic_map is empty.
DirectoryTopic NewestTopic(const GuidTopicMap &guid_topic_map) {
  if (guid_topic_map.empty()) {
    throw std::runtime_error("Empty guid_topic_map");
  }
  auto newest_topic = guid_topic_map.begin()->second;
  for (const auto &guid_topic : guid_topic_map) {
    auto &topic = guid_topic.second;
    if (topic.timestamp > newest_topic.timestamp) {
      newest_topic = topic;
    }
  }
  return newest_topic;
}

}  // namespace

RequestClient::RequestClient(boost::asio::io_service &ios, Directory &directory,
                             const std::string &topic_name, ConnectionHandler connect_handler,
                             ConnectionHandler disconnect_handler)
    : ios_(&ios),
      directory_(&directory),
      topic_name_(topic_name),
      on_connect_(connect_handler),
      on_disconnect_(disconnect_handler),
      timeout_timer_(ios) {
  topic_observer_ref_ =
      directory.observe(topic_name, bind2(&RequestClient::onDirectoryTopicsChanged));
}

RequestClient::~RequestClient() { directory_->unobserve(topic_name_, topic_observer_ref_); }

// TODO (adas): For Boost 1.66+ timeout argument needs to be an integer
boost::system::error_code RequestClient::request(const azmq::message_vector &message,
                                                 SmartMessageHandler reply_handler,
                                                 ErrorHandler error_handler, float timeout) {
  reply_handler_ = reply_handler;
  if (waiting_for_receive_) {
    logger_->warn("Ignoring request, still waiting on previous reply");
    return boost::system::errc::make_error_code(
        boost::system::errc::connection_already_in_progress);
  }
  boost::system::error_code ec;
  if (!isConnected()) {
    logger_->error("ReplyServer not connected");
    return boost::system::errc::make_error_code(boost::system::errc::not_connected);
  }
  socket_->send(message, ec);
  if (ec) {
    logger_->error("Error sending request: {}", ec.message());
    return ec;
  }
  socket_->receive(bind1(&RequestClient::sendReplyToHandler), error_handler);
  waiting_for_receive_ = true;
  if (timeout > 0) {
    // For Boost 1.66+ timeout needs to be converted to integer
    timeout_timer_.expires_from_now(boost::posix_time::seconds(int(timeout)));
    timeout_timer_.async_wait([this, timeout, error_handler](const boost::system::error_code &ec) {
      if (ec == boost::asio::error::operation_aborted) {
        return;
      }
      logger_->warn("Request response has timed out after {}s", timeout);
      cleanupRequest();
      // It is necessary to totally tear down and re-create the socket here, because otherwise the
      // next request sent will fail, since it is still waiting for a reply from the previous
      // request.
      reconnect();
      error_handler(boost::system::errc::make_error_code(boost::system::errc::timed_out));
    });
  }
  return ec;
}

void RequestClient::sendReplyToHandler(azmq::message_vector &message) {
  cleanupRequest();
  reply_handler_(message);
}

void RequestClient::cleanupRequest() {
  timeout_timer_.cancel();
  waiting_for_receive_ = false;
}

void RequestClient::onDirectoryTopicsChanged(const std::string &topic_name,
                                             const GuidTopicMap &guid_topic_map) {
  if (guid_topic_map.empty()) {
    disconnect();
    return;
  }
  connect(NewestTopic(guid_topic_map));
}

void RequestClient::connect(const DirectoryTopic &topic) {
  if (isConnected()) {
    if (topic == connected_topic_) {
      return;
    }
    disconnect();
  }
  logger_->info("RequestClient [{0}] @ {1}", topic.name, topic.address);
  connected_topic_ = topic;
  socket_ = std::make_unique<Socket>(*ios_, ZMQ_REQ, "RequestClient");
  socket_->socket().connect(topic.address);
}

void RequestClient::disconnect() {
  if (!isConnected()) {
    return;
  }
  logger_->info("RequestClient [{0}] !@ {1}", connected_topic_.name, connected_topic_.address);
  socket_->socket().disconnect(connected_topic_.address);
  socket_ = nullptr;
}

void RequestClient::reconnect() {
  if (!isConnected()) {
    return;
  }
  auto topic = connected_topic_;
  disconnect();
  connect(topic);
}

}  // namespace dispatch
}  // namespace a17
