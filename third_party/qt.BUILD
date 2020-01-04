cc_library(
    name = "qt_core",
    hdrs = glob(["QtCore/**"]),
    includes = [
        ".",
        "QtCore",
    ],
    linkopts = ["-lQt5Core"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "qt_widgets",
    hdrs = glob(["QtWidgets/**"]),
    includes = [
        ".",
        "QtWidgets",
    ],
    linkopts = ["-lQt5Widgets"],
    visibility = ["//visibility:public"],
    deps = [
        ":qt_core",
        ":qt_gui",
    ],
)

cc_library(
    name = "qt_gui",
    hdrs = glob(["QtGui/**"]),
    includes = ["."],
    linkopts = ["-lQt5Gui"],
    visibility = ["//visibility:public"],
    deps = [":qt_core"],
)

