package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD 3-Clause

cc_library(
    name = "octomap",
    srcs = [
        "octomap/src/AbstractOcTree.cpp",
        "octomap/src/AbstractOccupancyOcTree.cpp",
        "octomap/src/ColorOcTree.cpp",
        "octomap/src/CountingOcTree.cpp",
        "octomap/src/OcTree.cpp",
        "octomap/src/OcTreeNode.cpp",
        "octomap/src/OcTreeStamped.cpp",
        "octomap/src/Pointcloud.cpp",
        "octomap/src/ScanGraph.cpp",
    ],
    hdrs = glob(["octomap/include/octomap/*.h*"]),
    copts = ["-fopenmp"],
    includes = ["octomap/include"],
    linkopts = ["-fopenmp"],
    deps = [":math"],
)

cc_library(
    name = "math",
    srcs = [
        "octomap/src/math/Pose6D.cpp",
        "octomap/src/math/Quaternion.cpp",
        "octomap/src/math/Vector3.cpp",
    ],
    hdrs = glob(["octomap/include/octomap/math/*.h*"]),
    includes = ["octomap/include"],
)
