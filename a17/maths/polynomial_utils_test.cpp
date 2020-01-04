#include "Eigen/Core"
#include "catch.hpp"

#include "polynomial_utils.h"

namespace a17 {
namespace maths {
namespace test {

TEST_CASE("QuadraticRealRoots", "[maths]") {
  constexpr auto kPrecision = 1e-8;

  // Generate equations of the form (q*x + r)*(s*x + t), multiply it out
  // to get a*x^2 + b*x + c, feed it to QuadraticRealRoots. We know the
  // true roots are x = -r/q and x = -t/s.

  srand(0);
  for (int i = 0; i < 100; ++i) {
    Eigen::Vector4d params = Eigen::Vector4d::Random();
    double q = params[0];
    double r = params[1];
    double s = params[2];
    double t = params[3];

    double a = q * s;
    double b = q * t + r * s;
    double c = r * t;

    double root1 = -r / q;
    double root2 = -t / s;

    std::vector<double> known_roots{root1};
    if (root2 != root1) {
      known_roots.push_back(root2);
    }

    auto computed_roots = QuadraticRealRoots(a, b, c);

    REQUIRE(known_roots.size() == computed_roots.size());

    std::sort(known_roots.begin(), known_roots.end());
    std::sort(computed_roots.begin(), computed_roots.end());

    for (size_t i = 0; i < known_roots.size(); ++i) {
      REQUIRE(std::abs(known_roots[i] - computed_roots[i]) < kPrecision);
    }
  }
}

}  // namespace test
}  // namespace maths
}  // namespace a17
