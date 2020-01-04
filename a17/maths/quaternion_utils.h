#pragma once

#include <math.h>

#include "Eigen/Geometry"

#include "angle_utils.h"

namespace a17 {
namespace maths {

// Based on https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

/// Compute the euler angle yaw component of a given quaternion.
/// Note: this is only valid if the pitch is not +/- 90 degrees! Otherwise yaw can take on an
/// arbitrary value (roll would be determined by yaw value).
template <typename Scalar, int Options>
Scalar YawFromQuaternion(const Eigen::Quaternion<Scalar, Options> &quaternion) {
  auto siny = 2.0 * (quaternion.w() * quaternion.z() + quaternion.x() * quaternion.y());
  auto cosy = 1.0 - 2.0 * (quaternion.y() * quaternion.y() + quaternion.z() * quaternion.z());
  return std::atan2(siny, cosy);
}

/// Compute the euler angle roll component of a given quaternion.
/// Note: this is only valid if the pitch is not +/- 90 degrees! Otherwise roll can take on an
/// arbitrary value (yaw would be determined by roll value).
template <typename Scalar, int Options>
Scalar RollFromQuaternion(const Eigen::Quaternion<Scalar, Options> &quaternion) {
  auto sinr = 2.0 * (quaternion.w() * quaternion.x() + quaternion.y() * quaternion.z());
  auto cosr = 1.0 - 2.0 * (quaternion.x() * quaternion.x() + quaternion.y() * quaternion.y());
  return std::atan2(sinr, cosr);
}

/// Compute the euler angle pitch component of a given quaternion.
template <typename Scalar, int Options>
Scalar PitchFromQuaternion(const Eigen::Quaternion<Scalar, Options> &quaternion) {
  constexpr auto kPrecision = 1e-10;
  auto sinp = 2.0 * (quaternion.w() * quaternion.y() - quaternion.x() * quaternion.z());
  if (sinp >= 1.0 - kPrecision) {
    return M_PI_2;
  } else if (sinp <= -1.0 + kPrecision) {
    return -M_PI_2;
  } else {
    return std::asin(sinp);
  }
}

/// Convert a quaternion to euler angles
/// @return Eigen::Vector3 in the form [roll, pitch, yaw].
template <typename Scalar, int Options>
Eigen::Matrix<Scalar, 3, 1, Options> QuaternionToEuler(
    const Eigen::Quaternion<Scalar, Options> &q) {
  Scalar roll = 0.0, pitch = 0.0, yaw = 0.0;
  pitch = PitchFromQuaternion(q);
  if (pitch == M_PI_2 || pitch == -M_PI_2) {
    // Roll and yaw have no unique values because we're gimbal locked.
    // Pick roll to be zero, and compute the corresponding yaw.
    auto sign = std::copysign(1.0, pitch);
    roll = 0;
    yaw = sign * -2 * std::atan2(q.x(), q.w());
  } else {
    roll = RollFromQuaternion(q);
    yaw = YawFromQuaternion(q);
  }
  return Eigen::Matrix<Scalar, 3, 1, Options>{WrapToPi(roll), WrapToPi(pitch), WrapToPi(yaw)};
}

/// Convert a Eigen::Vector of the form [roll, pitch, yaw] into a quaternion.
/// The expected input is [roll, pitch, yaw], but the rotations are composed as Tait-Bryan angles,
/// i.e. yaw-pitch-roll.
/// @return Eigen::Quaternion representing the rotation.
template <typename Scalar, int Options>
Eigen::Quaternion<Scalar, Options> EulerToQuaternion(
    const Eigen::Matrix<Scalar, 3, 1, Options> &euler) {
  Eigen::Quaternion<Scalar, Options> q;
  q = Eigen::AngleAxis<Scalar>(euler.z(), Eigen::Matrix<Scalar, 3, 1, Options>::UnitZ()) *
      Eigen::AngleAxis<Scalar>(euler.y(), Eigen::Matrix<Scalar, 3, 1, Options>::UnitY()) *
      Eigen::AngleAxis<Scalar>(euler.x(), Eigen::Matrix<Scalar, 3, 1, Options>::UnitX());
  return q;
}

}  // namespace maths
}  // namespace a17
