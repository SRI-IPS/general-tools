@0xf4b83230daa6a23e;  # File-level unique ID (generated via `capnp id`)

# Metadata envelope published over ZeroMQ so a subscriber can
# locate and map the POSIX shared memory segment holding the payload.
struct SharedMemoryHandle @0x9a3c7fed8b210456 {
  # POSIX shm name as passed to shm_open(). Includes the leading '/'.
  name @0 :Text;

  # Byte length of the mapped region, matching the mmap() length used by the pool.
  size @1 :UInt32;
}
