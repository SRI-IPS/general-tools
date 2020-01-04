#include "smart_capnp_reader.h"

#include <assert.h>

namespace a17 {
namespace dispatch {

SmartCapnpReader::SmartCapnpReader(azmq::message &message, unsigned long long id)
    : id_(id),
      buffer_(CapnpBufferFromAzmqMessage(message)),
      reader_(buffer_) {}

// It's possible that you could send a message_vector with size < 2, which would assert
SmartCapnpReader::SmartCapnpReader(azmq::message_vector &message_vector)
    : id_(idFromSmartMessage(message_vector)),
      buffer_(CapnpBufferFromAzmqMessage(message_vector[1])),
      reader_(buffer_) {}

kj::Array<capnp::word> SmartCapnpReader::CapnpBufferFromAzmqMessage(const azmq::message &message) {
  auto word_aligned_buffer = kj::heapArray<capnp::word>(message.size() / sizeof(capnp::word));
  memcpy(word_aligned_buffer.begin(), message.data(), word_aligned_buffer.asBytes().size());
  return word_aligned_buffer;
}

}  // namespace dispatch
}  // namespace a17
