#include "catch.hpp"
#include <ewok/ed_ring_buffer.h>

static const int POW = 6;
static const int N = (1 << POW);

TEST_CASE("ed_ring_buffer template args") {
  WHEN("all template args") {
    ewok::EuclideanDistanceRingBuffer<POW,int16_t,float,int> rrb(0.1, 1.0);
    auto val = rrb.getVolumeCenter();
  }
  WHEN("only POW is supplied") {
    ewok::EuclideanDistanceRingBuffer<POW> rrb(0.1, 1.0);
    auto val = rrb.getVolumeCenter();
  }
}
