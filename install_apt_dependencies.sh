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
    libboost-all-dev \
    libzmq3-dev \
    libsodium-dev \
    capnproto \
    libcapnp-dev \
    libgflags-dev \
    libspdlog-dev

echo "--- apt dependencies installed successfully ---"
echo "Warning: Installed versions may differ from project requirements."