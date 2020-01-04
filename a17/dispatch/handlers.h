#pragma once

#include <functional>
#include <string>

#include "smart_capnp_builder.h"

namespace a17 {
namespace dispatch {

// TODO(kgreenek): Deprecate these azmq-based methods and use the higher-level methods below.
using SmartMessageHandler = std::function<void(azmq::message_vector &)>;
using ReplySender = std::function<void(const azmq::message_vector &)>;
using ReplySmartMessageHandler = std::function<void(azmq::message_vector &request, ReplySender)>;
using ErrorHandler = std::function<void(const boost::system::error_code &)>;

using ConnectionHandler = std::function<void(const std::string &topic_name)>;

template <typename T>
using CapnpMessageHandler = std::function<void(const T &)>;

using ExceptionHandler = std::function<void(const std::exception &)>;

using ReplyServerCapnpReplyHandler = std::function<void(const SmartCapnpBuilder &)>;

template <typename RequestT>
using ReplyServerCapnpRequestHandler =
    std::function<void(const RequestT &, ReplyServerCapnpReplyHandler)>;

using ReplyServerExceptionHandler =
    std::function<void(const std::exception &, ReplyServerCapnpReplyHandler)>;


}  // namespace dispatch
}  // namespace a17
