# Description:
#   util-linux is a standard package of the Linux operating system. A fork,
#   util-linux-ng—with ng meaning "next generation"—was created when development
#   stalled,[3] but as of January 2011 has been renamed back to util-linux, and
#   is the official version of the package

licenses(["notice"])  # BSD 3-Clause

exports_files(["libuuid/COPYING"])

UUID_COPTS = [
    "-DHAVE_USLEEP",
    "-Iinclude",
]

cc_library(
    name = "uuid_needed_headers",
    srcs = [
        "include/all-io.h",
        "include/c.h",
        "include/nls.h",
        "include/randutils.h",
        "include/strutils.h",
        "lib/randutils.c",
    ],
    copts = UUID_COPTS,
    includes = ["include"],
    visibility = ["//visibility:private"],
)

cc_library(
    name = "uuid",
    srcs = glob([
        "libuuid/src/*.h",
        "libuuid/src/*.c",
    ]),
    hdrs = ["libuuid/src/uuid.h"],
    copts = UUID_COPTS,
    include_prefix = "uuid",
    includes = ["include"],
    strip_include_prefix = "libuuid/src",
    visibility = ["//visibility:public"],
    deps = [":uuid_needed_headers"],
)
