package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD license

#TODO(murt) split this library up into individual libraries
cc_library(
    name = "vtk",
    srcs = glob(["lib/libvtk*.so"]),
    hdrs = glob([
        "include/vtk-6.3/**/*.h",
        "include/vtk-6.3/**/*.txx",
    ]),
    includes = ["include/vtk-6.3/"],
    visibility = ["//visibility:public"],
    deps = ["@//cmake-out/boost"],
)
