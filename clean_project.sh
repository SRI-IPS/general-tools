#!/bin/bash
#
# This script cleans all build artifacts for both CMake and Bazel.
# It is designed to be run INSIDE the Docker container.

set -e

echo '>>> CLEANING PREVIOUS BUILD ARTIFACTS <<<'

# Clean CMake build directories and install directory
rm -rf /workspace/third_party/build
rm -rf /workspace/a17/utils/build
rm -rf /workspace/a17/capnp_msgs/build
rm -rf /workspace/a17/dispatch/build
rm -rf /workspace/install

# Clean Bazel cache
cd /workspace && bazel clean --expunge