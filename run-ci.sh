#!/bin/bash
set -e

echo "--- Running CI build and tests inside Docker container ---"

IMAGE_NAME="a17-tools-dev:latest"
CONTAINER_NAME="ci-build-container"

# Clean up any previous container with the same name to avoid conflicts
docker rm -f ${CONTAINER_NAME} > /dev/null 2>&1 || true

# Run the build inside a container.
# The --rm flag ensures the container is automatically removed after it exits,
# regardless of whether the build succeeds or fails.
docker run --rm --name "${CONTAINER_NAME}" \
  -v "$(pwd)":/workspace:z \
  -w /workspace \
  --user "$(id -u):$(id -g)" \
  -e HOME=/workspace \
  -e USER="$USER" \
  "${IMAGE_NAME}" /bin/bash -c '
  set -e # Exit immediately if a command fails inside the container.

  # Ensure scripts inside the container are executable, as permissions can be lost from git.
  chmod +x /workspace/*.sh

  echo "--- Running CMake build and tests ---"
  /workspace/build_project.sh

  # To run the bazel build as well, uncomment the following lines:
  # echo "--- Running Bazel build and tests ---"
  # /workspace/build_bazel.sh
'

echo "--- CI run completed. ---"