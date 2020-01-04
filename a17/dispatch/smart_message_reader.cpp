#include "smart_message_reader.h"

namespace a17 {
namespace dispatch {

// The first element will always be the capnp id, and the second element will always be a capnp
// struct. Any additional elements, if available will be raw buffers
SmartMessageReader::SmartMessageReader(azmq::message_vector &message_vector)
    : message_vector_(message_vector),
      reader_(new SmartCapnpReader(message_vector)),
      id_(idFromSmartMessage(message_vector)) {}

// pos = 1 will always be the capnp struct, additional buffers will be available starting at pos=2
void *SmartMessageReader::bufferAt(size_t pos, size_t &size) {
  auto buf = message_vector_[pos + 1].buffer();
  size = boost::asio::buffer_size(buf);
  return boost::asio::buffer_cast<void *>(buf);
}

}  // namespace dispatch
}  // namespace a17
