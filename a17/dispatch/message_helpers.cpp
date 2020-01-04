#include "message_helpers.h"
#include <sstream>

namespace a17 {
namespace dispatch {

long long getMicros() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

const unsigned long long idFromSmartMessage(const azmq::message_vector &msg_vec) {
  return *boost::asio::buffer_cast<const unsigned long long *>(msg_vec[0].cbuffer());
}

// dynamic output
std::ostream &operator<<(std::ostream &os, capnp::DynamicValue::Reader value) {
  switch (value.getType()) {
    case capnp::DynamicValue::VOID:
      os << "";
      break;
    case capnp::DynamicValue::BOOL:
      os << (value.as<bool>() ? "True" : "False");
      break;
    case capnp::DynamicValue::INT:
      os << value.as<int64_t>();
      break;
    case capnp::DynamicValue::UINT:
      os << value.as<uint64_t>();
      break;
    case capnp::DynamicValue::FLOAT:
      os << value.as<double>();
      break;
    case capnp::DynamicValue::TEXT:
      os << '\"' << value.as<capnp::Text>().cStr() << '\"';
      break;
    case capnp::DynamicValue::LIST: {
      os << "[";
      bool first = true;
      for (auto element : value.as<capnp::DynamicList>()) {
        if (first) {
          first = false;
        } else {
          os << ", ";
        }
        os << element;
      }
      os << "]";
      break;
    }
    case capnp::DynamicValue::ENUM: {
      auto enumValue = value.as<capnp::DynamicEnum>();
      KJ_IF_MAYBE(enumerant, enumValue.getEnumerant()) {
        os << enumerant->getProto().getName().cStr();
      }
      else {
        // Unknown enum value; output raw number.
        os << enumValue.getRaw();
      }
      break;
    }
    case capnp::DynamicValue::STRUCT: {
      os << "(";
      auto structValue = value.as<capnp::DynamicStruct>();
      bool first = true;
      for (auto field : structValue.getSchema().getFields()) {
        if (!structValue.has(field)) continue;
        if (first) {
          first = false;
        } else {
          os << " ";
        }
        os << field.getProto().getName().cStr() << "=" << structValue.get(field);
      }
      os << ")";
      break;
    }
    default:
      // There are other types, we aren't handling them.
      os << "?";
      break;
  }

  return os;
}

std::ostream &operator<<(std::ostream &out, const azmq::message_vector &message_vector) {
  for (int i = 0; i < static_cast<int>(message_vector.size()); i++) {
    if (i > 0) {
      out << ' ';
    }
    out << &message_vector[i];
  }
  return out;
}

std::ostream &operator<<(std::ostream &out, const azmq::message &message) {
  boost::asio::const_buffer buffer = message.cbuffer();
  const uint8_t *data = boost::asio::buffer_cast<const uint8_t *>(buffer);
  size_t size = boost::asio::buffer_size(buffer);
  out << streamMessage(data, size);
  return out;
}

std::string streamMessage(const uint8_t *buffer, size_t size) {
  std::ostringstream data_ostream;
  data_ostream << '[';
  size_t capSize = size < 255 ? size : 255;
  size_t i;
  for (i = 0; i < capSize; i++) {
    if (!isprint(buffer[i])) break;
  }
  if (i < size) {
    char hex[3];
    data_ostream << size << " bytes:";
    for (i = 0; i < capSize; i++) {
      data_ostream << ((i > 0 && (i % 8) == 0) ? '|' : ' ');
      sprintf(hex, "%02x", buffer[i]);
      data_ostream << hex;
    }
  } else {
    for (i = 0; i < capSize; i++) {
      data_ostream << buffer[i];
    }
  }

  if (capSize < size) {
    data_ostream << " (" << size - capSize << " more)";
  }

  data_ostream << ']';
  return data_ostream.str();
}

bool operator==(const azmq::message_vector &msg1, const azmq::message_vector &msg2) {
  if (msg1.size() != msg2.size()) return false;

  for (int i = 0; i < static_cast<int>(msg1.size()); i++) {
    boost::asio::const_buffer buf1 = msg1[i].cbuffer();
    boost::asio::const_buffer buf2 = msg2[i].cbuffer();
    size_t size = boost::asio::buffer_size(buf1);
    if (size != boost::asio::buffer_size(buf2)) return false;

    if (memcmp(boost::asio::buffer_cast<const void *>(buf1),
               boost::asio::buffer_cast<const void *>(buf2), size)) {
      return false;
    }
  }

  return true;
}

bool operator!=(const azmq::message_vector &msg1, const azmq::message_vector &msg2) {
  return !(msg1 == msg2);
}

}  // namespace dispatch
}  // namespace a17
