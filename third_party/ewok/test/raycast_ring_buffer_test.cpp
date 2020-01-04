#include "catch.hpp"
#include <ewok/raycast_ring_buffer.h>

TEST_CASE("raycast_ring_buffer template args") {
  const int POW = 6;

  WHEN("all template args") {
    using RaycastRingBufferBaseType = ewok::RaycastRingBuffer<POW,int16_t,float,int>;
    auto rrb  = RaycastRingBufferBaseType(0.1);
    RaycastRingBufferBaseType::Vector3i idx;
    REQUIRE(!rrb.isUpdated(idx));
    REQUIRE(!rrb.isOccupied(idx));
  }
  WHEN("only POW is supplied") {
    using RaycastRingBufferBaseType = ewok::RaycastRingBuffer<POW>;
    auto rrb  = RaycastRingBufferBaseType(0.1);
    RaycastRingBufferBaseType::Vector3i idx;
    REQUIRE(!rrb.isUpdated(idx));
    REQUIRE(!rrb.isOccupied(idx));
  }

}
