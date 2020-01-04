#pragma once

namespace a17 {
namespace maths {

/// Converts the angle to be within the range of -PI to PI.
/// @param angle The angle to normalize (in radians).
double WrapToPi(double angle) noexcept;

/// Finds the minimum angle difference between two angles in radians.
/// @param angle1 First angle (radians)
/// @param angle2 Second angle (radians)
double AngleDiff(double angle1, double angle2) noexcept;

/// Converts the input angle in degrees and returns the equivalent angle in radians.
/// @param degrees Angle in degrees
double DegreesToRadians(double degrees) noexcept;

/// Converts the input angle in radians and returns the equivalent angle in degrees.
/// @param radians Angle in radians
double RadiansToDegrees(double radians) noexcept;

}  // namespace maths
}  // namespace a17
