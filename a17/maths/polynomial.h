#pragma once

#include <exception>
#include <string>
#include <vector>

#include "Eigen/Dense"
#include "mav_trajectory_generation/rpoly.h"

#include "float_utils.h"
#include "polynomial_utils.h"

namespace a17 {
namespace maths {

class Polynomial {
 public:
  /// Default constructor for convenience when using polynomials in other classes.
  Polynomial() : coefficients_(Eigen::VectorXd(0)) {}

  /// Construct a polynomial with the given coefficients in increasing order, i.e.
  /// P(t) = c[0] + c[1] * t + c[2] * t^2 + ...
  /// @param coefficients The coefficients of the polynomial.
  explicit Polynomial(Eigen::VectorXd coefficients) { setCoefficients(coefficients); }

  /// Sets polynomial coefficients.
  /// @param coefficients The coefficients of the polynomial.
  void setCoefficients(Eigen::VectorXd coefficients) {
    // If highest order coefficients are zero, we remove them.
    auto reduced_order = 0;
    for (auto k = coefficients.size() - 1; k > 0; --k) {
      if (!a17::maths::FloatEq(coefficients[k], 0.0)) {
        reduced_order = k;
        break;
      }
    }
    coefficients_ = coefficients.head(reduced_order + 1);
  }

  /// Evaluate the polynomial.
  /// @param t The parameter value to evaluate the polynomial at.
  /// @returns the polynomial evaluated at t.
  double operator()(double t) const {
    auto coeff_idx = coefficients_.size() - 1;
    auto value = coefficients_[coeff_idx];
    for (--coeff_idx; coeff_idx >= 0; --coeff_idx) {
      value *= t;
      value += coefficients_[coeff_idx];
    }
    return value;
  }

  /// Evaluate the derivative of the polynomial.
  /// @param t The parameter value to evaluate at.
  /// @param order The order of derivative to take. Should be positive.
  /// @throws std::runtime_error if order < 0.
  /// @returns the value of the derivative at t.
  double derivative(double t, int order) const {
    auto derivative_polynomial = derivative(order);
    return derivative_polynomial(t);
  }

  /// Return the derivative of the polynomial.
  /// @param order The order of derivative to take. Should be positive.
  /// @throws std::runtime_error if order < 0.
  /// @returns The polynomial representing the derivative.
  Polynomial derivative(int order) const {
    if (order < 0) throw std::runtime_error("Derivative order must be positive");
    if (order == 0) return Polynomial(coefficients_);

    auto new_size = coefficients_.size() - order;
    if (new_size <= 0) {
      Eigen::VectorXd new_coefficients = Eigen::VectorXd::Zero(1);
      return Polynomial(new_coefficients);
    }
    auto coeff_idx = coefficients_.size() - 1;
    auto derivative_coefficient = 1.0;
    for (auto k = coeff_idx; k > coeff_idx - order; --k) {
      derivative_coefficient *= k;
    }

    auto new_coefficients = Eigen::VectorXd(new_size);
    new_coefficients[new_size - 1] = coefficients_[coeff_idx] * derivative_coefficient;
    for (--coeff_idx; coeff_idx >= order; --coeff_idx) {
      derivative_coefficient = 1.0;
      for (auto k = coeff_idx; k > coeff_idx - order; --k) {
        derivative_coefficient *= k;
      }
      new_coefficients[coeff_idx - order] = coefficients_[coeff_idx] * derivative_coefficient;
    }
    return Polynomial(new_coefficients);
  }

  /// Find the level set of the polynomial.
  /// @param level The level to search for.
  /// @returns a vector of all values of t such that P(t) = level.
  std::vector<double> levelSet(double level) const {
    std::vector<double> level_set;
    auto order = coefficients_.size() - 1;
    // Polynomial is 0th order and has no roots.
    if (order < 1) return level_set;

    // We find the level set by finding the roots of G(t) = P(t) - level.
    Eigen::VectorXd level_poly = coefficients_;
    level_poly[0] -= level;

    if (order == 1) {
      level_set.emplace_back(-level_poly[0] / level_poly[1]);
      return level_set;
    } else if (order == 2) {
      return a17::maths::QuadraticRealRoots(level_poly[2], level_poly[1], level_poly[0]);
    }

    Eigen::VectorXcd roots;
    if (mav_trajectory_generation::findRootsJenkinsTraub(level_poly, &roots)) {
      for (auto k = 0; k < roots.size(); k++) {
        if (FloatEq(roots[k].imag(), 0.0)) {
          level_set.emplace_back(roots[k].real());
        }
      }
    }
    return level_set;
  }

  Eigen::VectorXd coefficients() const { return coefficients_; }

  /// Returns a polynomial which is reversed over an interval [t0, t1]: G(t) = P(t0 + t1 - t).
  /// @param t0 The start of the interval.
  /// @param t1 The end of the interval.
  /// @returns the reverse of the polynomial over the interval [t0, t1].
  Polynomial reverse(double t0, double t1) const {
    auto size = coefficients_.size();
    // Coefficients for G(t) = P(t0 + t1 - t) are computed using the following matrix:
    // 1  c   c^2  c^3   c^4 ...
    // 0 -1 -2c  -3c^2 -4c^3 ...
    // 0  0   1   3c    6c^2 ...
    // 0  0   0  -1    -4c   ...
    // 0  0   0   0     1    ...
    // ...
    // where c = t0 + t1, coefficients are from Pascal's triangle, and signs alternate every row.
    Eigen::MatrixXd conversion_matrix = Eigen::MatrixXd::Zero(size, size);
    auto c = t0 + t1;
    conversion_matrix(0, 0) = 1.0;
    for (auto col = 1; col < size; col++) {
      conversion_matrix(0, col) = c * conversion_matrix(0, col - 1);
    }
    for (auto row = 1; row < size; row++) {
      for (auto col = row; col < size; col++) {
        conversion_matrix(row, col) =
            c * conversion_matrix(row, col - 1) - conversion_matrix(row - 1, col - 1);
      }
    }
    auto reverse_coefficients = conversion_matrix * coefficients_;
    return Polynomial(reverse_coefficients);
  }

  std::string toString() const {
    auto ostream = std::ostringstream{};
    for (auto k = 0; k < coefficients_.size(); k++) {
      ostream << coefficients_[k] << "t^" << k;
      if (k < coefficients_.size() - 1) {
        ostream << " + ";
      }
    }
    return ostream.str();
  }

 private:
  Eigen::VectorXd coefficients_;
};

}  // namespace maths
}  // namespace a17
