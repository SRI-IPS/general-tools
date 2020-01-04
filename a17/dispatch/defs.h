#pragma once

#include <chrono>
#include <string>

#include "azmq/message.hpp"
#include "a17/utils/bind.h"

namespace a17 {
namespace dispatch {

typedef uint64_t message_id;
typedef std::string message_type;

bool operator==(const azmq::message_vector &, const azmq::message_vector &);
bool operator!=(const azmq::message_vector &, const azmq::message_vector &);

/// Global function that returns the default device name. It can be configured by using the
/// "--device_name" commandline flag, or with the A17_DEVICE_NAME environment variable.
std::string DeviceName();

}  // namespace dispatch
}  // namespace a17
