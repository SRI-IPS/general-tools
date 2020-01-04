#include "catch.hpp"
#include "buffer_pool.h"

TEST_CASE("malloc", "[pool]") {
  a17::utils::BufferPool pool(64, 4);
  uint8_t *start = static_cast<uint8_t *>(pool.pool());

  // allocate entire pool
  void *ptr = pool.malloc();
  REQUIRE(ptr == start);
  REQUIRE(!pool.empty());

  ptr = pool.malloc();
  REQUIRE(ptr == start + 64);
  REQUIRE(!pool.empty());

  ptr = pool.malloc();
  REQUIRE(ptr == start + 128);
  REQUIRE(!pool.empty());

  ptr = pool.malloc();
  REQUIRE(ptr == start + 192);

  // pool should be empty
  REQUIRE(pool.empty());

  // when empty, pool should allocate buffers from OS
  void *osPtr1 = pool.malloc();
  REQUIRE(osPtr1 != nullptr);
  bool outsidePool = osPtr1 < start || osPtr1 >= start + 256;
  REQUIRE(outsidePool);

  void *osPtr2 = pool.malloc();
  REQUIRE(osPtr2 != nullptr);
  outsidePool = osPtr2 < start || osPtr2 >= start + 256;
  REQUIRE(outsidePool);

  // free in different order
  pool.free(start + 64);
  REQUIRE(!pool.empty());

  pool.free(start + 192);
  pool.free(start);

  // allocations should reflect order of previous frees
  ptr = pool.malloc();
  REQUIRE(ptr == start + 64);
  ptr = pool.malloc();
  REQUIRE(ptr == start + 192);
  ptr = pool.malloc();
  REQUIRE(ptr == start);

  // pool should free buffers from OS
  pool.free(osPtr1);
  pool.free(osPtr2);

  pool.free(start);
  pool.free(start + 64);
  pool.free(start + 128);
  pool.free(start + 192);
}

TEST_CASE("calloc", "[pool]") {
  a17::utils::BufferPool pool(64, 4);

  uint8_t *ptr = static_cast<uint8_t *>(pool.calloc());
  for (int i = 0; i < 64; i++) {
    REQUIRE(ptr[i] == 0);
  }

  pool.free(ptr);
}
