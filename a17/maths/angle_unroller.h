#pragma once

namespace a17 {
namespace maths {

/// This class "unrolls" a series of angles. It is useful for creating a sequence of angles without
/// any discontinuities. Typically, this comes up when it is necessary to remove the discontinuities
/// from -180 to 180 degrees.
/// All angles are in radians.
class AngleUnroller {
 public:
  /// @param initial_angle The angle to use as the starting point for unrolling (in radians).
  AngleUnroller(double initial_angle);

  /// Adds an angle to the unroller and unrolls it. The unrolled angle is returned, and you can also
  /// call the unrolledAngle() method to get the unrolled angle.
  /// @param angle The angle to add (in radians).
  /// @returns The unrolled angle (in radians).
  double unrollAngle(double angle);

  /// Returns the current unrolled angle (in radians). Note that this angle is not expected to be
  /// between -pi and pi.
  double unrolledAngle() const noexcept { return unrolled_angle_; }

 private:
  double unrolled_angle_ = 0.0;
};

}  // namespace dispatch_nodes
}  // namespace a17
