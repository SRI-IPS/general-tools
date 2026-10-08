#pragma once

#include <capnp/message.h>
#include <capnp/serialize.h>

#include <atomic>
#include <memory>
#include <string>

#include "a17/utils/buffer_pool.h"
#include "a17/capnp_msgs/shm.capnp.h"          // generated from shm.capnp
#include "shm_pool.h"
#include "smart_capnp_builder.h"
#include "socket.h"             // for Socket::send()

namespace a17 {
namespace dispatch {

/// Zero-copy publisher for local (same-machine) communication.
///
/// The caller:
///   1. Constructs ShmPublisher, which pre-allocates the shared memory.
///   2. Calls ptr() to get a raw write pointer and fills the payload.
///   3. Calls publish(socket) to send a tiny SharedMemoryHandle over ZMQ.
///
/// The segment persists until this object is destroyed, giving slow subscribers
/// time to map and read it before the memory is reclaimed.
///
/// Naming convention: names are built as "/dispatch_shm_<counter>" where
/// <counter> is a process-global atomic integer, guaranteeing uniqueness across
/// multiple ShmPublisher instances in the same process.
class ShmPublisher {
 public:
  /// @param size   Payload size in bytes. The shm segment is exactly this large.
  explicit ShmPublisher(uint32_t size)
      : size_(size), pool_(makeName(), size) {}

  /// Raw pointer into the shared memory segment. Write your payload here before
  /// calling publish().
  inline void* ptr() const noexcept { return pool_.ptr(); }

  /// Byte capacity of the shared region.
  inline uint32_t size() const noexcept { return size_; }

  /// Serializes a SharedMemoryHandle via Cap'n Proto and sends it over the
  /// supplied ZMQ socket. The socket is typically a dispatch::Publisher or any
  /// Socket subclass that exposes send(azmq::message_vector).
  ///
  /// @param socket  Any dispatch::Socket (Publisher, Server, …).
  /// @returns       ZMQ error_code; falsy on success.
  boost::system::error_code publish(Socket& socket) {
    SmartCapnpBuilder builder(capnpPool_);
    auto handle = builder.initRoot<::SharedMemoryHandle>();
    handle.setName(pool_.name());
    handle.setSize(size_);
    return socket.send(builder.getSmartMessage());
  }

 private:
  /// Generates a unique POSIX shm name for each instance.
  static std::string makeName() {
    static std::atomic<uint32_t> counter{0};
    return "/dispatch_shm_" + std::to_string(counter.fetch_add(1));
  }

  uint32_t              size_;
  ShmPool               pool_;
  a17::utils::BufferPool capnpPool_;  // reused for Cap'n Proto serialization
};

}  // namespace dispatch
}  // namespace a17
