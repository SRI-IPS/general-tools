#include "rotations.h"

namespace a17 {
namespace maths {

const Eigen::Vector3d Rotations::pointEnuToNed(const Eigen::Vector3d &enu) {
  return q_enu_to_ned_._transformVector(enu);
};

const Eigen::Quaterniond Rotations::rotEnuToNed(const Eigen::Quaterniond &enu) {
  return q_ned_to_enu_ * enu * q_enu_to_ned_;
};

}  // namespace maths
}  // namespace a17
