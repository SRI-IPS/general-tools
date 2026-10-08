#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

namespace a17 {
namespace dispatch {

/// POSIX shared memory pool.
/// Each instance owns one named shm segment. The publisher creates a pool
/// per payload slot; the subscriber maps by name and never takes ownership.
///
/// Lifecycle (publisher side):
///   ShmPool pool("/dispatch_cam_0", sizeof(MyFrame));
///   void* ptr = pool.ptr();          // write payload here
///   pool.name(); pool.size();        // pass these in SharedMemoryHandle
///
/// Lifecycle (consumer side):
///   ShmPool view = ShmPool::map("/dispatch_cam_0", sizeof(MyFrame));
///   const void* ptr = view.ptr();   // read payload
///   // view destructor calls munmap only (no shm_unlink)
class ShmPool {
 public:
  /// Creates and owns a new POSIX shm segment.
  /// @param name  Must start with '/' (POSIX requirement).
  /// @param size  Bytes to map. Rounded up to the next page internally by the OS.
  ShmPool(const std::string& name, uint32_t size)
      : name_(name), size_(size), owner_(true) {
    // O_CREAT | O_EXCL ensures we get a fresh segment; fail loudly if stale.
    fd_ = ::shm_open(name_.c_str(), O_CREAT | O_RDWR, 0600);
    if (fd_ < 0) {
      throw std::runtime_error("shm_open(create) failed for " + name_ +
                               ": " + std::strerror(errno));
    }
    if (::ftruncate(fd_, static_cast<off_t>(size_)) < 0) {
      ::close(fd_);
      ::shm_unlink(name_.c_str());
      throw std::runtime_error("ftruncate failed for " + name_ +
                               ": " + std::strerror(errno));
    }
    mapMemory(PROT_READ | PROT_WRITE);
  }

  /// Non-owning view: maps an existing segment created by another ShmPool.
  /// The caller is responsible for ensuring the segment already exists.
  static ShmPool map(const std::string& name, uint32_t size) {
    return ShmPool(name, size, /*owner=*/false);
  }

  // Non-copyable; move-only so the fd/ptr transfer is explicit.
  ShmPool(const ShmPool&) = delete;
  ShmPool& operator=(const ShmPool&) = delete;

  ShmPool(ShmPool&& o) noexcept
      : name_(std::move(o.name_)),
        size_(o.size_),
        owner_(o.owner_),
        fd_(o.fd_),
        ptr_(o.ptr_) {
    o.fd_ = -1;
    o.ptr_ = nullptr;
  }

  ~ShmPool() {
    if (ptr_) {
      ::munmap(ptr_, size_);
      ptr_ = nullptr;
    }
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
    // Only the creating side removes the name from the kernel namespace.
    if (owner_) {
      ::shm_unlink(name_.c_str());
    }
  }

  inline void*        ptr()  const noexcept { return ptr_; }
  inline uint32_t     size() const noexcept { return size_; }
  inline const std::string& name() const noexcept { return name_; }

 private:
  /// Private constructor used by ShmPool::map() for non-owning views.
  ShmPool(const std::string& name, uint32_t size, bool owner)
      : name_(name), size_(size), owner_(owner) {
    fd_ = ::shm_open(name_.c_str(), O_RDONLY, 0);
    if (fd_ < 0) {
      throw std::runtime_error("shm_open(map) failed for " + name_ +
                               ": " + std::strerror(errno));
    }
    mapMemory(PROT_READ);
  }

  void mapMemory(int prot) {
    ptr_ = ::mmap(nullptr, size_, prot, MAP_SHARED, fd_, 0);
    if (ptr_ == MAP_FAILED) {
      ptr_ = nullptr;
      ::close(fd_);
      fd_ = -1;
      if (owner_) ::shm_unlink(name_.c_str());
      throw std::runtime_error("mmap failed for " + name_ +
                               ": " + std::strerror(errno));
    }
  }

  std::string name_;
  uint32_t    size_;
  bool        owner_;   // true → destructor calls shm_unlink
  int         fd_  = -1;
  void*       ptr_ = nullptr;
};

}  // namespace dispatch
}  // namespace a17
