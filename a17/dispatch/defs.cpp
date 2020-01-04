#include "defs.h"

#include "gflags/gflags.h"

namespace a17 {
namespace dispatch {

namespace {

DEFINE_string(device_name, "", "Hardware-assigned device id. Defaults to A17_DEVICE_NAME env var");

}

std::string DeviceName() {
  if (FLAGS_device_name != "") {
    return FLAGS_device_name;
  }
  auto env_device_name = std::getenv("A17_DEVICE_NAME");
  if (env_device_name) {
    return std::string{env_device_name};
  }
  return std::string{""};
}

}  // namespace dispatch
}  // namespace a17
