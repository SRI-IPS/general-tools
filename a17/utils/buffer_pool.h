#ifndef A17_UTILS_BUFFER_POOL_H_
#define A17_UTILS_BUFFER_POOL_H_

#include <vector>
#include <functional>
#include <mutex>

namespace a17 {
namespace utils {

// Default buffer size for BufferPool.
// For processes with message sizes larger or smaller than this, construct BufferPool with a more
// optimal buffer size for better performance.
const uint16_t DEFAULT_POOL_BUFFER_COUNT = 64;
const size_t DEFAULT_POOL_BUFFER_SIZE = 64 * 8;  // 8 == sizeof(capnp::word)

/**
 * multiple-producer single-consumer memory pool for send buffers.
 * Only the free() method is multiple-thread-safe (multiple producers) while the malloc() method is
 * not (single consumer).
 * This enables our use case with zmqq, where one Node thread allocates send buffers, and the Node
 * thread or a zmq thread may free them.
 * Locking behavior in free() can be turned off by constructing with singleThreaded = true.
 * For performance reasons, there aren't any guards in the free() method. It is possible to corrupt
 * the pool by freeing duplicate or invalid pointers.
 * If the pool runs out of buffers, it will return buffers allocated by OS malloc(). This is to
 * prevent out of memory errors that could crash the process.
 */
class BufferPool {
 public:
  /**
   * Choose a buffer size that is a little larger than the max expected message size. Powers of 2
   * are preferred for memory alignment.
   *
   * Choose a buffer count that leaves enough room for enough buffers in transit between the Node
   * main thread and the zmq send thread. The higher the rate of message send, the more buffers
   * needed.
   *
   * @param bufferSize size of a buffer in bytes
   * @param bufferCount total number of buffers in the pool
   * @param singleThreaded whether to bypass locking in single threaded usage
   */
  BufferPool(size_t bufferSize = DEFAULT_POOL_BUFFER_SIZE,
             uint16_t bufferCount = DEFAULT_POOL_BUFFER_COUNT, bool singleThreaded = false);
  ~BufferPool();

  inline void *pool() const { return pool_; }
  inline size_t bufferSize() const { return bufferSize_; }
  inline uint16_t bufferCount() const { return bufferCount_; }
  inline bool empty() const { return head_ == tail_; }
  inline bool full() const { return next(tail_) == head_; }
  inline uint16_t remaining() const {
    return tail_ > head_ ? tail_ - head_ : bufferCount_ - (head_ - tail_) + 1;
  }

  void *malloc();
  void *calloc();
  void free(void *ptr);

  // Static free method that conforms to zmq_free_ffn.
  static void zmqFree(void *ptr, void *hint);

 private:
  inline uint16_t next(uint16_t pos) const { return (pos + 1) % (bufferCount_ + 1); }

  uint8_t *pool_;
  uint8_t *poolEnd_;
  size_t poolSize_;
  size_t bufferSize_;
  uint16_t bufferCount_;
  bool singleThreaded_;

  std::vector<void *> ptrs_;
  uint16_t head_;
  uint16_t tail_;
  std::mutex tailMutex_;
};

}  // namespace utils
}  // namespace a17

#endif  // A17_UTILS_BUFFER_POOL_H_
