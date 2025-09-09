# Dockerfile for a17-tools build environment
# This sets up an Ubuntu 20.04 container with all dependencies needed to build
# and test the project using either CMake or Bazel.
# It builds dependencies from source to ensure specific versions are used.
FROM ubuntu:20.04

# Avoid prompts from apt during installation
ENV DEBIAN_FRONTEND=noninteractive

# Define Bazelisk version to install
ARG BAZELISK_VERSION=v1.19.0

# Install base dependencies and build tools.
# We will install Bazel using Bazelisk instead of apt.
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    wget \
    curl \
    python3-pip \
    python3-dev \
    libboost-all-dev \
    # For Bazelisk download
    unzip \
    # Clean up apt lists to reduce image size
    && rm -rf /var/lib/apt/lists/*

# Install Bazelisk, which will read .bazelversion and use the correct Bazel version
RUN wget https://github.com/bazelbuild/bazelisk/releases/download/${BAZELISK_VERSION}/bazelisk-linux-amd64 -O /usr/local/bin/bazel \
    && chmod +x /usr/local/bin/bazel

# Copy and run the dependency installation script
COPY install_apt_dependencies.sh /usr/local/bin/install_apt_dependencies.sh
RUN chmod +x /usr/local/bin/install_apt_dependencies.sh && \
    /usr/local/bin/install_apt_dependencies.sh && \
    rm /usr/local/bin/install_apt_dependencies.sh

# Install Python dependencies from requirements file
COPY requirements.txt /tmp/requirements.txt
RUN pip3 install --no-cache-dir -r /tmp/requirements.txt

# Create a non-root user to match the host user, avoiding file permission issues.
# The user ID and group ID will be passed in during the build.
ARG UID=1000
ARG GID=1000
RUN groupadd -g $GID user && useradd -u $UID -g $GID -ms /bin/bash user

# Set final working directory for the container
WORKDIR /workspace

# Set the A17_ROOT environment variable for CMake and other tools to find project files.
ENV A17_ROOT=/workspace