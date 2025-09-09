#!/bin/bash
set -e

IMAGE_NAME="a17-tools-dev"

# Check if the Docker image exists (same as before)
if [[ "$(docker images -q ${IMAGE_NAME}:latest 2> /dev/null)" == "" ]]; then
  echo "Docker image '${IMAGE_NAME}' not found. Building..."
  docker build -t ${IMAGE_NAME} -f Dockerfile .
  echo "Docker image built successfully."
fi

echo "Starting development container. Your project directory is mounted at /workspace."
echo "To exit the container, type 'exit'."

# This is the new, more advanced run command
# It will create a user inside the container that matches your host user
docker run -it --rm \
  -v "$(pwd)":/workspace:z \
  -w /workspace \
  -e HOST_UID=$(id -u) \
  -e HOST_GID=$(id -g) \
  ${IMAGE_NAME} /bin/bash -c '
    # Create a group and user on the fly with the host's IDs
    groupadd -f -g $HOST_GID user
    useradd -o -u $HOST_UID -g $HOST_GID -s /bin/bash -m user
    
    # Switch to the newly created user and start a shell
    export HOME=/home/user
    exec su user
  '
  
echo "Exited development container."