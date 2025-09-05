#include <boost/asio/signal_set.hpp>

#include "a17/utils/buffer_pool.h"
#include "a17/utils/repeater.h"

#include "handlers.h"
#include "directory.h"
#include "message_helpers.h"
#include "publisher.h"
#include "reply_server.h"
#include "request_client.h"
#include "subscriber.h"
#include "topic.h"

namespace a17 {
namespace dispatch {

class Node {
 public:
  /// @param topic The topic that the publisher will publish to.
  template <typename T>
  std::shared_ptr<Publisher> registerCapnpPublisher(const Topic &topic) {
    return std::make_shared<Publisher>(ios_, directory_, topic.str(), std::set<message_type>{typeOf<T>()});
  }

  /// Creates a new Subscriber.
        error_handler(e, reply_handler);
      }
    };
    return std::make_shared<ReplyServer>(ios_, directory_, topic.str(),
                                         std::set<message_type>{typeOf<RequestT>()},
                                         std::set<message_type>{typeOf<ReplyT>()},
                                         zmq_request_handler);
  }

  /// Creates a new RequestClient.
  /// Use the Node::newCapnpMessageBuilder() method to create a builder for the request.
  /// @param topic The topic that requests will be sent to.
  template <typename RequestT, typename ReplyT>
  std::shared_ptr<RequestClient> newRequestClient(const Topic &topic) {
    return std::make_shared<RequestClient>(ios_, directory_, topic.str());
  }

  /// Creates a new Repeater that runs on the same io_service as the node. The specified operation

