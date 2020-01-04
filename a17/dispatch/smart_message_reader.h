#pragma once

#include <iostream>
#include <vector>
#include <assert.h>
#include <spdlog/spdlog.h>
#include "defs.h"
#include "message_helpers.h"
#include "smart_capnp_reader.h"

namespace a17 {
namespace dispatch {

/// Wrapper for reading and manipulating built SmartMessages (azmq::message_vector) with extra raw
/// data buffers
class SmartMessageReader {
 public:
  SmartMessageReader(azmq::message_vector &message_vector);
  inline size_t size() { return message_vector_.size() - 1; }
  inline unsigned long long id() { return id_; }

  // Hide the base class method in order to assert that the passed-in template argument
  // matches the message id
  template <typename RootType>
  typename RootType::Reader getRoot() {
    unsigned long long requestedId = idOf<RootType>();
    if (id_ != requestedId) {
      throw std::runtime_error(
          "Requested message type in SmartMessageReader.getRoot<> (" + std::to_string(requestedId) +
          ") doesn't match received message type (" + std::to_string(id_) + ")");
    }
    if (!reader_) {
      throw std::runtime_error("SmartMessageReader.getRoot<>, reader pointer is null!");
    }

    return reader_->getRoot<RootType>();
  }

  // Return the raw buffer pointer and size for the message part at the given position.
  void *bufferAt(size_t pos, size_t &size);

 private:
  azmq::message_vector &message_vector_;
  std::unique_ptr<SmartCapnpReader> reader_;
  unsigned long long id_;
};

}  // namespace dispatch
}  // namespace a17
