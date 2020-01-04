# Description
#   OpenSSL is a robust, commercial-grade, and full-featured toolkit for the Transport Layer
#   Security (TLS) and Secure Sockets Layer (SSL) protocols. It is also a general-purpose
#   cryptography library.

licenses(["notice"]) # Double license: OpenSSL License and the original SSLeay license apply.

exports_files(["LICENSE"])

cc_library(
    name = "openssl",
    visibility = ["//visibility:public"],
    deps = [
        ":crypto",
        ":ssl",
    ],
)

# Currently installed by:
#   sudo apt-get install libssl-dev
# TODO(murt): Implement proper build procedure.
cc_library(
    name = "ssl",
    linkopts = ["-lssl"],
    visibility = ["//visibility:public"],
)

# Currently installed by:
#   sudo apt-get install libcrypto++-dev
# TODO(murt): Implement proper build procedure.
cc_library(
    name = "crypto",
    linkopts = ["-lcrypto"],
    visibility = ["//visibility:public"],
)
