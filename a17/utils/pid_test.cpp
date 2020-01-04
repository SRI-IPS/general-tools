#include <chrono>
#include <limits>
#include <thread>

#include "catch.hpp"
#include "pid.h"

namespace a17 {
namespace utils {

constexpr double kDoubleTolerance = 1.0e-4;
constexpr double kDefaultKP = 1.0;
constexpr double kDefaultKI = 0.1;
constexpr double kDefaultKD = 0.3;

class PIDUnitTests{
public:
  void test_init() {
    PID test_pid(kDefaultKP, kDefaultKI, kDefaultKD);
    REQUIRE(test_pid.kP_ == kDefaultKP);
    REQUIRE(test_pid.kI_ == kDefaultKI);
    REQUIRE(test_pid.kD_ == kDefaultKD);
    REQUIRE(test_pid.max_output_ == std::numeric_limits<double>::max());
    REQUIRE(test_pid.min_output_ == std::numeric_limits<double>::lowest());
    REQUIRE(test_pid.first_sample_computed_ == false);
    REQUIRE(test_pid.integral_error_ == 0);
    REQUIRE(test_pid.last_error_ == 0);
  }

  void test_restart() {
    PID test_pid(kDefaultKP, kDefaultKI, kDefaultKD);
    // Compute several iterations to change the value of internal variables
    test_pid.compute(2, 0);
    test_pid.compute(2, 0);
    test_pid.compute(2, 0);
    test_pid.compute(2, 0);

    REQUIRE(test_pid.first_sample_computed_ == true);
    REQUIRE(test_pid.integral_error_ != 0);
    REQUIRE(test_pid.last_error_ != 0);

    test_pid.restartController();

    REQUIRE(test_pid.first_sample_computed_ == false);
    REQUIRE(test_pid.integral_error_ == 0);
    REQUIRE(test_pid.last_error_ == 0);
  }

  void test_clip() {
    PID test_pid(kDefaultKP, kDefaultKI, kDefaultKD);
    double value;
    double limit;
    SECTION ("Clip from top"){
      value = 10.0;
      limit = 5.0;
      test_pid.clip(value, limit);
      REQUIRE(value == 5.0);
    }
    SECTION ("Clip from bottom"){
      value = -10.0;
      limit = 5.0;
      test_pid.clip(value, limit);
      REQUIRE(value == -5.0);
    }
    SECTION ("Clip to 0"){
      value = 1231232;
      limit = 0.0;
      test_pid.clip(value, limit);
      REQUIRE(value == 0.0);
    }
  }

  void test_antiwindup() {
    // We force integral action here by making kI 20 times bigger
    PID test_pid(kDefaultKP, 20 * kDefaultKI, kDefaultKD);

    SECTION ("Windup is happenning") {
      test_pid.setAntiWindup(1, 99999);
      test_pid.compute(10000, 0);
      std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(100));
      test_pid.compute(10000, 0);
      REQUIRE(test_pid.integral_error_ >= 10.0);
    }

    SECTION ("Positive windup is limited") {
      test_pid.setAntiWindup(1, 10);
      test_pid.compute(10000, 0);
      std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(100));
      test_pid.compute(10000, 0);
      REQUIRE(test_pid.integral_error_ == 10.0);
    }

    SECTION ("Negative windup is limited") {
      test_pid.setAntiWindup(1, 10);
      test_pid.compute(-10000, 0);
      std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds(100));
      test_pid.compute(-10000, 0);
      REQUIRE(test_pid.integral_error_ == -10.0);
    }
  }

  void test_max_min_output() {
    PID test_pid(kDefaultKP, kDefaultKI, kDefaultKD);

    SECTION ("Test max") {
      double output = test_pid.compute(10000, 0);
      REQUIRE(output >= 5.0);

      test_pid.setMaxOutput(5.0);
      output = test_pid.compute(10000, 0);
      REQUIRE(output == 5.0);
    }

    SECTION ("Test min") {
      double output = test_pid.compute(-10000, 0);
      REQUIRE(output <= -5.0);

      test_pid.setMinOutput(-5.0);
      output = test_pid.compute(-10000, 0);
      REQUIRE(output == -5.0);
    }

    SECTION ("Test minmax") {
      PID test_pid_2(kDefaultKP, kDefaultKI, kDefaultKD);
      test_pid_2.setMaxOutput(5000.0);
      test_pid_2.setMinOutput(-5000.0);
      double output_1 = test_pid.compute(10, 0);
      double output_2 = test_pid.compute(10, 0);
      REQUIRE( std::fabs(output_1 - output_2) < kDoubleTolerance);
    }
  }

  void test_angular() {
    PID test_pid(kDefaultKP, kDefaultKI, kDefaultKD);

    SECTION ("Test linear") {
      double error = 100.0;
      test_pid.handleAngularDiscontinuity(error);
      REQUIRE(error == 100.0);
    }

    SECTION ("Test angular") {
      test_pid.setType(PID::Type::ANGULAR);
      for (int i=-100; i<100; ++i) {
        double error = 0.1 * i;
        test_pid.handleAngularDiscontinuity(error);
        REQUIRE(std::fabs(error) <= M_PI);
      }
    }
  }

};

TEST_CASE("Initialization", "[PID]") {
  PIDUnitTests test;
  test.test_init();
}

TEST_CASE("Restart", "[PID]") {
  PIDUnitTests test;
  test.test_restart();
}

TEST_CASE("Clip", "[PID]") {
  PIDUnitTests test;
  test.test_clip();
}

TEST_CASE("Antiwindup", "[PID]") {
  PIDUnitTests test;
  test.test_antiwindup();
}

TEST_CASE("Max Min", "[PID]") {
  PIDUnitTests test;
  test.test_max_min_output();
}

TEST_CASE("Angular", "[PID]") {
  PIDUnitTests test;
  test.test_angular();
}

}  //namespace utils
}  //namespace a17
