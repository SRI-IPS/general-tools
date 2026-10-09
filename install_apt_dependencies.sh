#!/bin/bash
#
# This is an EXPERIMENTAL script to install dependencies using apt-get.
# NOTE: This will install the versions available in the system's repositories
# (e.g., Ubuntu 20.04), which may NOT match the specific versions required
# for a fully reproducible build.
#
# For a reproducible build with specific versions, use install_dependencies.sh

set -e

echo "--- Installing dependencies using apt ---"

apt-get update && apt-get install -y \
    libsodium-dev \
    libzmq3-dev \
    capnproto \
    libcapnp-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libeigen3-dev \
    libspdlog-dev \


# AZMQ is header-only and not in apt, so we must download it manually.
echo "--- Installing AZMQ (header-only) from source ---"
cd /tmp
wget https://github.com/zeromq/azmq/archive/refs/tags/v1.0.3.tar.gz -O azmq.tar.gz
tar -xf azmq.tar.gz
cp -r azmq-1.0.3/azmq /usr/local/include/
rm -rf azmq.tar.gz azmq-1.0.3

echo "--- apt dependencies installed successfully ---"
echo "Warning: Installed versions may differ from project requirements."