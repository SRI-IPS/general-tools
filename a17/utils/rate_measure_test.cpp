#include "catch.hpp"
#include "rate_measure.h"
#include "nearest_interval_bin.h"

TEST_CASE("exact samples", "[measure]") {
  a17::utils::RateMeasure measure(std::chrono::seconds(5));
  measure.mark(1000000000);
  measure.mark(2000000000);
  measure.mark(3000000000);
  measure.mark(4000000000);

  REQUIRE(measure.sampleCount() == 3);
  REQUIRE(measure.frequency() == 1.0);
  REQUIRE(measure.jitter() == 0);
  REQUIRE(measure.ticks() == 4);

  measure.mark(5000000000);
  measure.mark(6000000000);
  measure.mark(7000000000);
  measure.mark(8000000000);

  REQUIRE(measure.sampleCount() == 5);
  REQUIRE(measure.frequency() == 1.0);
  REQUIRE(measure.jitter() == 0);
  REQUIRE(measure.ticks() == 8);
}

TEST_CASE("variable samples", "[measure]") {
  a17::utils::RateMeasure measure(std::chrono::seconds(5));
  measure.mark(1000000000);
  measure.mark(2001000000);
  measure.mark(3002000000);
  measure.mark(4003000000);
  measure.mark(5004000000);
  measure.mark(6005000000);

  REQUIRE(measure.frequency() - 0.999000999 < .00000001);
  REQUIRE(measure.jitter() == 1000.0);
  REQUIRE(measure.ticks() == 6);
}

TEST_CASE("wrap around", "[measure]") {
  a17::utils::RateMeasure measure(std::chrono::seconds(5));

  // no matter what comes before, the only last 5 intervals should be computed
  measure.mark(1000000000L);
  measure.mark(2000000000L);
  measure.mark(3000000000L);
  measure.mark(4000000000L);
  measure.mark(5000000000L);
  measure.mark(6000000000L);
  measure.mark(7000000000L);
  measure.mark(8000000000L);
  measure.mark(9000000000L);

  REQUIRE(measure.sampleCount() == 5);
  REQUIRE(measure.frequency() == 1.0);
  REQUIRE(measure.jitter() == 0);
  REQUIRE(measure.ticks() == 9);
}

TEST_CASE("sizes", "[measure]") {
  a17::utils::RateMeasure measure(std::chrono::seconds(5));

  measure.markSize(50, 1);

  // only last 3 will count
  measure.markSize(60, 2);
  measure.markSize(70, 3);
  measure.markSize(80, 4);

  REQUIRE(measure.size() == 70);
  REQUIRE(measure.ticks() == 4);
}

TEST_CASE("target intervals", "[measure]") {
  uint8_t index = 0;
  REQUIRE(a17::utils::NearestIntervalBin(1472, index) == 1333);
  REQUIRE(a17::utils::NearestIntervalBin(1477772, index) == 1333333);
  REQUIRE(a17::utils::NearestIntervalBin(77777777, index) == 66666666);
  REQUIRE(a17::utils::NearestIntervalBin(2390483, index) == 2500000);
  REQUIRE(a17::utils::NearestIntervalBin(1, index) == 1);
  REQUIRE(a17::utils::NearestIntervalBin(0, index) == 0);
}
