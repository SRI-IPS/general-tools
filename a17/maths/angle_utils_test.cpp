#include <catch.hpp>

#include <cmath>

#include "angle_utils.h"

namespace a17 {
namespace maths {
namespace test {

TEST_CASE("Angle Utils Test", "[angle_utils]") {
  SECTION("WrapToPi") {
    REQUIRE(WrapToPi(0.0) == Approx(0.0));

    REQUIRE(WrapToPi(M_PI / 2) == Approx(M_PI / 2));
    REQUIRE(WrapToPi(-M_PI / 2) == Approx(-M_PI / 2));
    REQUIRE(WrapToPi(3 * M_PI / 2) == Approx(-M_PI / 2));
    REQUIRE(WrapToPi(-3 * M_PI / 2) == Approx(M_PI / 2));

    REQUIRE(WrapToPi(M_PI) == Approx(M_PI));
    REQUIRE(WrapToPi(-M_PI) == Approx(-M_PI));
    REQUIRE(WrapToPi(3 * M_PI) == Approx(M_PI));
    REQUIRE(WrapToPi(-3 * M_PI) == Approx(-M_PI));
    REQUIRE(WrapToPi(5 * M_PI) == Approx(M_PI));
    REQUIRE(WrapToPi(-5 * M_PI) == Approx(-M_PI));

    REQUIRE(WrapToPi(4 * M_PI) == Approx(0.0));
    REQUIRE(WrapToPi(-4 * M_PI) == Approx(0.0));
    REQUIRE(WrapToPi(6 * M_PI) == Approx(0.0));
    REQUIRE(WrapToPi(-6 * M_PI) == Approx(0.0));
  }

  SECTION("AngleDiff") {
    REQUIRE(AngleDiff(0.0, 0.0) == Approx(0.0));
    REQUIRE(AngleDiff(0.0, M_PI) == Approx(M_PI));
    REQUIRE(AngleDiff(0.0, -M_PI) == Approx(M_PI));
    REQUIRE(AngleDiff(-M_PI, -M_PI) == Approx(0.0));
    REQUIRE(AngleDiff(-M_PI, 0.0) == Approx(M_PI));
    REQUIRE(AngleDiff(M_PI, 0.0) == Approx(M_PI));
    REQUIRE(AngleDiff(-M_PI, M_PI) == Approx(0.0));
    REQUIRE(AngleDiff(-2 * M_PI, M_PI) == Approx(M_PI));
  }
}

TEST_CASE("Degrees to radians") {
  REQUIRE(DegreesToRadians(45) == M_PI / 4.0);
  REQUIRE(DegreesToRadians(90) == M_PI / 2.0);
  REQUIRE(DegreesToRadians(180) == M_PI);
  REQUIRE(DegreesToRadians(270) == 3 * M_PI / 2.0);
  REQUIRE(DegreesToRadians(0) == 0.0);
}

TEST_CASE("Radians to degrees") {
  REQUIRE(RadiansToDegrees(M_PI / 4.0) == 45);
  REQUIRE(RadiansToDegrees(M_PI / 2.0) == 90);
  REQUIRE(RadiansToDegrees(M_PI) == 180);
  REQUIRE(RadiansToDegrees(3 * M_PI / 2.0) == 270);
  REQUIRE(RadiansToDegrees(0.0) == 0.0);
}

}  // namespace test
}  // namespace maths
}  // namespace a17
