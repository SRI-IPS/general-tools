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
echo '>>> BUILDING AND TESTING WITH CMAKE <<<'
cd /workspace/a17/dispatch
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./unittests_A17Dispatch

echo '>>> BUILDING AND TESTING WITH BAZEL <<<'
cd /workspace
bazel test //...
"

# Run the commands non-interactively inside the container
docker run --rm -v "$(pwd)":/workspace -w /workspace --user $(id -u):$(id -g) ${IMAGE_NAME} /bin/bash -c "${CI_COMMANDS}"

echo "--- CI run completed successfully ---"