#include <catch.hpp>

#include <cmath>

#include "angle_unroller.h"

namespace a17 {
namespace maths {
namespace test {

TEST_CASE("Angle Unroller Test", "[task_gen_dispatch_node]") {
  auto angle_unroller = AngleUnroller{M_PI / 2};

  SECTION("Positive roll-over") {
    REQUIRE(angle_unroller.unrolledAngle() == Approx(M_PI / 2));
    REQUIRE(angle_unroller.unrollAngle(M_PI / 2) == Approx(M_PI / 2));
    REQUIRE(angle_unroller.unrollAngle(3 * M_PI / 4) == Approx(3 * M_PI / 4));
    REQUIRE(angle_unroller.unrollAngle(-3 * M_PI / 4) == Approx(5 * M_PI / 4));
    REQUIRE(angle_unroller.unrollAngle(-M_PI / 4) == Approx(7 * M_PI / 4));
  }

  SECTION("Negative roll-over") {
    REQUIRE(angle_unroller.unrolledAngle() == Approx(M_PI / 2));
    REQUIRE(angle_unroller.unrollAngle(-M_PI / 2) == Approx(-M_PI / 2));
    REQUIRE(angle_unroller.unrollAngle(-3 * M_PI / 4) == Approx(-3 * M_PI / 4));
    REQUIRE(angle_unroller.unrollAngle(3 * M_PI / 4) == Approx(-5 * M_PI / 4));
    REQUIRE(angle_unroller.unrollAngle(M_PI / 4) == Approx(-7 * M_PI / 4));
  }
}

}  // namespace test
}  // namespace maths
}  // namespace a17
