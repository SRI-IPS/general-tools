#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>

#include <capnp/message.h>
#include <capnp/schema.h>
#include <capnp/serialize.h>
#include <spdlog/spdlog.h>

#include "defs.h"
#include "message_helpers.h"

namespace a17 {
namespace dispatch {

/// Wraps capnproto reader with convenience constructors for dealing with azmq messages.
/// This reader knows the capnproto id of the underlying capnproto class, and verifies in
/// getRoot() that the correct class is passed in the template.
class SmartCapnpReader {
 public:
  SmartCapnpReader(azmq::message &message, unsigned long long id);
  SmartCapnpReader(azmq::message_vector &message_vector);

  inline unsigned long long id() const { return id_; }

  template <typename RootType>
  typename RootType::Reader getRoot() {
    unsigned long long requestedId = idOf<RootType>();
    if (id_ != requestedId) {
      throw std::runtime_error("Type in getRoot<> (" + std::to_string(requestedId) +
                               ") doesn't match id in message (" + std::to_string(id_) + ")");
    }
    return reader_.getRoot<RootType>();
  }

 private:
  /// Returns a word-aligned buffer from the azmq message. This function makes a copy of the
  /// data in message, because zmq does not guarantee the data is malloced on a word-boundary
  /// on all platforms (particularly on 64-bit systems).
  static kj::Array<capnp::word> CapnpBufferFromAzmqMessage(const azmq::message &message);

  const unsigned long long id_;
  /// Need to keep a reference to the buffer so that it stays alive in memory while the reader
  /// is being used.
  kj::Array<capnp::word> buffer_;
  capnp::FlatArrayMessageReader reader_;
};

}  // namespace dispatch
}  // namespace a17
