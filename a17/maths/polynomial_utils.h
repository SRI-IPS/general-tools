#pragma once

#include <vector>

namespace a17 {
namespace maths {

/** @brief Compute the real roots of the quadratic polynomial a^2*x + b*x + c = 0.
 */
template <typename Scalar>
std::vector<Scalar> QuadraticRealRoots(Scalar a, Scalar b, Scalar c) {
  std::vector<Scalar> roots;
  const Scalar discriminant = b * b - 4 * a * c;
  if (discriminant == 0) {
    // One real root.
    roots.push_back(-b / (2 * a));
  } else if (discriminant > 0) {
    // Two real roots.
    const double droot = sqrt(discriminant);
    roots.push_back((-b + droot) / (2 * a));
    roots.push_back((-b - droot) / (2 * a));
  }

  return roots;
}

}  // namespace maths
}  // namespace a17
