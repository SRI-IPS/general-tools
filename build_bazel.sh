#!/bin/bash
#
# This script builds and tests all Bazel components.
# It is designed to be run INSIDE the Docker container.

set -e

echo '>>> BUILDING AND TESTING WITH BAZEL <<<'
cd "${A17_ROOT}"
bazel --output_user_root=/tmp/bazel_output build //...
bazel --output_user_root=/tmp/bazel_output test //...
echo "--- Bazel build and test completed successfully ---"