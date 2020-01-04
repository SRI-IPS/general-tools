#ifndef A17_UTILS_PID_H_
#define A17_UTILS_PID_H_

#include <chrono>

namespace a17{
namespace utils{

/// Simple implementation of a PID controller.
class PID {
 public:
  /// ANGULAR controllers reduce the error to [-PI, PI].
  enum class Type {
    LINEAR,
    ANGULAR
  };
  static constexpr Type kDefaultType = Type::LINEAR;

  PID(double kp, double ki, double kd);

  /// Performs a PID iteration, with error = #reference - #signal.
  /// @returns Output of the PID computation.
  /// @param[in] reference What the controller is trying to follow.
  /// @param[in] signal Current state of the system.
  double compute(double reference, double signal);

  /// Performs a PID iteration, but returns only the proportional part of the output.
  /// @param[in] reference What the controller is trying to follow.
  /// @param[in] signal Current state of the system.
  double computeP(double reference, double signal);

  /// Helper setter functions.
  void setKP(double kp);
  void setKI(double ki);
  void setKD(double kd);
  void setGains(double kp, double ki, double kd);
  void setMaxOutput(double max);
  void setMinOutput(double min);
  void setAntiWindup(double damping, double integral_error_limit);
  void setType(const Type& type);

  /// Resets the accumulated integral error, last error and last time variables.
  void restartController();

  /// For testing
  friend class PIDUnitTests;

 private:
  /// Clips #value in place between [-limit, limit].
  void clip(double& value, double limit);

  /// Reduces the error to [-PI, PI].
  void handleAngularDiscontinuity(double& error);

  double kP_; ///< Proportional gain.
  double kI_; ///< Integral gain.
  double kD_; ///< Derivative gain.
  double min_output_; ///< Limit output of the PID.
  double max_output_; ///< Limit output of the PID.

  /// Accumulated integral error will be multiplied by this factor before each computation.
  double antiwindup_damping_; ///< Should be between (0, 1).
  /// Integral error will be clipped by this value after each computation.
  double integral_error_limit_;

  Type type_; ///< Linear or angular. Angular PIDs are treated differently.
  bool first_sample_computed_; ///< Flag.

  double integral_error_; ///< Accumulated integral error.
  double last_error_;     ///< Error in last computation.

  /// Timepoint of the latest computation.
  std::chrono::time_point<std::chrono::high_resolution_clock> last_timepoint_;
};

}  // namespace utils
}  // namespace a17

#endif  // A17_UTILS_PID_H_
