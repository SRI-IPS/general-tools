#pragma once

#include "Eigen/Geometry"

namespace a17 {
namespace maths {

class Rotations {
 public:
  Rotations(){};
  const Eigen::Vector3d pointEnuToNed(const Eigen::Vector3d &enu);
  const Eigen::Quaterniond rotEnuToNed(const Eigen::Quaterniond &enu);

 private:
  // Quaternion representing 180 degree rotation about the x-axis.
  const Eigen::Quaterniond q_pi_x_{Eigen::AngleAxis<double>(M_PI, Eigen::Vector3d::UnitX())};
  // Quaternion representing 90 degree rotation about the z-axis.
  const Eigen::Quaterniond q_pi_2_z_{Eigen::AngleAxis<double>(M_PI_2, Eigen::Vector3d::UnitZ())};
  // Quaternion from NED to ENU is a 90 degree rotation about the z-axis
  // and then a 180 degree rotation about the x-axis.
  const Eigen::Quaterniond q_ned_to_enu_ = q_pi_2_z_ * q_pi_x_;
  // Quaternion from ENU to NED is the quaternion inverse of the NED to ENU rotation.
  const Eigen::Quaterniond q_enu_to_ned_ = q_ned_to_enu_.inverse();

 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}  // namespace maths
}  // namespace a17
