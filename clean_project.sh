#!/bin/bash
#
# This script cleans all build artifacts for both CMake and Bazel.
# It is designed to be run INSIDE the Docker container.

set -e

echo '>>> CLEANING PREVIOUS BUILD ARTIFACTS <<<'

# Clean CMake build directories and install directory
rm -rf "${A17_ROOT}/third_party/build"
rm -rf "${A17_ROOT}/a17/utils/build"
rm -rf "${A17_ROOT}/a17/capnp_msgs/build"
rm -rf "${A17_ROOT}/a17/dispatch/build"
rm -rf "${A17_ROOT}/install"

# Clean Bazel cache
cd "${A17_ROOT}" && bazel --output_user_root=/tmp/bazel_output clean --expunge