#include "catch.hpp"
#include <ewok/ring_buffer_base.h>

TEST_CASE("ring_buffer_base template args") {
  const int POW = 6;

  WHEN("int16t,float") {
    using RingBufferBaseType = ewok::RingBufferBase<POW,int16_t,float>;

    auto rbb  = RingBufferBaseType(0.1);
    auto expected = -(int(pow(2,POW)/2))*RingBufferBaseType::Vector3i::Ones();

    RingBufferBaseType::Vector3i offset;
    rbb.getOffset(offset);
    REQUIRE(offset == expected);
  }

  WHEN("int32, double") {
    using RingBufferBaseType = ewok::RingBufferBase<POW,int32_t,double>;
    auto rbb  = RingBufferBaseType(0.1);
    auto expected = -(int(pow(2,POW)/2))*RingBufferBaseType::Vector3i::Ones();

    RingBufferBaseType::Vector3i offset;
    rbb.getOffset(offset);
    REQUIRE(offset == expected);
  }

  WHEN("int, double") {
    using RingBufferBaseType = ewok::RingBufferBase<POW,int,double>;
    auto rbb  = RingBufferBaseType(0.1);
    auto expected = -(int(pow(2,POW)/2))*RingBufferBaseType::Vector3i::Ones();

    RingBufferBaseType::Vector3i offset;
    rbb.getOffset(offset);
    REQUIRE(offset == expected);
  }

  WHEN("double, double") {
    using RingBufferBaseType = ewok::RingBufferBase<POW,double,double>;
    auto rbb  = RingBufferBaseType(0.1);
    auto expected = -(int(pow(2,POW)/2))*RingBufferBaseType::Vector3i::Ones();

    RingBufferBaseType::Vector3i offset;
    rbb.getOffset(offset);
    REQUIRE(offset == expected);
  }

}
