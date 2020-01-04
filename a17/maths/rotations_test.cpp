#include <catch.hpp>

#include "rotations.h"

namespace a17 {
namespace maths {
namespace test {

TEST_CASE("Rotations ENU to NED Test", "[Rotations]") {
  auto rotations = Rotations();
  SECTION("3D Points") {
    auto point_zero = Eigen::Vector3d{0.0, 0.0, 0.0};
    auto point_onetwothree = Eigen::Vector3d{1.0, 2.0, 3.0};
    REQUIRE(rotations.pointEnuToNed(point_zero)
                .isApprox(point_zero, 1e-6));
    REQUIRE(rotations.pointEnuToNed(point_onetwothree)
                .isApprox(Eigen::Vector3d{2.0, 1.0, -3.0}, 1e-6));
  }

  SECTION("Rotations") {
    auto quaternion_w = Eigen::Quaterniond{1.0, 0.0, 0.0, 0.0};
    auto quaternion_x = Eigen::Quaterniond{0.0, 1.0, 0.0, 0.0};
    auto quaternion_y = Eigen::Quaterniond{0.0, 0.0, 1.0, 0.0};
    auto quaternion_z = Eigen::Quaterniond{0.0, 0.0, 0.0, 1.0};
    auto quaternion_random = Eigen::Quaterniond{-0.61678802, 0.05557776, 0.43890737, 0.651033};

    // Identity quaternion is not changed.
    REQUIRE(rotations.rotEnuToNed(quaternion_w)
                .isApprox(Eigen::Quaterniond{1.0, 0.0, 0.0, 0.0}));
    // The x-axis becomes the y-axis (EAST becomes NORTH).
    REQUIRE(rotations.rotEnuToNed(quaternion_x)
                .isApprox(Eigen::Quaterniond{0.0, 0.0, 1.0, 0.0}, 1e-6));
    // The y-axis becomes the x-axis (NORTH becomes EAST).
    REQUIRE(rotations.rotEnuToNed(quaternion_y)
                .isApprox(Eigen::Quaterniond{0.0, 1.0, 0.0, 0.0}, 1e-6));
    // The z-axis becomes the -z axis (UP becomes DOWN).
    REQUIRE(rotations.rotEnuToNed(quaternion_z)
                .isApprox(Eigen::Quaterniond{0.0, 0.0, 0.0, -1.0}, 1e-6));
    REQUIRE(
        rotations.rotEnuToNed(quaternion_random)
            .isApprox(Eigen::Quaterniond{-0.61678802, 0.43890737, 0.05557776, -0.651033}, 1e-6));
  }
}

}  // namespace test
}  // namespace maths
}  // namespace a17
