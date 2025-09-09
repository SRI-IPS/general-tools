#!/bin/bash
set -e

IMAGE_NAME="a17-tools-dev"

# Check if the Docker image exists
if [[ "$(docker images -q ${IMAGE_NAME}:latest 2> /dev/null)" == "" ]]; then
  echo "Docker image '${IMAGE_NAME}' not found. Building..."
  # The Dockerfile is now self-contained and doesn't require external build-args.
  docker build -t ${IMAGE_NAME} -f Dockerfile .
  echo "Docker image built successfully."
fi

echo "Starting development container. Your project directory is mounted at /workspace."
echo "To exit the container, type 'exit'."

# Start the container
docker run -it --rm \
  -v "$(pwd)":/workspace:z \
  -w /workspace \
  --user "$(id -u):$(id -g)" \
  -e HOME=/workspace \
  -e USER="$USER" \
  ${IMAGE_NAME} \
  /bin/bash

echo "Exited development container."