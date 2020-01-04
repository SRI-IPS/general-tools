#include "smart_capnp_builder.h"

namespace a17 {
namespace dispatch {

SmartCapnpBuilder::SmartCapnpBuilder(a17::utils::BufferPool &pool)
    : next_size_(pool.bufferSize() / sizeof(capnp::word)),
      pool(&pool),
      own_first_segment_(true),
      returned_first_segment_(false),
      first_segment_(nullptr) {}

SmartCapnpBuilder::~SmartCapnpBuilder() noexcept(false) {
  if (!invalidated_ && returned_first_segment_) {
    if (own_first_segment_) {
      if (is_first_segment_pooled_) {
        pool->free(first_segment_);
      } else {
        free(first_segment_);
      }
    }

    KJ_IF_MAYBE(s, more_segments_) {
      MoreSegments *more = s->get();
      for (int i = 0; i < static_cast<int>(more->segments.size()); i++) {
        if (more->is_segment_pooled[i]) {
          pool->free(more->segments[i]);
        } else {
          free(more->segments[i]);
        }
      }
    }
  }
}

kj::ArrayPtr<capnp::word> SmartCapnpBuilder::allocateSegment(uint minimumSize) {
  if (!returned_first_segment_ && !own_first_segment_) {
    kj::ArrayPtr<capnp::word> result =
        kj::arrayPtr(reinterpret_cast<capnp::word *>(first_segment_), next_size_);
    if (result.size() >= minimumSize) {
      returned_first_segment_ = true;
      return result;
    }
    own_first_segment_ = true;
  }

  // use the buffer pool if requested segment size matches buffer size
  uint size = kj::max(minimumSize, next_size_);
  bool usePool = pool && size * sizeof(capnp::word) <= pool->bufferSize();
  void *result = nullptr;

  if (usePool) {
    result = pool->calloc();
    if (!result) {
      if (spdlog::get("Socket")) {
        spdlog::get("Socket")->warn("allocateSegment(): buffer pool failed using calloc");
      }
    }
  }

  if (!result) {
    result = calloc(size, sizeof(capnp::word));
    if (!result) {
      throw std::runtime_error("calloc failed");
    }
  }

  if (!returned_first_segment_) {
    first_segment_ = result;
    returned_first_segment_ = true;
    is_first_segment_pooled_ = usePool;
  } else {
    MoreSegments *segments;
    KJ_IF_MAYBE(s, more_segments_) { segments = *s; }
    else {
      auto newSegments = kj::heap<MoreSegments>();
      segments = newSegments;
      more_segments_ = mv(newSegments);
    }

    segments->segments.push_back(result);
    segments->is_segment_pooled.push_back(usePool);
  }

  return kj::arrayPtr(reinterpret_cast<capnp::word *>(result), size);
}

azmq::message SmartCapnpBuilder::build() const {
  auto segments = ((SmartCapnpBuilder *)this)->getSegmentsForOutput();
  size_t messageSize = capnp::computeSerializedSizeInWords(segments) * sizeof(capnp::word);

  // use buffer pool for the final message, otherwise malloc
  if (pool && messageSize <= pool->bufferSize()) {
    void *buf = pool->malloc();
    if (buf) {
      azmq::nocopy_t nocopy;
      azmq::message message(nocopy, boost::asio::mutable_buffer(buf, messageSize), pool,
                            a17::utils::BufferPool::zmqFree);
      buildSmartMessage(message, segments);
      return message;
    } else {
      if (spdlog::get("Socket")) {
        spdlog::get("Socket")->warn("build(): buffer pool failed using malloc");
      }
    }
  }

  azmq::message message(messageSize);
  buildSmartMessage(message, segments);
  return message;
}

// Copy directly into an azmq message
void SmartCapnpBuilder::buildSmartMessage(
    azmq::message &message, kj::ArrayPtr<const kj::ArrayPtr<const capnp::word>> &segments) const {
  boost::asio::mutable_buffer buf = message.buffer();
  uint32_t *table = boost::asio::buffer_cast<uint32_t *>(buf);

  table[0] = segments.size() - 1;

  for (uint i = 0; i < segments.size(); i++) {
    table[i + 1] = segments[i].size();
  }

  // Set padding byte
  if (segments.size() % 2 == 0) {
    table[segments.size() + 1] = 0;
  }

  capnp::word *dst = boost::asio::buffer_cast<capnp::word *>(buf) + segments.size() / 2 + 1;

  for (auto &segment : segments) {
    memcpy(dst, segment.begin(), segment.size() * sizeof(capnp::word));
    dst += segment.size();
  }
}

}  // namespace dispatch
}  // namespace a17
