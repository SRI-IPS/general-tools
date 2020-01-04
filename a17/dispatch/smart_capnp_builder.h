#pragma once

#include <iostream>
#include <vector>
#include <assert.h>
#include <spdlog/spdlog.h>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <capnp/schema.h>
#include <capnp/dynamic.h>
#include "defs.h"
#include "a17/utils/buffer_pool.h"
#include "message_helpers.h"

namespace a17 {
namespace dispatch {

/**
 * A smart message consists of several azmq::message components. The first component of a smart
message is an azmq::message that contains the id of the capnp structure that is packaged as the 2nd
azmq::message. Any additional azmq::messages that are appended onto the azmq::message_vector are
assumed to be raw data buffers (non capnp structures).
 *
 * id: uint64_t identifier of message part (from capnproto class id)
 * typeOf<T>: returns the std::string representation of the capnp id
 * idOf<T>: returns the uint64_t representation fo the capnp id
 *
 * azmq::message_vector: zmq multipart message (std::vector<azmq::message>)
 * azmq::message: single message of zmq multipart message
 * SmartCapnpReader: id-aware version of capnproto::MessageReader
 * SmartCapnpBuilder: id-aware version of capnproto::MessageBuilder
 * SmartMessageReader: Wrapper for reading and manipulating built SmartMessages
(azmq::message_vector) with extra raw data buffers
**/

/**
 * Capnproto MessageBuilder that extracts the capnproto id from the class passed
 * to initRoot().
 * SmartCapnpBuilder msg;
 * ::Builder = msg.initRoot<my::capnproto::class>();
 * msg.setStuff(...);
 * socket.send(msg, ec);
 * ::Builder = msg.initRoot<my::other::class>();
 * msg.setOtherStuff(...);
 * socket.send(msg, ec);
 */
class SmartCapnpBuilder : public capnp::MessageBuilder {
 public:
  // Builder that uses the given memory pool to allocate segments.
  explicit SmartCapnpBuilder(a17::utils::BufferPool &pool);

  SmartCapnpBuilder(SmartCapnpBuilder &other) = delete;

  SmartCapnpBuilder(SmartCapnpBuilder &&other)
      : id_(other.id_),
        next_size_(other.next_size_),
        pool(other.pool),
        own_first_segment_(other.own_first_segment_),
        returned_first_segment_(other.returned_first_segment_),
        first_segment_(other.first_segment_),
        is_first_segment_pooled_(other.is_first_segment_pooled_),
        more_segments_(std::move(other.more_segments_)) {
    invalidated_ = true;
  }

  virtual ~SmartCapnpBuilder() noexcept(false);

  inline unsigned long long id() const { return id_; }

  // Builds a zmq message part from the builder data
  azmq::message build() const;

  // Builds a complete zmq multipart message where the first message is the id and the second
  // message is populated from the builder We call this message since we hide the fact that we are
  // appending an id message at the front of the message vector
  inline azmq::message_vector getSmartMessage() const {
    return azmq::message_vector{azmq::message(boost::asio::const_buffer(&id_, sizeof(id_))),
                                build()};
  }
  // Helper operator to allow passing the builder directly to socket.send()
  inline operator azmq::message_vector() const { return getSmartMessage(); }

  virtual kj::ArrayPtr<capnp::word> allocateSegment(uint minimumSize) override;

  // Hide the base class method in order to force extraction of the capnproto class id
  template <typename RootType>
  typename RootType::Builder initRoot() {
    id_ = idOf<RootType>();
    return capnp::MessageBuilder::initRoot<RootType>();
  }

  // Hide the base class method in order to force extraction of the capnproto class id
  template <typename RootType>
  void setRoot(const typename RootType::Reader &root) {
    id_ = idOf<RootType>();
    capnp::MessageBuilder::setRoot(root);
  }

 private:
  void buildSmartMessage(azmq::message &message,
                         kj::ArrayPtr<const kj::ArrayPtr<const capnp::word>> &segments) const;

  unsigned long long id_ = 0;
  uint next_size_;
  a17::utils::BufferPool *pool;

  bool own_first_segment_;
  bool returned_first_segment_;
  void *first_segment_;
  bool is_first_segment_pooled_;

  bool invalidated_ = false;

  struct MoreSegments {
    std::vector<void *> segments;
    std::vector<bool> is_segment_pooled;
    MoreSegments() {}
    MoreSegments(MoreSegments &&other)
        : segments(std::move(other.segments)), is_segment_pooled(std::move(other.is_segment_pooled)) {}
  };

  kj::Maybe<kj::Own<MoreSegments>> more_segments_;
};

}  // namespace dispatch
}  // namespace a17
