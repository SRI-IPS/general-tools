#pragma once

#include <future>
#include <set>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/common.h>
#if SPDLOG_VERSION >= 10000
  #include <spdlog/sinks/stdout_color_sinks.h>
#endif
#include <boost/asio/signal_set.hpp>

#include "a17/utils/buffer_pool.h"
#include "a17/utils/repeater.h"

#include "directory.h"
#include "message_helpers.h"
#include "publisher.h"
#include "reply_server.h"
#include "request_client.h"
#include "smart_capnp_builder.h"
#include "smart_capnp_reader.h"
#include "smart_message_reader.h"
#include "subscriber.h"
#include "topic.h"

namespace a17 {
namespace dispatch {

template <typename T>
using CapnpMessageHandler = std::function<void(const T &)>;

using ExceptionHandler = std::function<void(const std::exception &)>;

using ReplyServerCapnpReplyHandler = std::function<void(const SmartCapnpBuilder &)>;

template <typename RequestT>
using ReplyServerCapnpRequestHandler =
    std::function<void(const RequestT &, ReplyServerCapnpReplyHandler)>;

using ReplyServerExceptionHandler =
    std::function<void(const std::exception &, ReplyServerCapnpReplyHandler)>;

class Node {
 public:
  explicit Node(const std::string &name = "Node");
  ~Node();

  /// Creates a new Publisher.
  /// IMPORTANT: The Publisher object returned has a reference to the node instance, so the node
  /// MUST be kept alive as long as the returned object is alive.
  /// @param topic The topic that the publisher will publish to.
  template <typename T>
  std::shared_ptr<Publisher> registerCapnpPublisher(const Topic &topic) {
    // For some reason, std::make_shared() isn't able to deduce which constructor to use here.
    return std::shared_ptr<Publisher>{new Publisher{ios_, directory_, topic.str(), {typeOf<T>()}}};
  }

  /// Creates a new Subscriber.
  /// IMPORTANT: The Subscriber object returned has a reference to the node instance, so the node
  /// MUST be kept alive as long as the returned object is alive.
  /// @param topic The topic that the subscriber will subscribe to.
  /// @param handler A callback that is called whenever a message is received.
  /// @param error_handler A callback that is called whenever there is an error parsing a received
  ///   message, or some other application-level error occurs that triggers an exception.
  template <typename T>
  std::shared_ptr<Subscriber> registerCapnpSubscriber(
      const Topic &topic, CapnpMessageHandler<typename T::Reader> handler,
      ExceptionHandler error_handler) {
    auto zmq_handler = [handler, error_handler](azmq::message_vector &msg_vec) {
      try {
        auto reader = a17::dispatch::SmartCapnpReader(msg_vec);
        auto request = reader.getRoot<T>();
        handler(request);
      } catch (const std::exception &e) {
        error_handler(e);
      }
    };
    return std::make_shared<Subscriber>(ios_, directory_, topic.str(), zmq_handler);
  }

  /// Creates a new Subscriber with a default error_handler that simply logs the error.
  /// IMPORTANT: The Subscriber object returned has a reference to the node instance, so the node
  /// MUST be kept alive as long as the returned object is alive.
  /// @param topic The topic that the subscriber will subscribe to.
  /// @param handler A callback that is called whenever a message is received.
  template <typename T>
  std::shared_ptr<Subscriber> registerCapnpSubscriber(
      const Topic &topic, CapnpMessageHandler<typename T::Reader> handler) {
    return registerCapnpSubscriber<T>(topic, handler, [this, topic](const std::exception &e) {
      this->logger_->warn("Unhandled exception in subscriber {}: {}", topic.str(), e.what());
    });
  }

  // TODO(kgreenek): Deprecate this in favor of using a proper zmq Router (i.e. a service). The
  // problem with using a zmq PubClient/ReplyServer is that it assumes only one client and one
  // server. If two clients send a request to the same server at the same time, then the ReplyServer
  // will get into a bad state, which is VERY BAD. It assumes there is 1 request, followed by 1
  // reply.
  /// Creates a new ReplyServer.
  /// Use the Node::newCapnpMessageBuilder() method to create a builder for the reply.
  /// IMPORTANT: The ReplyServer object returned has a reference to the node instance, so the node
  /// MUST be kept alive as long as the returned object is alive.
  /// @param topic The topic that the subserver will subscribe to.
  /// @param handler A callback that is called whenever a message is received. Its first argument
  ///   is the request, and the second argument is a callback that MUST be called with a reply.
  ///   if the reply callback is not called, then the ReplyServer will be in a permenantly
  ///   unrecoverable state, so it must be called.
  /// @param error_handler This is called whenever there is an error parsing the request, or other
  ///   application-level error that triggers an exception. This MUST be implemented, and the caller
  ///   MUST call the given function with a reply (any reply) or the server will no longer be able
  ///   to receive requests.
  template <typename RequestT, typename ReplyT>
  std::shared_ptr<ReplyServer> registerCapnpReplyServerAsync(
      const Topic &topic, ReplyServerCapnpRequestHandler<typename RequestT::Reader> request_handler,
      ReplyServerExceptionHandler error_handler) {
    auto zmq_request_handler = [request_handler, error_handler](azmq::message_vector &msg_vec,
                                                                ReplySender send_reply_fn) {
      auto reply_handler = [send_reply_fn](const SmartCapnpBuilder &reply_builder) {
        send_reply_fn(reply_builder.getSmartMessage());
      };
      try {
        auto reader = a17::dispatch::SmartCapnpReader(msg_vec);
        auto request = reader.getRoot<RequestT>();
        request_handler(request, reply_handler);
      } catch (const std::exception &e) {
        error_handler(e, reply_handler);
      }
    };
    // For some reason, std::make_shared() isn't able to deduce which constructor to use here.
    return std::shared_ptr<ReplyServer>{new ReplyServer{ios_,
                                                        directory_,
                                                        topic.str(),
                                                        {typeOf<RequestT>()},
                                                        {typeOf<ReplyT>()},
                                                        zmq_request_handler}};
  }

  /// Creates a new RequestClient.
  /// Use the Node::newCapnpMessageBuilder() method to create a builder for the request.
  /// @param topic The topic that requests will be sent to.
  template <typename RequestT, typename ReplyT>
  std::shared_ptr<RequestClient> newRequestClient(const Topic &topic) {
    // For some reason, std::make_shared() isn't able to deduce which constructor to use here.
    return std::shared_ptr<RequestClient>{new RequestClient{ios_, directory_, topic.str()}};
  }

  /// Creates a new Repeater that runs on the same io_service as the node. The specified operation
  /// is scheduled to run every millis.
  /// @param millis Delay between calls to operation.
  /// @param operation The function that will be called on a timer.
  /// @param auto_start Whether to start the repeater immediately on creation.
  std::shared_ptr<a17::utils::Repeater> registerRepeater(int millis,
                                                         a17::utils::RepeatOperation operation,
                                                         bool auto_start = true) {
    return std::make_shared<a17::utils::Repeater>(ios_, millis, operation, auto_start);
  }

  /// Returns a builder for creating capnp requests. It uses an internal memory allocation pool to
  /// prevent heap thrashing and speed up requests -- especially if sending at a high rate.
  SmartCapnpBuilder newCapnpMessageBuilder() { return SmartCapnpBuilder{pool_}; }

  inline boost::asio::io_service &service() { return ios_; }
  inline std::shared_ptr<spdlog::logger> logger() { return logger_; }

  /// Returns a fully-qualified topic from the given sub-topic. This is a helper to automatically
  /// prefix the given topic with the command-line-configured device_name and the node_name.
  Topic topic(const std::string &topic);

  // Starts the node on a background thread. Asynchronous.
  void start();
  // The main run loop of the node. Blocking.
  virtual void run() { ios_.run(); }
  // Signals the thread to stop. (the thead may not stop immediately)
  virtual void stop();
  inline void post(std::function<void()> operation) { ios_.post(operation); }

  inline const std::string &name() { return name_; }
  inline Directory &directory() { return directory_; }
  inline a17::utils::BufferPool &pool() { return pool_; }
  inline bool signaledShutdown() const { return signaled_shutdown_; }

 protected:
  boost::asio::io_service ios_;
  std::string name_;
  a17::utils::BufferPool pool_;
  Directory directory_;
  bool signaled_shutdown_ = false;
  std::shared_ptr<spdlog::logger> logger_;
  boost::asio::signal_set signals_;
  virtual void signal(const boost::system::error_code &ec, int signalNumber);

 private:
  std::future<void> future_;
};

}  // namespace dispatch
}  // namespace a17
