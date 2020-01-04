#include <catch.hpp>

#include <iostream>
#include <limits>

#include "float_utils.h"

namespace a17 {
namespace maths {
namespace test {

TEST_CASE("Float Utils", "[float_utils]") {

  auto infinite = std::numeric_limits<float>::max();

  SECTION("FloatEq Basic") {
    REQUIRE(FloatEq(0.0, 0.0));
    REQUIRE(FloatEq(-1.0, -1.0));
    REQUIRE(FloatEq(1.0, 1.0));
    REQUIRE(FloatEq(-0.1, -0.1));
    REQUIRE(FloatEq(0.1, 0.1));
    REQUIRE(FloatEq(0.2, 1 / std::sqrt(5) / std::sqrt(5)));

    REQUIRE(!FloatEq(-0.1, -0.10001));
    REQUIRE(FloatEq(-0.1, -0.100000001));
  }

  SECTION("FloatEq 1 To Infinite") {
    for (double i = 1.223345; i < std::numeric_limits<float>::max(); i *= 2.01) {
      // Make these volatile to prevent the compiler from optimizing out the multiplication.
      volatile auto sqrt_i = std::sqrt(i);
      volatile auto i_with_error = sqrt_i * std::sqrt(i);
      REQUIRE(FloatEq(i, i_with_error));
    }
  }

  SECTION("FloatEq -1 To -Infinite") {
    for (double i = -1.211345; i > std::numeric_limits<float>::max(); i *= 2.01) {
      // Make these volatile to prevent the compiler from optimizing out the multiplication.
      volatile auto sqrt_i = std::sqrt(i);
      volatile auto i_with_error = sqrt_i * std::sqrt(i);
      REQUIRE(FloatEq(i, i_with_error));
    }
  }

  SECTION("FloatEq Near Infinite") {
    REQUIRE(FloatEq(infinite, infinite));
    REQUIRE(FloatEq(-infinite, -infinite));
    REQUIRE(FloatEq(infinite / 8 * 3 * 2, infinite / 4 * 3));
    REQUIRE(FloatEq(-infinite / 4 * 3, -infinite / 8 * 3 * 2));
  }

  SECTION("Overflow") {
    REQUIRE(!FloatEq(infinite / 4 * 3, -infinite / 4 * 3));
    REQUIRE(!FloatEq(-infinite / 4 * 3, infinite / 4 * 3));
  }
}

}  // namespace test
}  // namespace maths
}  // namespace a17
