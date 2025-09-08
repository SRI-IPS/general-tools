#!/bin/bash
#
# This script builds and tests all CMake and Bazel components.
# It is designed to be run INSIDE the Docker container.

set -e

#echo '>>> CLEANING PREVIOUS BUILD ARTIFACTS <<<'
# Clean CMake build directories
#rm -rf /workspace/third_party/build
#rm -rf /workspace/a17/utils/build
#rm -rf /workspace/a17/capnp_msgs/build
#rm -rf /workspace/a17/dispatch/build
#rm -rf /workspace/install
# Clean Bazel cache
# cd /workspace && bazel --enable_bzlmod=false clean --expunge

echo '>>> BUILDING AND TESTING WITH CMAKE (in order) <<<'

# 1. Create a local directory to install libraries into
INSTALL_DIR=/workspace/install
mkdir -p ${INSTALL_DIR}

# 2. Build and install all of third_party
echo '--- Building third_party dependencies ---'
cd /workspace/third_party
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
make -j4 install # This will build and install Catch, among others

# 3. Build, test, and install a17-utils
echo '--- Building a17-utils ---'
cd /workspace/a17/utils
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_PREFIX_PATH=${INSTALL_DIR}
make -j4
./unittests_A17Utils
make install

# 4. Build, test, and install a17-capnp_msgs
echo '--- Building a17-capnp_msgs ---'
cd /workspace/a17/capnp_msgs
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_PREFIX_PATH=${INSTALL_DIR}
make -j4 install

# 5. Build, test, and install a17-maths
echo '--- Building a17-maths ---'
cd /workspace/a17/maths
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_PREFIX_PATH=${INSTALL_DIR}
make -j4
./unittests_A17Maths
make install

# 5. Build and test a17-dispatch, pointing it to our local install directory
echo '--- Building a17-dispatch ---'
cd /workspace/a17/dispatch
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${INSTALL_DIR}
make -j4
./unittests_A17Dispatch

echo '>>> BUILDING AND TESTING WITH BAZEL <<<'
cd /workspace
#bazel build //third_party/...
bazel test //...

echo "--- Build and test completed successfully ---"