# Description:
#   Very fast, header only, C++ logging library.

licenses(["notice"])  # MIT License

cc_library(
    name = "spdlog",
    hdrs = glob(["include/spdlog/**"]),
    includes = ["include"],
    linkopts = ["-lpthread"],
    visibility = ["//visibility:public"],
)
