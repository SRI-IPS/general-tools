#pragma once

#include <cmath>
#include <limits>

namespace a17 {
namespace maths {

inline float FloatEpsilon(float left, float right) noexcept {
  // Multiply by 2 to give some extra wiggle room.
  return std::max(std::abs(std::nexttoward(left, right) - left) * 2.0f,
                  std::numeric_limits<float>::min());
}

/// Calculates the absolute value difference between left and right. Returns a double to prevent
/// float overflow.
inline double FloatAbsDiff(float left, float right) noexcept {
  return std::abs(static_cast<double>(left) - static_cast<double>(right));
}

static const auto kFloatMin = std::nexttoward(std::numeric_limits<float>::lowest(), 0.0);
static const auto kFloatMax = std::nexttoward(std::numeric_limits<float>::max(), 0.0);

/// Returns true if left is equal to right, within floating point precision limits.
inline bool FloatEq(float left, float right, double epsilon = -1.0) noexcept {
  if (left <= kFloatMin) {
    return right <= kFloatMin;
  }
  if (left >= kFloatMax) {
    return right >= kFloatMax;
  }
  if (right <= kFloatMin || right >= kFloatMax) {
    return false;
  }
  if (epsilon < 0.0) {
    epsilon = static_cast<double>(FloatEpsilon(left, right));
  }
  return FloatAbsDiff(left, right) < epsilon;
}

/// Returns whether |left| is less than |right|, accounting for floating-point precision edge-cases.
inline bool FloatLt(float left, float right) noexcept {
  return left < right && !FloatEq(left, right);
}

/// Returns whether |left| is less than or equal to |right|, accounting for floating-point =
/// precision edge-cases.
inline bool FloatLtEq(float left, float right) noexcept {
  return left < right || FloatEq(left, right);
}

/// Returns whether |left| is greater than |right|, accounting for floating-point precision
/// edge-cases.
inline bool FloatGt(float left, float right) noexcept {
  return left > right && !FloatEq(left, right);
}

/// Returns whether |left| is greater than or equal to |right|, accounting for floating-point
/// precision edge-cases.
inline bool FloatGtEq(float left, float right) noexcept {
  return left > right || FloatEq(left, right);
}

/// Returns an int from a given double, rounded down, accounting for floating-point precision
/// edge-cases.
inline int FloatFloor(double value) noexcept {
  auto rounded_value = std::round(value);
  if (FloatAbsDiff(value, rounded_value) < FloatEpsilon(value, rounded_value)) {
    return static_cast<int>(rounded_value);
  }
  return std::floor(value);
}

}  // namespace maths
}  // namespace a17
