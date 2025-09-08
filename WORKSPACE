load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# ------------------------------------------------------------------------------
# NOTE: This MUST be kept exactly in-sync with the protobuf in //CMakeLists.txt.
http_archive(
    name = "com_google_protobuf",
    sha256 = "390191a0d7884b3e52bb812c440ad1497b9d484241f37bb8e2ccc8c2b72d6c36",
    strip_prefix = "protobuf-3.19.3",
    urls = ["https://github.com/google/protobuf/archive/v3.19.3.tar.gz"],
)

http_archive(
    name = "com_google_protobuf_cc",
    sha256 = "390191a0d7884b3e52bb812c440ad1497b9d484241f37bb8e2ccc8c2b72d6c36",
    strip_prefix = "protobuf-3.19.3",
    urls = ["https://github.com/google/protobuf/archive/v3.19.3.tar.gz"],
)

# ------------------------------------------------------------------------------
## rules_python - December 6, 2022 (must be before call to rules_proto_dependencies())
http_archive(
    name = "rules_python",
    sha256 = "497ca47374f48c8b067d786b512ac10a276211810f4a580178ee9b9ad139323a",
    strip_prefix = "rules_python-0.16.1",
    url = "https://github.com/bazelbuild/rules_python/archive/refs/tags/0.16.1.tar.gz",
)

# rules_proto - December 27, 2022
http_archive(
    name = "rules_proto",
    sha256 = "dc3fb206a2cb3441b485eb1e423165b231235a1ea9b031b4433cf7bc1fa460dd",
    strip_prefix = "rules_proto-5.3.0-21.7",
    urls = [
        "https://github.com/bazelbuild/rules_proto/archive/refs/tags/5.3.0-21.7.tar.gz",
    ],
)

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")

rules_proto_dependencies()

rules_proto_toolchains()

## boringssl - January 9, 2023
# https://github.com/google/boringssl/blob/master/INCORPORATING.md
git_repository(
    name = "boringssl",
    commit = "f8a10eeef9549601a91c4525f55bf8a59d338eff",
    remote = "https://boringssl.googlesource.com/boringssl",
)

# rules_docker - June 22, 2022
http_archive(
    name = "io_bazel_rules_docker",
    sha256 = "b1e80761a8a8243d03ebca8845e9cc1ba6c82ce7c5179ce2b295cd36f7e394bf",
    urls = ["https://github.com/bazelbuild/rules_docker/releases/download/v0.25.0/rules_docker-v0.25.0.tar.gz"],
)

load("@io_bazel_rules_docker//repositories:repositories.bzl", container_repositories = "repositories")

container_repositories()

load("@io_bazel_rules_docker//container:pull.bzl", "container_pull")
load("@io_bazel_rules_docker//contrib:dockerfile_build.bzl", "dockerfile_image")

container_pull(
    name = "ubuntu2004",
    digest = "sha256:b795f8e0caaaacad9859a9a38fe1c78154f8301fdaf0872eaf1520d66d9c0b98",
    registry = "docker.io",
    repository = "ubuntu",
    tag = "20.04",
)

dockerfile_image(
    name = "service_dockerfile",
    dockerfile = "//a17/backend/docker/service:Dockerfile",
)

# ------------------------------------------------------------------------------
# rules_proto_grpc - December 04, 2022
http_archive(
    name = "rules_proto_grpc",
    sha256 = "fb7fc7a3c19a92b2f15ed7c4ffb2983e956625c1436f57a3430b897ba9864059",
    strip_prefix = "rules_proto_grpc-4.3.0",
    urls = ["https://github.com/rules-proto-grpc/rules_proto_grpc/archive/4.3.0.tar.gz"],
)

load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_repos", "rules_proto_grpc_toolchains")

rules_proto_grpc_toolchains()

rules_proto_grpc_repos()

# python grpc rules
load("@rules_proto_grpc//python:repositories.bzl", rules_proto_grpc_python_repos = "python_repos")

rules_proto_grpc_python_repos()

# python rules
load("@rules_python//python:repositories.bzl", "python_register_toolchains")

python_register_toolchains(
    name = "python3_9",
    # Available versions are listed in @rules_python//python:versions.bzl.
    # We recommend using the same version your team is already standardized on.
    python_version = "3.9",
)

load("@python3_9//:defs.bzl", "interpreter")
load("@rules_python//python:pip.bzl", "pip_parse")

pip_parse(
    name = "pip_deps",
    requirements_lock = "//:requirements.txt",
    python_interpreter_target = interpreter,
)

# cpp rules
load("@rules_proto_grpc//cpp:repositories.bzl", rules_proto_grpc_cpp_repos = "cpp_repos")

rules_proto_grpc_cpp_repos()

# grpc deps - not sure if needed
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

# ------------------------------------------------------------------------------
# NOTE: This MUST be kept exactly in-sync with the gflags in //CMakeLists.txt.
http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "ae27cdbcd6a2f935baa78e4f21f675649271634c092b1be01469440495609d0e",
    strip_prefix = "gflags-2.2.1",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.1.tar.gz"],
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

bind(
    name = "gflags_nothreads",
    actual = "@com_github_gflags_gflags//:gflags_nothreads",
)

# ------------------------------------------------------------------------------
# NOTE: This MUST be kept exactly in-sync with the glog in //CMakeLists.txt.
http_archive(
    name = "com_github_google_glog",
    build_file = "//third_party:glog.BUILD",
    sha256 = "7580e408a2c0b5a89ca214739978ce6ff480b5e7d8d7698a2aa92fadc484d1e0",
    strip_prefix = "glog-0.3.5",
    url = "https://github.com/google/glog/archive/v0.3.5.tar.gz",
)

bind(
    name = "glog",
    actual = "@com_github_google_glog//:glog",
)

# ------------------------------------------------------------------------------
http_archive(
    name = "com_github_OctoMap_octomap",
    build_file = "//third_party:octomap.BUILD",
    sha256 = "5f81c9a8cbc9526b2e725251cd3a829e5222a28201b394314002146d8b9214dd",
    strip_prefix = "octomap-1.9.0",
    urls = ["https://github.com/OctoMap/octomap/archive/v1.9.0.tar.gz"],
)

bind(
    name = "octomap",
    actual = "@com_github_OctoMap_octomap//:octomap",
)

# ------------------------------------------------------------------------------
http_archive(
    name = "eigen_archive",
    build_file = "//third_party:eigen.BUILD",
    sha256 = "858695738f7d9414ca20defa2b4b0163305c102a1d4b6b6683f23a4a25d8869c",
    strip_prefix = "eigen-3.4.0",
    urls = ["https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz"],
)

bind(
    name = "eigen",
    actual = "@eigen_archive//:eigen",
)

# GPL LICENSE DEPENDENCY - REMOVE ASAP
http_archive(
    name = "eigen_restricted_archive",
    build_file = "//third_party:eigen_restricted.BUILD",
    sha256 = "858695738f7d9414ca20defa2b4b0163305c102a1d4b6b6683f23a4a25d8869c",
    strip_prefix = "eigen-3.4.0",
    urls = ["https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz"],
)

bind(
    name = "eigen_restricted",
    actual = "@eigen_restricted_archive//:eigen_restricted",
)

# ------------------------------------------------------------------------------
http_archive(
    name = "com_github_sandstorm_io_capnproto",
    build_file = "//third_party:capnproto.BUILD",
    sha256 = "2992576f776c52a91266c26d2460d3d5f5737d266e7441865df19f54f48b0c0a",
    strip_prefix = "capnproto-0.8.0",
    urls = ["https://github.com/capnproto/capnproto/archive/refs/tags/v0.8.0.tar.gz"],
)

bind(
    name = "capnproto",
    actual = "@com_github_sandstorm_io_capnproto//:capnproto",
)

http_archive(
    name = "zmq_archive",
    build_file = "//third_party:zmq.BUILD",
    sha256 = "ea5eecbb9cc231a750535a7b74f887b18656014cc92601e8ab34734db799b796",
    strip_prefix = "libzmq-4.2.2",
    urls = ["https://github.com/zeromq/libzmq/archive/v4.2.2.tar.gz"],
)

bind(
    name = "zmq",
    actual = "@zmq_archive//:zmq",
)

http_archive(
    name = "cppzmq_archive",
    build_file = "//third_party:cppzmq.BUILD",
    sha256 = "7592759e3b08b1a459b20e03ab9720a221f7e078f4a66e44b5f8541e42b2c4e6",
    strip_prefix = "cppzmq-4.2.1",
    urls = ["https://github.com/zeromq/cppzmq/archive/v4.2.1.tar.gz"],
)

bind(
    name = "cppzmq",
    actual = "@cppzmq_archive//:cppzmq",
)

http_archive(
    name = "util_linux_archive",
    build_file = "//third_party:util_linux.BUILD",
    sha256 = "2baabb17419faaf8325792b2bb02353078128dfa42b41b09f23e0ced011c8b6d",
    strip_prefix = "util-linux-2.29.2",
    urls = ["https://github.com/karelzak/util-linux/archive/v2.29.2.tar.gz"],
)

bind(
    name = "uuid",
    actual = "@util_linux_archive//:uuid",
)

http_archive(
    name = "spdlog_archive",
    build_file = "//third_party:spdlog.BUILD",
    sha256 = "5514f7756f7e4566c1b3531b1afe252f534be94b01e4a52055627685600f6b49",
    strip_prefix = "spdlog-0.12.0",
    urls = ["https://github.com/gabime/spdlog/archive/v0.12.0.tar.gz"],
)

bind(
    name = "spdlog",
    actual = "@spdlog_archive//:spdlog",
)

http_archive(
    name = "azmq_archive",
    build_file = "//third_party:azmq.BUILD",
    sha256 = "5a588b7762a48cca70a1c1d81747c355c3c0f6f663c11451f2f84b7a13e2f75a",
    strip_prefix = "azmq-1.0.3",
    urls = ["https://github.com/zeromq/azmq/archive/v1.0.3.tar.gz"],
)

bind(
    name = "azmq",
    actual = "@azmq_archive//:azmq",
)

http_archive(
    name = "sodium_archive",
    build_file = "//third_party:sodium.BUILD",
    sha256 = "4d1b824f94b304033230a11545633a696a0904014f3d1323605282255740422c",
    strip_prefix = "libsodium-1.0.10",
    urls = ["https://github.com/jedisct1/libsodium/archive/1.0.10.tar.gz"],
)

bind(
    name = "sodium",
    actual = "@sodium_archive//:sodium",
)

http_archive(
    name = "xerces_archive",
    build_file = "//third_party:xerces.BUILD",
    sha256 = "b75f56c547805950d24e9411295b65101a073f8373e3519b5d3c81e9f291350a",
    strip_prefix = "xerces-c-3.1.3",
    urls = ["https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.3.tar.gz"],
)

bind(
    name = "xerces",
    actual = "@xerces_archive//:xerces",
)

http_archive(
    name = "googletest_archive",
    build_file = "//third_party:googletest.BUILD",
    sha256 = "9846337525b64267e72152a51a2a4b81c2f1f3e7952a1d2e2b34914104032d84",
    strip_prefix = "googletest-release-1.8.0",
    urls = ["https://github.com/google/googletest/archive/release-1.8.0.tar.gz"],
)

bind(
    name = "googletest",
    actual = "@googletest_archive//:gtest",
)

# git_repository(
#     name = "catch_archive",
#     build_file = "//third_party:catch.BUILD",
#     branch = "master",
#     remote = "https://github.com/catchorg/Catch2.git",
# )

http_archive(
    name = "catch_archive",
    # This attribute IS valid for http_archive in Bazel 5.4.1
    build_file = "//third_party:catch.BUILD",
    sha256 = "524f4693a7d18e24749f5a4325c613fe82b853a1656c1ce89b25f49896dc4047",
    strip_prefix = "Catch2-f87b475df8a73a388556e409b3b83870b28e6c4e",
    urls = ["https://github.com/catchorg/Catch2/archive/f87b475df8a73a388556e409b3b83870b28e6c4e.tar.gz"],
)

bind(
    name = "catch",
    actual = "@catch_archive//:catch",
)

# TODO(curtismuntz): This is actually unused unless specifically called... Do we need this at all?
# Maybe we should configure this toolchain to be clang, or set bazel.rc flags to point to this
# compiler to enforce the same gcc version as the cross-compiler.
http_archive(
    name = "org_linaro_components_toolchain_gcc",
    build_file = "//third_party:linaro_linux_gcc.BUILD",
    sha256 = "987941c9fffdf56ffcbe90e8984673c16648c477b537fcf43add22fa62f161cd",
    strip_prefix = "gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabihf",
    url = "https://bazel-mirror.storage.googleapis.com/releases.linaro.org/components/toolchain/binaries/latest-5/arm-linux-gnueabihf/gcc-linaro-5.3.1-2016.05-x86_64_arm-linux-gnueabihf.tar.xz",
)

# Keep this compiler in sync with the kernel cross-compiler in the //kernel/cross_compile_kernel.sh
# file.
http_archive(
    name = "org_linaro_components_toolchain_gcc_aarch64",
    build_file = "//third_party:linaro_linux_gcc_aarch64.BUILD",
    sha256 = "8109232f6882d8a52c37a2062e66414465dcf20bdf6794ad6dc9188a1075f1bd",
    strip_prefix = "gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu",
    url = "https://releases.linaro.org/components/toolchain/binaries/5.4-2017.05/aarch64-linux-gnu/gcc-linaro-5.4.1-2017.05-x86_64_aarch64-linux-gnu.tar.xz",
)

new_local_repository(
    name = "qt",
    build_file = "//third_party:qt.BUILD",
    path = "/usr/include/x86_64-linux-gnu/qt5",
)

new_local_repository(
    name = "cuda_local",
    build_file = "//third_party:cuda.BUILD",
    path = "/usr/local/cuda",
)

bind(
    name = "cuda",
    actual = "@cuda_local//:cuda",
)

http_archive(
    name = "openssl_archive",
    build_file = "//third_party:openssl.BUILD",
    sha256 = "a3d3a7c03c90ba370405b2d12791598addfcafb1a77ef483c02a317a56c08485",
    url = "https://github.com/openssl/openssl/archive/OpenSSL_1_0_2l.tar.gz",
)

bind(
    name = "openssl",
    actual = "@openssl_archive//:openssl",
)

# Jan 19, 2022
http_archive(
    name = "googleapis_archive",
    build_file = "//third_party:googleapis.BUILD",
    sha256 = "df585427959ebb9949ca12a45888b50a5b8edcb2b270aba1800225d159a49ff6",
    strip_prefix = "googleapis-9fe00a1330817b5ce00919bf2861cd8a9cea1a00",
    urls = ["https://github.com/googleapis/googleapis/archive/9fe00a1330817b5ce00919bf2861cd8a9cea1a00.tar.gz"],
)

bind(
    name = "googleapis_protos",
    actual = "@googleapis_archive//:protos",
)

bind(
    name = "googleapis_py_proto",
    actual = "@googleapis_archive//:py_proto",
)

# master/HEAD as of July 2, 2018.
http_archive(
    name = "hfsm_archive",
    build_file = "//third_party:hfsm.BUILD",
    sha256 = "820113624d6424f76759eb7918b8b3a4f597fd3fae6c1f95a4a0de85c4b03a91",
    strip_prefix = "HFSM-fb35d57bf999847fca7188929a893b1e36399456",
    url = "https://github.com/andrew-gresyk/HFSM/archive/fb35d57bf999847fca7188929a893b1e36399456.tar.gz",
)

bind(
    name = "hfsm",
    actual = "@hfsm_archive//:hfsm",
)

http_archive(
    name = "cpplint_archive",
    build_file = "//third_party:cpplint.BUILD",
    sha256 = "952d19b925ebfefbd2a7724f48682dc42617965131f8058a4732fa167734c416",
    strip_prefix = "cpplint-5a38c3a6446914740884a976c9b5cded86b98e1c",
    urls = ["https://github.com/cpplint/cpplint/archive/5a38c3a6446914740884a976c9b5cded86b98e1c.tar.gz"],
)

http_archive(
    name = "pycodestyle_archive",
    build_file = "//third_party:pycodestyle.BUILD",
    sha256 = "559a8c0aab9140ff897ca834f26b6a0328a11278936c8c6ff6653401904b4540",
    strip_prefix = "pycodestyle-f4f6e556faea76809b70413fea9f496288d56713",
    urls = ["https://github.com/PyCQA/pycodestyle/archive/f4f6e556faea76809b70413fea9f496288d56713.tar.gz"],
)

# Aug 14, 2019
http_archive(
    name = "subpar",
    strip_prefix = "subpar-9fae6b63cfeace2e0fb93c9c1ebdc28d3991b16f",
    urls = ["https://github.com/google/subpar/archive/9fae6b63cfeace2e0fb93c9c1ebdc28d3991b16f.tar.gz"],
)

# Mavlink submodule version as of PX4 v1.8.0
# https://github.com/mavlink/c_library_v2/tree/033fa8e7a4a75a0c3f17cea57e3be8966e05f770
http_archive(
    name = "mavlink_archive",
    build_file = "//third_party:mavlink.BUILD",
    sha256 = "aa2e5ab3b860bceca352a599cef934538c9dd855e0c0c9d940baa6e03226e5e0",
    strip_prefix = "c_library_v2-033fa8e7a4a75a0c3f17cea57e3be8966e05f770",
    url = "https://github.com/mavlink/c_library_v2/archive/033fa8e7a4a75a0c3f17cea57e3be8966e05f770.tar.gz",
)

bind(
    name = "mavlink",
    actual = "@mavlink_archive//:mavlink",
)