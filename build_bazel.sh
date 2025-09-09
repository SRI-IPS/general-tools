#!/bin/bash
#
# This script builds and tests all Bazel components.
# It is designed to be run INSIDE the Docker container.

set -e

echo '>>> BUILDING AND TESTING WITH BAZEL <<<'
cd "${A17_ROOT}"
bazel build //...
bazel test //...
echo "--- Bazel build and test completed successfully ---"