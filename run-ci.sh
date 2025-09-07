#!/bin/bash
set -e

IMAGE_NAME="a17-tools-dev"

# Ensure the Docker image is built. This reuses the logic from docker-dev.sh
if [[ "$(docker images -q ${IMAGE_NAME}:latest 2> /dev/null)" == "" ]]; then
  echo "Docker image '${IMAGE_NAME}' not found. Building..."
  docker build \
    --build-arg UID=$(id -u) \
    --build-arg GID=$(id -g) \
    -t ${IMAGE_NAME} \
    -f Dockerfile .
  echo "Docker image built successfully."
fi

echo "--- Running CI build and tests inside Docker container ---"

# Define the sequence of commands to run inside the container
CI_COMMANDS="
set -e
echo '>>> CLEANING PREVIOUS BUILD ARTIFACTS <<<'
# Clean CMake build directories
rm -rf /workspace/a17/utils/build
rm -rf /workspace/a17/dispatch/build
rm -rf /workspace/install
# Clean Bazel cache
cd /workspace && bazel clean --expunge

echo '>>> BUILDING AND TESTING WITH CMAKE (in order) <<<'

# 1. Create a local directory to install libraries into
INSTALL_DIR=/workspace/install
mkdir -p \${INSTALL_DIR}

# 2. Build and install a17-utils
echo '--- Building a17-utils ---'
cd /workspace/a17/utils
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=\${INSTALL_DIR}
make install

# 3. Build and test a17-dispatch, pointing it to our local install directory
echo '--- Building a17-dispatch ---'
cd /workspace/a17/dispatch
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=\${INSTALL_DIR}
make
./unittests_A17Dispatch

echo '>>> BUILDING AND TESTING WITH BAZEL <<<'
cd /workspace
bazel test //...
"

# Run the commands non-interactively inside the container
docker run --rm -v "$(pwd)":/workspace -w /workspace --user $(id -u):$(id -g) ${IMAGE_NAME} /bin/bash -c "${CI_COMMANDS}"

echo "--- CI run completed successfully ---"