#include "Eigen/Core"
#include "Eigen/Dense"
#include "catch.hpp"

#include "matrix_utils.h"

namespace a17 {
namespace maths {
namespace test {

TEST_CASE("Adjugate", "[maths]") {
  // Compare the Adjugate function with correct values from Wolfram Alpha.
  constexpr auto kPrecision = 1e-8;

  SECTION("Invertible matrix") {
    Eigen::Matrix3d mat;
    mat << 1, 2, 3, 2, 4, 5, 3, 5, 6;

    Eigen::Matrix3d correct_adjugate;
    correct_adjugate << -1, 3, -2, 3, -3, 1, -2, 1, 0;

    REQUIRE((maths::Adjugate(mat) - correct_adjugate).norm() < kPrecision);
  }

  SECTION("Another invertible matrix") {
    Eigen::Matrix3d mat;
    mat << 1.3, 0, 3.2, 2, 4.9, 51, 31, 0.5, -6;

    Eigen::Matrix3d correct_adjugate;
    correct_adjugate << -54.9, 1.6, -15.68, 1593, -107, -59.9, -150.9, -0.65, 6.37;

    REQUIRE((maths::Adjugate(mat) - correct_adjugate).norm() < kPrecision);
  }

  SECTION("Another, bigger invertible matrix") {
    Eigen::Matrix4d mat;
    mat << 1, 2, 2, 1, 3, 4, 4, 3, 1, 2, 3, 4, -1, -2, -3, 5;

    Eigen::Matrix4d correct_adjugate;
    correct_adjugate << 36, -18, 2, 2, -45, 9, 12, -6, 18, 0, -12, 6, 0, 0, -2, -2;

    REQUIRE((maths::Adjugate(mat) - correct_adjugate).norm() < kPrecision);
  }


  SECTION("Invertible dynamic size matrix") {
    Eigen::MatrixXd mat;
    mat.resize(3, 3);
    mat << 1, 2, 3, 2, 4, 5, 3, 5, 6;

    Eigen::Matrix3d correct_adjugate;
    correct_adjugate << -1, 3, -2, 3, -3, 1, -2, 1, 0;

    REQUIRE((maths::Adjugate(mat) - correct_adjugate).norm() < kPrecision);
  }

  SECTION("Non-invertible matrix") {
    Eigen::Matrix3d mat;
    mat << 1, 2, 3, 4, 5, 6, 7, 8, 9;

    Eigen::Matrix3d correct_adjugate;
    correct_adjugate << -3, 6, -3, 6, -12, 6, -3, 6, -3;

    REQUIRE((maths::Adjugate(mat) - correct_adjugate).norm() < kPrecision);
  }

  SECTION("Non-square dynamic matrix") {
    Eigen::MatrixXd mat(2, 3);
    mat << 1, 2, 3, 4, 5, 6;

    REQUIRE_THROWS(maths::Adjugate(mat));
  }

  // Non-square static sized matrix static asserts, so it won't even compile.
}

TEST_CASE("SkewSymmetricMatrix", "[maths]") {
  // Test that multiplying by the skew symmetric matrix is the same as
  // computing the cross product.
  constexpr auto kPrecision = 1e-8;

  srand(0);
  for (int i = 0; i < 100; ++i) {
    Eigen::Vector3d vec = Eigen::Vector3d::Random();
    Eigen::Matrix3d mat = SkewSymmetricMatrix(vec);

    for (int j = 0; j < 100; ++j) {
      Eigen::Vector3d other = Eigen::Vector3d::Random();

      Eigen::Vector3d result1 = vec.cross(other);
      Eigen::Vector3d result2 = mat * other;

      REQUIRE((result1 - result2).norm() < kPrecision);
    }
  }
}

}  // namespace test
}  // namespace maths
}  // namespace a17
