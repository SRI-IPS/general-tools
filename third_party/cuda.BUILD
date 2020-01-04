config_setting(
    name = "aarch64-linux-gnu",
    values = {
        "crosstool_top": "//tools/arm_compiler:toolchain",
    },
)

config_setting(
    name = "x86_64-linux-gnu",
    values = {
        "cpu": "k8",
    },
)

cc_library(
    name = "cuda",
    hdrs = select({
        ":aarch64-linux-gnu": glob(["targets/aarch64-linux/include/**"]),
        ":x86_64-linux-gnu": glob(["targets/x86_64-linux/include/**"]),
    }),
    includes = select({
        ":aarch64-linux-gnu": ["targets/aarch64-linux/include"],
        ":x86_64-linux-gnu": ["targets/x86_64-linux/include"],
    }),
    # Add the directories and linker flags, but don't add the libs as sources. The reason for this
    # is because when cross-compiling for aarch64, cuda only installs "stub" libraries. We don't
    # want to use those when the code is deployed, we want to use the cuda libs that are
    # system-installed on the device.
    linkopts = select({
        ":aarch64-linux-gnu": [
            "-L/usr/local/cuda/targets/aarch64-linux/lib",
            "-L/usr/local/cuda/targets/aarch64-linux/lib/stubs",
        ],
        # NOTE: On the host system, the stubs directory won't exist. But that is fine.
        ":x86_64-linux-gnu": [
            "-L/usr/local/cuda/targets/x86_64-linux/lib",
            "-L/usr/local/cuda/targets/x86_64-linux/lib/stubs",
        ],
    }) + [
        # These 5 libs are all that opencv requires.
        # vt/hqstereo requires just cudart.
        # NOTE: DO NOT link against cublas or nvblas. They interfere with vtlapack.
        "-lcudart",
        "-lcufft",
        "-lnppc",
        # nppi is no longer around for CUDA version 8.0+, let's get these from opencv_cuda_libs?
        #"-lnppi",
        "-lnppial",
        "-lnppicc",
        "-lnppicom",
        "-lnppidei",
        "-lnppif",
        "-lnppig",
        "-lnppim",
        "-lnppist",
        "-lnppisu",
        "-lnppitc",
        "-lnpps",
    ],
    visibility = ["//visibility:public"],
)
