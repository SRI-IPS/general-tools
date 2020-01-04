#pragma once

#include <iostream>
#include <vector>
#include <assert.h>
#include <chrono>
#include <spdlog/spdlog.h>
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <capnp/schema.h>
#include <capnp/dynamic.h>
#include "defs.h"
#include "a17/utils/buffer_pool.h"

namespace a17 {
namespace dispatch {

// TODO(pickledgator): move this to utils
long long getMicros();

// Returns the message type of the templated capnproto class
template <typename T>
unsigned long long idOf() {
  capnp::StructSchema schema = capnp::Schema::from<T>();
  return schema.getProto().getId();
}

// returns id from a zmq message
const unsigned long long idFromSmartMessage(const azmq::message_vector &msg_vec);

template <typename T>
std::string typeOf() {
  return std::to_string(idOf<T>());
}

// TODO(kgreenek): define these functions in a sensible location instead of sticking them here.
std::string streamMessage(const uint8_t *buffer, size_t size);
std::ostream &operator<<(std::ostream &out, const azmq::message_vector &message_vector);
std::ostream &operator<<(std::ostream &out, const azmq::message &message_part);

}  // namespace dispatch
}  // namespace a17
