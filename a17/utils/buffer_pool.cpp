#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <thread>
#include "buffer_pool.h"

namespace a17 {
namespace utils {

BufferPool::BufferPool(size_t bufferSize, uint16_t bufferCount, bool singleThreaded)
    : bufferSize_(bufferSize),
      bufferCount_(bufferCount),
      singleThreaded_(singleThreaded),
      ptrs_(bufferCount + 1) {
  poolSize_ = bufferCount_ * bufferSize_;
  pool_ = static_cast<uint8_t *>(::malloc(poolSize_));
  poolEnd_ = pool_ + poolSize_;

  head_ = 0;
  tail_ = bufferCount_;

  for (uint32_t i = 0; i < bufferCount_; i++) {
    ptrs_[i] = pool_ + i * bufferSize;
  }

  ptrs_[bufferCount_] = nullptr;
}

BufferPool::~BufferPool() {
  // TODO(pickledgator, kgreene) figure out why this crashes for some nodes
  if (pool_) {
    ::free(pool_);
  }
}

void *BufferPool::malloc() {
  if (head_ == tail_) return ::malloc(bufferSize_);

  void *ptr = ptrs_[head_];
  head_ = next(head_);
  return ptr;
}

void *BufferPool::calloc() {
  void *ptr = malloc();
  if (ptr) memset(ptr, 0, bufferSize_);
  return ptr;
}

void BufferPool::free(void *ptr) {
  if (ptr < pool_ || ptr >= poolEnd_) {
    ::free(ptr);
    return;
  }

  if (!singleThreaded_) tailMutex_.lock();
  ptrs_[tail_] = ptr;
  tail_ = next(tail_);
  if (!singleThreaded_) tailMutex_.unlock();
}

void BufferPool::zmqFree(void *ptr, void *hint) { static_cast<BufferPool *>(hint)->free(ptr); }

}  // namespace utils
}  // namespace a17
