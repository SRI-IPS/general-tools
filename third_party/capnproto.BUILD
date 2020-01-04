# Description:
#   Cap’n Proto is an insanely fast data interchange format and capability-based RPC system.
#   Think JSON, except binary. Or think Protocol Buffers, except faster. In fact, in benchmarks,
#   Cap’n Proto is INFINITY TIMES faster than Protocol Buffers.
#
#   This benchmark is, of course, unfair. It is only measuring the time to encode and decode a
#   message in memory. Cap’n Proto gets a perfect score because there is no encoding/decoding step.
#   The Cap’n Proto encoding is appropriate both as a data interchange format and an in-memory
#   representation, so once your structure is built, you can simply write the bytes straight out to
#   disk!

licenses(["notice"])  # MIT

exports_files(["LICENSE"])

cc_library(
    name = "kj",
    srcs = glob(
        ["c++/src/kj/**/*.c++"],
        exclude = ["c++/src/kj/**/*-test.c++"],
    ),
    hdrs = glob(["c++/src/kj/**/*.h"]),
    includes = ["c++/src"],
    defines = ["KJ_USE_FUTEX=0"],
    copts = [
        "-Wno-maybe-uninitialized",
        "-Wno-sign-compare",
        "-Wno-strict-aliasing",
        "-Wno-unused-variable",
    ],
    linkopts = ["-pthread"],
)

cc_library(
    name = "capnproto",
    srcs = glob([
            "c++/src/capnp/*.c++",
            "c++/src/capnp/compiler/*.c++",
        ], exclude = [
            "c++/src/capnp/**/*-test.c++",
            "c++/src/capnp/**/*-testcase.c++",
            "c++/src/capnp/compiler/capnp.c++",
            "c++/src/capnp/compiler/capnpc-c++.c++",
            "c++/src/capnp/compiler/capnpc-capnp.c++",
            "c++/src/capnp/test-util.c++",
        ]
    ),
    hdrs = glob([
        "c++/src/capnp/*.h",
        "c++/src/capnp/compiler/*.h",
    ]),
    deps = [":kj"],
    includes = ["c++/src"],
    copts = [
        "-Wno-maybe-uninitialized",
        "-Wno-sign-compare",
    ],
    visibility = ["//visibility:public"]
)

cc_binary(
    name = "capnp",
    srcs = ["c++/src/capnp/compiler/capnp.c++"],
    deps = [":capnproto"],
    visibility = ["//visibility:public"]
)

cc_binary(
    name = "capnpc-c++",
    srcs = ["c++/src/capnp/compiler/capnpc-c++.c++"],
    deps = [":capnproto"],
    visibility = ["//visibility:public"]
)

cc_binary(
    name = "capnpc-capnp",
    srcs = ["c++/src/capnp/compiler/capnpc-capnp.c++"],
    deps = [":capnproto"],
    visibility = ["//visibility:public"]
)

filegroup(
    name = "capnp-capnp",
    srcs = glob(["c++/src/capnp/**/*.capnp"]),
    visibility = ["//visibility:public"]
)
