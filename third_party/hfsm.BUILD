# Description:
#   Header-only heriarchical FSM framework in C++14, completely static (no dynamic allocations),
#   built with variadic templates.

licenses(["notice"])  # MIT

exports_files(["LICENSE"])

cc_library(
    name = "hfsm",
    hdrs = glob([
        "include/hfsm/machine.hpp",
        "include/hfsm/machine.inl",
        "include/hfsm/detail/*.hpp",
        "include/hfsm/detail/*.inl",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "hfsm_test",
    srcs = ["test/main.cpp"],
    size = "small",
    deps = [":hfsm"],
)
