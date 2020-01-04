#include "angle_unroller.h"

#include <cmath>

#include "angle_utils.h"

namespace a17 {
namespace maths {

AngleUnroller::AngleUnroller(double initial_angle) : unrolled_angle_(initial_angle) {}

double AngleUnroller::unrollAngle(double angle) {
  auto previous_angle = a17::maths::WrapToPi(unrolled_angle_);
  angle = a17::maths::WrapToPi(angle);
  auto diff = angle - previous_angle;
  if (std::abs(diff) <= M_PI) {
    unrolled_angle_ += diff;
  } else if (diff > 0) {
    unrolled_angle_ -= (2 * M_PI) - diff;
  } else {  // if (diff < 0) {
    unrolled_angle_ += (2 * M_PI) + diff;
  }
  return unrolled_angle_;
}

}  // namespace maths
}  // namespace a17
