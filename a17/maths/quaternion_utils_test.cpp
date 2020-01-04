#include "Eigen/Core"
#include "catch.hpp"

#include "angle_utils.h"
#include "quaternion_utils.h"

namespace a17 {
namespace maths {
namespace test {

// The maths implementation is faster than this more official eigen way:
double ManualYawFromQuaternion(const Eigen::Quaterniond &quaternion) {
  auto x_axis_in_world = quaternion * Eigen::Vector3d::UnitX();
  return std::atan2(x_axis_in_world[1], x_axis_in_world[0]);
}

bool SmallRotation(double left, double right) {
  auto angle_change = std::abs(left - right);
  while (angle_change > 2 * M_PI) {
    angle_change -= 2 * M_PI;
  }
  return angle_change < M_PI_4;
}

bool CompareQuats(const Eigen::Quaterniond &q1, const Eigen::Quaterniond &q2, double precision) {
  // q and -q are the same rotation.
  auto q2_neg = (q2 * Eigen::Quaterniond{-1, 0, 0, 0});
  return q1.isApprox(q2, precision) || q1.isApprox(q2_neg, precision);
}

TEST_CASE("YawFromQuaternion", "[maths]") {
  // Test over the range of yaw values with small perturbations.
  std::vector<Eigen::Quaterniond> rots;
  constexpr double angle_delta = 0.01;
  srand(0);
  auto rotation_axis = Eigen::Vector3d::UnitX();
  for (auto angle = 0.0; angle < 2 * M_PI; angle += angle_delta) {
    Eigen::Quaterniond rot(Eigen::AngleAxisd(angle, rotation_axis));
    Eigen::Vector3d perturbation_axis = Eigen::Vector3d::Random();
    perturbation_axis.normalize();
    double perturbation_angle = rand() * (angle_delta * 0.5) / RAND_MAX;
    Eigen::Quaterniond perturbation(Eigen::AngleAxisd(perturbation_angle, perturbation_axis));

    rots.push_back(rot * perturbation);
  }

  auto counter = 0;
  auto last_yaw = 0.0;
  for (const auto &rot : rots) {
    counter++;
    auto expected_yaw = ManualYawFromQuaternion(rot);
    auto yaw = a17::maths::YawFromQuaternion(rot);
    // Compare to slower Eigen implementation:
    REQUIRE(yaw == expected_yaw);

    // Make sure the angles don't oscillate.
    if (counter > 1) {
      REQUIRE(SmallRotation(yaw, last_yaw));
    }
    last_yaw = yaw;
  }

  // TODO(StefanJorgensen): Add some known test cases.
}

TEST_CASE("EulerToQuaternion", "[maths]") {
  Eigen::Quaterniond kRotate_90_Roll_Axis(
      Eigen::AngleAxisd(DegreesToRadians(90), Eigen::Vector3d::UnitX()));
  Eigen::Quaterniond kRotate_180_Roll_Axis(
      Eigen::AngleAxisd(DegreesToRadians(180), Eigen::Vector3d::UnitX()));
  Eigen::Quaterniond kRotate_270_Roll_Axis(
      Eigen::AngleAxisd(DegreesToRadians(270), Eigen::Vector3d::UnitX()));

  Eigen::Quaterniond kRotate_90_Pitch_Axis(
      Eigen::AngleAxisd(DegreesToRadians(90), Eigen::Vector3d::UnitY()));

  Eigen::Quaterniond kRotate_180_Pitch_Axis(
      Eigen::AngleAxisd(DegreesToRadians(180), Eigen::Vector3d::UnitY()));
  Eigen::Quaterniond kRotate_270_Pitch_Axis(
      Eigen::AngleAxisd(DegreesToRadians(270), Eigen::Vector3d::UnitY()));

  Eigen::Quaterniond kRotate_90_Yaw_Axis(
      Eigen::AngleAxisd(DegreesToRadians(90), Eigen::Vector3d::UnitZ()));
  Eigen::Quaterniond kRotate_180_Yaw_Axis(
      Eigen::AngleAxisd(DegreesToRadians(180), Eigen::Vector3d::UnitZ()));
  Eigen::Quaterniond kRotate_270_Yaw_Axis(
      Eigen::AngleAxisd(DegreesToRadians(270), Eigen::Vector3d::UnitZ()));

  Eigen::Quaterniond result = EulerToQuaternion(Eigen::Vector3d{0, 0, DegreesToRadians(270)});
  REQUIRE(kRotate_270_Yaw_Axis.isApprox(result));
}

TEST_CASE("Trivial Quaternion Rotation Tests", "[maths]") {
  // For reference: https://en.wikipedia.org/wiki/Aircraft_principal_axes
  // q = x, y, z, w (note Eigen initializer is of the form (w, x, y z))
  // x is the Longitudinal Axis (roll)
  // y is the Transverse Axis (pitch)
  // z is the Vertical Axis (yaw) axis (aka heading)
  Eigen::Quaterniond kRotate_90_Roll_Axis(
      Eigen::AngleAxisd(DegreesToRadians(90), Eigen::Vector3d::UnitX()));
  Eigen::Quaterniond kRotate_180_Roll_Axis(
      Eigen::AngleAxisd(DegreesToRadians(180), Eigen::Vector3d::UnitX()));
  Eigen::Quaterniond kRotate_270_Roll_Axis(
      Eigen::AngleAxisd(DegreesToRadians(270), Eigen::Vector3d::UnitX()));

  Eigen::Quaterniond kRotate_90_Pitch_Axis(
      Eigen::AngleAxisd(DegreesToRadians(90), Eigen::Vector3d::UnitY()));

  Eigen::Quaterniond kRotate_180_Pitch_Axis(
      Eigen::AngleAxisd(DegreesToRadians(180), Eigen::Vector3d::UnitY()));
  Eigen::Quaterniond kRotate_270_Pitch_Axis(
      Eigen::AngleAxisd(DegreesToRadians(270), Eigen::Vector3d::UnitY()));

  Eigen::Quaterniond kRotate_90_Yaw_Axis(
      Eigen::AngleAxisd(DegreesToRadians(90), Eigen::Vector3d::UnitZ()));
  Eigen::Quaterniond kRotate_180_Yaw_Axis(
      Eigen::AngleAxisd(DegreesToRadians(180), Eigen::Vector3d::UnitZ()));
  Eigen::Quaterniond kRotate_270_Yaw_Axis(
      Eigen::AngleAxisd(DegreesToRadians(270), Eigen::Vector3d::UnitZ()));

  SECTION("Trivial Rotations") {
    REQUIRE(YawFromQuaternion(kRotate_90_Roll_Axis) == Approx(WrapToPi(0.0)));
    REQUIRE(YawFromQuaternion(kRotate_90_Pitch_Axis) == Approx(WrapToPi(0.0)));
    REQUIRE(YawFromQuaternion(kRotate_90_Yaw_Axis) == Approx(WrapToPi(M_PI / 2.0)));
    REQUIRE(YawFromQuaternion(kRotate_180_Yaw_Axis) == Approx(WrapToPi(M_PI)));
    REQUIRE(YawFromQuaternion(kRotate_270_Yaw_Axis) == Approx(WrapToPi(3.0 * M_PI / 2.0)));

    REQUIRE(RollFromQuaternion(kRotate_90_Pitch_Axis) == Approx(WrapToPi(0.0)));
    REQUIRE(RollFromQuaternion(kRotate_90_Yaw_Axis) == Approx(WrapToPi(0.0)));
    REQUIRE(RollFromQuaternion(kRotate_90_Roll_Axis) == Approx(WrapToPi(M_PI / 2.0)));
    REQUIRE(RollFromQuaternion(kRotate_180_Roll_Axis) == Approx(WrapToPi(M_PI)));
    REQUIRE(RollFromQuaternion(kRotate_270_Roll_Axis) == Approx(WrapToPi(3.0 * M_PI / 2.0)));

    REQUIRE(PitchFromQuaternion(kRotate_90_Roll_Axis) == Approx(WrapToPi(0.0)));
    REQUIRE(PitchFromQuaternion(kRotate_90_Yaw_Axis) == Approx(WrapToPi(0.0)));
  }

  SECTION("Recomposing the quaternion") {
    // It is more important to us that we recompose the quaternion nicely.
    auto limit = 180.0;
    auto increment = 5.0;
    auto pitch_increment = increment;
    for (auto roll = -limit; roll < limit; roll += increment) {
      for (auto pitch = -limit; pitch < limit; pitch += pitch_increment) {
        for (auto yaw = -limit; yaw < limit; yaw += increment) {
          Eigen::Vector3d given_rpy{DegreesToRadians(roll), DegreesToRadians(pitch),
                                    DegreesToRadians(yaw)};
          auto given_quat = EulerToQuaternion(given_rpy);

          Eigen::Vector3d recomposed_rpy = QuaternionToEuler(given_quat);
          auto recomposed_quat = EulerToQuaternion(recomposed_rpy);

          REQUIRE(CompareQuats(given_quat, recomposed_quat, 1e-6));
        }
        // It is important to test the pitch around +- 90 degrees as it affects how roll and yaw are
        // calculated. This increment adjuster helps look closely at those angles.
        if (std::abs(pitch) <= (90 + increment) && std::abs(pitch) >= (90 - increment)) {
          pitch_increment = 0.2;
        } else {
          pitch_increment = increment;
        }
      }
    }
  }
}

}  // namespace test
}  // namespace maths
}  // namespace a17
