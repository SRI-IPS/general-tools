#!/bin/bash
set -e

echo "--- Running CI build and tests inside Docker container ---"

IMAGE_NAME="a17-tools-dev:latest"

# Run the commands non-interactively inside the container
# The Docker image is now built by the GitHub Actions workflow itself.
docker run --rm -v "$(pwd)":/workspace:z -w /workspace --user $(id -u):$(id -g) ${IMAGE_NAME} /bin/bash -c "
  # Ensure scripts inside the container are executable, as permissions can be lost from git.
  chmod +x /workspace/*.sh

  set -e
  echo '--- Running CMake build and tests ---'
  /workspace/build_project.sh

  # To run the bazel build as well, uncomment the following lines:
  # echo '--- Running Bazel build and tests ---'
  # /workspace/build_bazel.sh
"

echo "--- CI run completed successfully ---"