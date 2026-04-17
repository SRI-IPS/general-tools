// shm_test.cpp — Catch2 unit tests for the shared-memory pub/sub layer.
// Link against: dispatch_shm (shm_pool.h, shm_publisher.h, shm_subscriber.h,
//               shm.capnp generated code), dispatch, Catch2.
//
// Run: ./dispatch_unittests "[shm]"

#include "catch.hpp"

#include <cstring>
#include <string>
#include <thread>
#include <atomic>

#include "shm_pool.h"
#include "shm_subscriber.h"

// Bring in the Cap'n Proto generated header so we can build handles manually
// in the low-level ShmPool tests.
#include "shm.capnp.h"
#include "smart_capnp_builder.h"
#include "smart_capnp_reader.h"
#include "a17/utils/buffer_pool.h"

namespace a17 {
namespace dispatch {
namespace test {

// ---------------------------------------------------------------------------
// 1. ShmPool — allocation and data integrity
// ---------------------------------------------------------------------------
TEST_CASE("ShmPool creates and maps a segment", "[shm]") {
  const uint32_t kSize = 256;
  ShmPool pool("/dispatch_test_basic", kSize);

  // Pointer must be non-null and writable.
  REQUIRE(pool.ptr() != nullptr);
  REQUIRE(pool.size() == kSize);
  REQUIRE(pool.name() == "/dispatch_test_basic");

  // Write a sentinel pattern through the owning mapping.
  std::memset(pool.ptr(), 0xAB, kSize);
}
// pool destructor calls shm_unlink here — segment is gone.


TEST_CASE("ShmPool::map reads data written by owner", "[shm]") {
  const uint32_t    kSize    = 512;
  const std::string kName    = "/dispatch_test_rw";
  const uint8_t     kPattern = 0xCD;

  // Owner writes the pattern.
  {
    ShmPool owner(kName, kSize);
    std::memset(owner.ptr(), kPattern, kSize);

    // Non-owning view maps the same segment while the owner is still alive.
    ShmPool view = ShmPool::map(kName, kSize);
    REQUIRE(view.ptr() != nullptr);

    const auto* bytes = static_cast<const uint8_t*>(view.ptr());
    bool allMatch = true;
    for (uint32_t i = 0; i < kSize; ++i) {
      if (bytes[i] != kPattern) { allMatch = false; break; }
    }
    CHECK(allMatch);
  }
  // owner gone → shm_unlink called; view was destroyed first (LIFO scope).
}


// ---------------------------------------------------------------------------
// 2. SharedMemoryHandle — Cap'n Proto round-trip
// ---------------------------------------------------------------------------
TEST_CASE("SharedMemoryHandle serialises and deserialises correctly", "[shm]") {
  a17::utils::BufferPool capnpPool;

  // Build a handle.
  SmartCapnpBuilder builder(capnpPool);
  auto handle = builder.initRoot<::SharedMemoryHandle>();
  handle.setName("/dispatch_test_capnp");
  handle.setSize(1024);

  azmq::message_vector msg = builder.getSmartMessage();

  // Read it back.
  SmartCapnpReader reader(msg);
  auto h = reader.getRoot<::SharedMemoryHandle>();

  CHECK(std::string(h.getName().cStr()) == "/dispatch_test_capnp");
  CHECK(h.getSize() == 1024u);
}


// ---------------------------------------------------------------------------
// 3. Full publish / subscribe cycle (in-process, no ZMQ socket needed)
// ---------------------------------------------------------------------------
// We simulate the transport layer: the publisher serialises the handle into an
// azmq::message_vector; the subscriber deserialises and maps. This exercises
// the complete data path — from ShmPool write through Cap'n Proto encode/decode
// to the final memory read — without requiring live ZMQ sockets in CI.
TEST_CASE("ShmSubscriber maps and reads data via SharedMemoryHandle", "[shm]") {
  const uint32_t kPayloadSize = 128;
  const char     kMagic[]     = "Hello, shared memory!";

  // Create the segment directly through ShmPool — name() is already public.
  ShmPool owner("/dispatch_test_roundtrip", kPayloadSize);
  std::memcpy(owner.ptr(), kMagic, sizeof(kMagic));

  // Build the Cap'n Proto envelope exactly as ShmPublisher::publish() would.
  a17::utils::BufferPool capnpPool;
  SmartCapnpBuilder builder(capnpPool);
  {
    auto h = builder.initRoot<::SharedMemoryHandle>();
    h.setName(owner.name());   // public accessor — no test-only API needed
    h.setSize(owner.size());
  }
  azmq::message_vector wire = builder.getSmartMessage();

  // ShmSubscriber decodes the handle, maps read-only, fires the callback.
  ShmSubscriber sub;
  bool callbackFired = false;

  sub.receive(wire, [&](const void* ptr, uint32_t sz) {
    callbackFired = true;
    REQUIRE(sz == kPayloadSize);
    CHECK(std::memcmp(ptr, kMagic, sizeof(kMagic)) == 0);
  });

  CHECK(callbackFired);
}


// ---------------------------------------------------------------------------
// 4. Cross-thread data integrity
// ---------------------------------------------------------------------------
TEST_CASE("ShmPool is safe across threads with separate map views", "[shm]") {
  const uint32_t kSize    = 4096;
  const uint64_t kValue   = 0xDEADBEEFCAFEBABEull;
  const int      kReaders = 8;

  ShmPool owner("/dispatch_test_threads", kSize);
  // Write the magic value into the first 8 bytes.
  std::memcpy(owner.ptr(), &kValue, sizeof(kValue));

  std::atomic<int> successCount{0};
  std::vector<std::thread> readers;

  for (int i = 0; i < kReaders; ++i) {
    readers.emplace_back([&]() {
      // Each thread creates its own independent mapping.
      ShmPool view = ShmPool::map("/dispatch_test_threads", kSize);
      uint64_t got = 0;
      std::memcpy(&got, view.ptr(), sizeof(got));
      if (got == kValue) successCount.fetch_add(1);
    });
  }

  for (auto& t : readers) t.join();

  CHECK(successCount.load() == kReaders);
}

}  // namespace test
}  // namespace dispatch
}  // namespace a17
