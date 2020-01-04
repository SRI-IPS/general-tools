#include "catch.hpp"

#include <map>

#include "Eigen/Core"

#include "float_utils.h"
#include "polynomial.h"

namespace a17 {
namespace maths {
namespace test {

TEST_CASE("Polynomial", "[maths]") {
  Eigen::VectorXd coeff_constant(1);
  coeff_constant << 1.0;
  auto f_constant = Polynomial(coeff_constant);
  for (auto t = 0.0; t < 1; t += 0.05) {
    REQUIRE(FloatEq(f_constant(t), coeff_constant[0]));
    REQUIRE(FloatEq(f_constant.derivative(t, 1), 0.0));
  }
  for (auto level = 0.0; level < 5.0; level += 0.2) {
    REQUIRE(f_constant.levelSet(level).empty());
  }
  auto derivative_coefficients = f_constant.derivative(1).coefficients();
  REQUIRE(derivative_coefficients.size() == 1);
  REQUIRE(derivative_coefficients[0] == 0.0);

  REQUIRE_THROWS(f_constant.derivative(-10));

  Eigen::VectorXd coeff_linear(2);
  coeff_linear << 1.0, 2.0;
  auto f_linear = Polynomial(coeff_linear);
  for (auto t = 0.0; t < 1; t += 0.05) {
    REQUIRE(FloatEq(f_linear(t), coeff_linear[0] + coeff_linear[1] * t));
    REQUIRE(FloatEq(f_linear.derivative(t, 1), coeff_linear[1]));
    REQUIRE(FloatEq(f_linear.derivative(t, 2), 0.0));
  }
  for (auto level = 0.0; level < 5.0; level += 0.2) {
    auto level_set = f_linear.levelSet(level);
    REQUIRE(level_set.size() == 1);
    REQUIRE(FloatEq(level_set[0], (level - coeff_linear[0]) / coeff_linear[1]));
  }
  derivative_coefficients = f_linear.derivative(1).coefficients();
  REQUIRE(derivative_coefficients.size() == 1);
  REQUIRE(FloatEq(derivative_coefficients[0], coeff_linear[1]));

  Eigen::VectorXd coeff_quadratic(3);
  coeff_quadratic << 1.0, 2.0, 3.0;
  auto f_quadratic = Polynomial(coeff_quadratic);
  for (auto t = 0.0; t < 1.0; t += 0.05) {
    REQUIRE(FloatEq(f_quadratic(t),
                    coeff_quadratic[0] + coeff_quadratic[1] * t + coeff_quadratic[2] * t * t));
    REQUIRE(FloatEq(f_quadratic.derivative(t, 1), coeff_quadratic[1] + 2 * coeff_quadratic[2] * t));
    REQUIRE(FloatEq(f_quadratic.derivative(t, 2), 2 * coeff_quadratic[2]));
    REQUIRE(FloatEq(f_quadratic.derivative(t, 3), 0.0));
  }
  for (auto level = 0.0; level < 5.0; level += 0.2) {
    auto level_set = f_quadratic.levelSet(level);
    REQUIRE(level_set.size() <= 2);
    // Note: This is already tested in polynomial_utils_test.cpp
  }
  derivative_coefficients = f_quadratic.derivative(1).coefficients();
  REQUIRE(derivative_coefficients.size() == 2);
  REQUIRE(FloatEq(derivative_coefficients[0], coeff_quadratic[1]));
  REQUIRE(FloatEq(derivative_coefficients[1], 2*coeff_quadratic[2]));

  Eigen::VectorXd coeff_cubic(4);
  // f(t) = (t + 1)(t - 2)(t + 3)
  coeff_cubic << -6.0, -5.0, 2.0, 1.0;
  auto f_cubic = Polynomial(coeff_cubic);
  for (auto t = 0.0; t < 1.0; t += 0.05) {
    REQUIRE(FloatEq(f_cubic(t), coeff_cubic[0] + coeff_cubic[1] * t + coeff_cubic[2] * t * t +
                                    coeff_cubic[3] * t * t * t));
    REQUIRE(FloatEq(f_cubic.derivative(t, 1),
                    coeff_cubic[1] + 2 * coeff_cubic[2] * t + 3 * coeff_cubic[3] * t * t));
    REQUIRE(FloatEq(f_cubic.derivative(t, 2), 2 * coeff_cubic[2] + 6 * coeff_cubic[3] * t));
    REQUIRE(FloatEq(f_cubic.derivative(t, 3), 6 * coeff_cubic[3]));
    REQUIRE(FloatEq(f_cubic.derivative(t, 4), 0.0));
  }
  derivative_coefficients = f_cubic.derivative(1).coefficients();
  REQUIRE(derivative_coefficients.size() == 3);
  REQUIRE(FloatEq(derivative_coefficients[0], coeff_cubic[1]));
  REQUIRE(FloatEq(derivative_coefficients[1], 2*coeff_cubic[2]));
  REQUIRE(FloatEq(derivative_coefficients[2], 3*coeff_cubic[3]));

  std::map<double, std::vector<double>> expected_level_sets = {{0, {-1, 2, -3}},
                                                               {0.2, {-1.03353, 2.01324, -2.97971}},
                                                               {0.4, {-1.06748, 2.0263, -2.95882}},
                                                               {0.6, {-1.10191, 2.03918, -2.93727}},
                                                               {0.8, {-1.13688, 2.05189, -2.915}},
                                                               {1, {-1.17248, 2.06443, -2.89195}},
                                                               {1.2, {-1.20878, 2.07682, -2.86804}},
                                                               {1.4, {-1.24589, 2.08906, -2.84317}},
                                                               {1.6, {-1.28392, 2.10114, -2.81723}},
                                                               {1.8, {-1.32301, 2.11308, -2.79008}},
                                                               {2, {2.12489, -1.36333, -2.76156}},
                                                               {2.2, {2.13655, -1.4051, -2.73146}},
                                                               {2.4, {2.14809, -1.44858, -2.69951}},
                                                               {2.6, {2.1595, -1.49414, -2.66536}},
                                                               {2.8, {2.17078, -1.54224, -2.62854}},
                                                               {3, {2.18194, -1.59358, -2.58836}},
                                                               {3.2, {2.19299, -1.64916, -2.54383}},
                                                               {3.4, {2.20392, -1.71066, -2.49327}},
                                                               {3.6, {2.21474, -1.78113, -2.43361}},
                                                               {3.8, {2.22546, -1.8677, -2.35776}}};

  for (const auto &kv : expected_level_sets) {
    auto level = kv.first;
    auto expected_set = kv.second;
    auto level_set = f_cubic.levelSet(level);
    REQUIRE(level_set.size() == expected_set.size());
    for (auto k = 0u; k < level_set.size(); k++) {
      REQUIRE(std::abs(expected_set[k] - level_set[k]) < 0.00001);
    }
  }
}

TEST_CASE("Reversed polynomials", "[maths]") {
  Eigen::VectorXd coefficients(5);
  coefficients << 1., 2., 3., 4., 5.;
  auto t_start = -3.0;
  auto t_stop = 4.0;
  auto forward_poly = Polynomial(coefficients);
  auto reverse_poly = forward_poly.reverse(t_start, t_stop);

  for (auto t = 0.0; t < t_stop - t_start; t += 0.2) {
    REQUIRE(FloatEq(reverse_poly(t_start + t), forward_poly(t_stop - t)));
  }
}

}  // namespace test
}  // namespace maths
}  // namespace a17
