#pragma once

#include <functional>
#include <stdexcept>
#include <string>

#include "a17/capnp_msgs/shm.capnp.h"        // generated from shm.capnp
#include "shm_pool.h"
#include "smart_capnp_reader.h"

namespace a17 {
namespace dispatch {

/// Callback type handed to ShmSubscriber.
/// @param ptr   Read-only pointer into the shared memory region.
/// @param size  Byte length of the region (same as SharedMemoryHandle.size).
using ShmCallback = std::function<void(const void* ptr, uint32_t size)>;

/// Zero-copy subscriber for local (same-machine) communication.
///
/// Drop this into any existing ZMQ message-receive callback to transparently
/// upgrade it to shared-memory reads:
///
///   ShmSubscriber shmSub;
///   Subscriber sub(ios, pub_address, [&](azmq::message_vector& msg) {
///     shmSub.receive(msg, [](const void* ptr, uint32_t sz) {
///       // ptr is valid only inside this lambda
///       processFrame(ptr, sz);
///     });
///   });
///
/// The shared memory segment is mapped, the callback is invoked, and the
/// mapping is immediately unmapped. No memory is retained between calls.
class ShmSubscriber {
 public:
  ShmSubscriber() = default;

  /// Deserialises the SharedMemoryHandle carried in `msg_vec`, maps the shared
  /// memory segment read-only, calls `cb`, then immediately unmaps.
  ///
  /// @throws std::runtime_error if the message type does not match
  ///         SharedMemoryHandle or if shm_open/mmap fails.
  void receive(azmq::message_vector& msg_vec, ShmCallback cb) {
    // Decode the Cap'n Proto envelope — SmartCapnpReader validates the type id.
    SmartCapnpReader reader(msg_vec);
    auto handle = reader.getRoot<::SharedMemoryHandle>();

    const std::string name(handle.getName().cStr());
    const uint32_t    size = handle.getSize();

    // Map read-only; ShmPool::map() throws if the segment doesn't exist.
    ShmPool view = ShmPool::map(name, size);

    // Invoke user callback while the mapping is live.
    cb(view.ptr(), size);

    // view destructor munmaps here — no persistent mapping, no leak.
  }
};

}  // namespace dispatch
}  // namespace a17
