#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <math.h>
#include <cmath>

#include "pid.h"

namespace a17{
namespace utils{

PID::PID(double kp, double ki, double kd) :
  kP_(kp),
  kI_(ki),
  kD_(kd),
  min_output_(std::numeric_limits<double>::lowest()),
  max_output_(std::numeric_limits<double>::max()),
  antiwindup_damping_(1.0),
  integral_error_limit_(std::numeric_limits<double>::max()),
  type_(kDefaultType),
  first_sample_computed_(false),
  integral_error_(0),
  last_error_(0), 
  last_timepoint_(std::chrono::high_resolution_clock::now()) {
}
  
double PID::compute(double reference, double signal) {
  // Derivative / Integral terms make no sense on the first iteration.
  if (!first_sample_computed_) {
    first_sample_computed_ = true;
    return computeP(reference, signal);
  }

  auto current_timepoint = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> duration = current_timepoint - last_timepoint_;
  double elapsed_time_secs = duration.count();

  // Proportional
  double error = reference - signal;
  if (type_ == Type::ANGULAR) {
    handleAngularDiscontinuity(error);
  }

  // Integral
  integral_error_ *= antiwindup_damping_;
  integral_error_ += error * elapsed_time_secs;
  clip(integral_error_, integral_error_limit_);

  // Derivative
  double derivative_error = 0;
  if (elapsed_time_secs > 0) {
    derivative_error = (error - last_error_) / elapsed_time_secs;
  }
  
  last_error_ = error;
  last_timepoint_ = current_timepoint;

  // Compute PID output and keep it in range.
  double PID_output = kP_ * error + kI_ * integral_error_ + kD_ * derivative_error;
  PID_output = std::min(PID_output, max_output_);
  PID_output = std::max(PID_output, min_output_);

  return PID_output;
}

double PID::computeP(double reference, double signal) {
  double error = reference - signal;
  last_error_ = error;
  last_timepoint_ = std::chrono::high_resolution_clock::now();

  // Hangle angular discontinuity between PI/2 and -PI/2.
  if (type_ == Type::ANGULAR) {
    handleAngularDiscontinuity(error);
  }

  return kP_ * error;
}

void PID::restartController() {
  first_sample_computed_ = false;
  integral_error_ = 0;
  last_error_ = 0;
  last_timepoint_ = std::chrono::high_resolution_clock::now();
}

void PID::clip(double& value, double limit) {
  assert(limit >= 0);
  value = std::min(limit, std::max(-limit, value));
}

void PID::handleAngularDiscontinuity(double& error) {
  if (type_ == Type::ANGULAR) {
    // bound the error to [-2PI, 2PI] in case inputs are not in [-PI, PI].
    while (error > 2 * M_PI) {
      error -= 2 * M_PI;
    }
    while (error < -2 * M_PI) {
      error += 2* M_PI;
    }
    // Hangle angular discontinuity.
    if (error > M_PI) {
      error -= 2 * M_PI;
    } else if (error < -M_PI) {
      error += 2 * M_PI;
    }
  }
}

void PID::setKP(double kp) {
  kP_ = kp;
}

void PID::setKI(double ki) {
  kI_ = ki;
}

void PID::setKD(double kd) {
  kD_ = kd;
}

void PID::setGains(double kp, double ki, double kd) {
  kP_ = kp;
  kI_ = ki;
  kD_ = kd;
}

void PID::setMaxOutput(double max) {
  max_output_ = max;
}

void PID::setMinOutput(double min) {
  min_output_ = min;
}

void PID::setAntiWindup(double damping, double integral_error_limit) {
  assert(damping >= 0 && damping <= 1);
  antiwindup_damping_ = damping;
  integral_error_limit_ = std::fabs(integral_error_limit);
}

void PID::setType(const Type& type) {
  type_ = type;
}

}  // namespace utils
}  // namespace a17
