# Dockerfile for a17-tools build environment
# This sets up an Ubuntu 20.04 container with all dependencies needed to build
# and test the project using either CMake or Bazel.
# It builds dependencies from source to ensure specific versions are used.
FROM ubuntu:20.04

# Avoid prompts from apt during installation
ENV DEBIAN_FRONTEND=noninteractive

# Define dependency versions
ARG SODIUM_VERSION="1.0.10"
ARG ZMQ_VERSION="4.2.2"
ARG CPPZMQ_VERSION="4.2.1"
ARG CAPNP_VERSION="0.8.0"
ARG GFLAGS_VERSION="2.2.1"
ARG SPDLOG_VERSION="0.12.0"

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

# --- Build dependencies from source ---
WORKDIR /tmp

# libsodium
RUN wget https://download.libsodium.org/libsodium/releases/libsodium-${SODIUM_VERSION}.tar.gz && \
    tar -xf libsodium-${SODIUM_VERSION}.tar.gz && \
    cd libsodium-${SODIUM_VERSION} && \
    ./configure && make -j$(nproc) && make install && \
    cd .. && rm -rf libsodium*

# ZeroMQ (libzmq)
RUN wget https://github.com/zeromq/libzmq/releases/download/v${ZMQ_VERSION}/zeromq-${ZMQ_VERSION}.tar.gz && \
    tar -xf zeromq-${ZMQ_VERSION}.tar.gz && \
    cd zeromq-${ZMQ_VERSION} && \
    ./configure --with-libsodium && make -j$(nproc) && make install && \
    cd .. && rm -rf zeromq*

# cppzmq (header-only)
RUN wget https://github.com/zeromq/cppzmq/archive/v${CPPZMQ_VERSION}.tar.gz -O cppzmq.tar.gz && \
    tar -xf cppzmq.tar.gz && \
    cp cppzmq-${CPPZMQ_VERSION}/cppzmq.hpp /usr/local/include/ && \
    rm -rf cppzmq*

# Cap'n Proto
RUN wget https://capnproto.org/capnproto-c++-${CAPNP_VERSION}.tar.gz && \
    tar -xf capnproto-c++-${CAPNP_VERSION}.tar.gz && \
    cd capnproto-c++-${CAPNP_VERSION} && \
    ./configure && make -j$(nproc) && make install && \
    cd .. && rm -rf capnproto-c++*

# gflags
RUN wget https://github.com/gflags/gflags/archive/v${GFLAGS_VERSION}.tar.gz -O gflags.tar.gz && \
    tar -xf gflags.tar.gz && \
    cd gflags-${GFLAGS_VERSION} && mkdir build && cd build && \
    cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=/usr/local && make -j$(nproc) && make install && \
    cd ../.. && rm -rf gflags*

# spdlog
RUN wget https://github.com/gabime/spdlog/archive/v${SPDLOG_VERSION}.tar.gz -O spdlog.tar.gz && \
    tar -xf spdlog.tar.gz && \
    cd spdlog-${SPDLOG_VERSION} && mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && make -j$(nproc) && make install && \
    cd ../.. && rm -rf spdlog*

RUN ldconfig

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