package(default_visibility = ["//visibility:public"])
# Mavlink c_library_v2 is a library generated from https://github.com/mavlink/mavlink
# https://github.com/mavlink/c_library_v2

# MAVLink -- Micro Air Vehicle Message Marshalling Library.
#  MAVLink is a very lightweight, header-only message library for communication between drones
#  and/or ground control stations. It consists primarily of message-set specifications for different
#  systems ("dialects") defined in XML files, and Python tools that convert these into appropriate
#  source code for supported languages. There are additional Python scripts providing examples and
#  utilities for working with MAVLink data.

licenses(["notice"])  # MIT License https://mavlink.io/en/#license

cc_library(
    name = "mavlink",
    hdrs = glob(["**/*.h"]),
    include_prefix = "mavlink",
    visibility = ["//visibility:public"],
)
