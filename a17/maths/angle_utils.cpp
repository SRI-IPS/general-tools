#include "angle_utils.h"

#include <cmath>

#include "constants.h"

namespace a17 {
namespace maths {

double WrapToPi(double angle) noexcept {
  while (angle > kPi) {
    angle -= 2 * kPi;
  }
  while (angle < -kPi) {
    angle += 2 * kPi;
  }
  return angle;
}

double AngleDiff(double angle1, double angle2) noexcept {
  return kPi - std::fabs(std::fmod(std::fabs(angle1 - angle2), kPi * 2.0) - kPi);
}

double DegreesToRadians(double degrees) noexcept { return degrees * kPi / 180.0; }

double RadiansToDegrees(double radians) noexcept { return radians * 180.0 / kPi; }

}  // namespace maths
}  // namespace a17
