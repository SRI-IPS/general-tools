#!/bin/bash
set -e

IMAGE_NAME="a17-tools-dev"

# Read bazel version for the build-arg
BAZEL_VERSION=$(cat .bazelversion)

# Ensure the Docker image is built. This reuses the logic from docker-dev.sh
if [[ "$(docker images -q ${IMAGE_NAME}:latest 2> /dev/null)" == "" ]]; then
  echo "Docker image '${IMAGE_NAME}' not found. Building..."
  docker build \
    --build-arg BAZEL_VERSION=${BAZEL_VERSION} \
    --build-arg UID=$(id -u) \
    --build-arg GID=$(id -g) \
    -t ${IMAGE_NAME} \
    -f Dockerfile .
  echo "Docker image built successfully."
fi

echo "--- Running CI build and tests inside Docker container ---"

# Run the commands non-interactively inside the container
docker run --rm -v "$(pwd)":/workspace -w /workspace --user $(id -u):$(id -g) ${IMAGE_NAME} /bin/bash -c "
  /workspace/build_project.sh
  # To run the bazel build as well, uncomment the following line:
  # /workspace/build_bazel.sh
"

echo "--- CI run completed successfully ---"