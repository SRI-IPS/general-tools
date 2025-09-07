#!/bin/bash
set -e

IMAGE_NAME="a17-tools-dev"

# Check if the Docker image exists
if [[ "$(docker images -q ${IMAGE_NAME}:latest 2> /dev/null)" == "" ]]; then
  echo "Docker image '${IMAGE_NAME}' not found. Building..."
  docker build \
    --build-arg UID=$(id -u) \
    --build-arg GID=$(id -g) \
    -t ${IMAGE_NAME} \
    -f Dockerfile .
  echo "Docker image built successfully."
fi

echo "Starting development container. Your project directory is mounted at /workspace."
echo "To exit the container, type 'exit'."

# Start the container
docker run -it --rm \
  -v "$(pwd)":/workspace \
  -w /workspace \
  --user $(id -u):$(id -g) \
  ${IMAGE_NAME} \
  /bin/bash

echo "Exited development container."