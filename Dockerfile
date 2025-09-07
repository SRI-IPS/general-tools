# Dockerfile for a17-tools build environment
# This sets up an Ubuntu 20.04 container with all dependencies needed to build
# and test the project using either CMake or Bazel.
# It builds dependencies from source to ensure specific versions are used.
FROM ubuntu:20.04

# Install base dependencies, build tools, and Bazel from repositories
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    wget \
    curl \
    unzip \
    python3-pip \
    python3-dev \
    libboost-all-dev \
    # For Bazel installation
    apt-transport-https \
    gnupg \
    && curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > /etc/apt/trusted.gpg.d/bazel.gpg \
    && echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list \
    && apt-get update && apt-get install -y bazel \
    && rm -rf /var/lib/apt/lists/*

# Copy and run the dependency installation script
COPY install_dependencies.sh /usr/local/bin/install_dependencies.sh
RUN chmod +x /usr/local/bin/install_dependencies.sh && \
    /usr/local/bin/install_dependencies.sh && \
    rm /usr/local/bin/install_dependencies.sh

# Install project-specific Python dependencies via pip
RUN pip3 install --no-cache-dir \
    pycapnp \
    pyzmq \
    tornado \
    netifaces

# Create a non-root user to match the host user, avoiding file permission issues.
# The user ID and group ID will be passed in during the build.
ARG UID=1000
ARG GID=1000
RUN groupadd -g $GID user && useradd -u $UID -g $GID -ms /bin/bash user

# Set final working directory for the container
WORKDIR /workspace