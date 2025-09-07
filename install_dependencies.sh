#!/bin/bash
#
# This script downloads and builds the C++ dependencies required by the project.
# It is designed to be run on a Debian-based system (like Ubuntu 20.04)
# where build-essential, cmake, wget, etc., are already installed.

set -e

# --- Configuration ---
# Use environment variables if they are set, otherwise use these defaults.
SODIUM_VERSION=${SODIUM_VERSION:-"1.0.10"}
ZMQ_VERSION=${ZMQ_VERSION:-"4.2.2"}
CPPZMQ_VERSION=${CPPZMQ_VERSION:-"4.2.1"}
CAPNP_VERSION=${CAPNP_VERSION:-"0.8.0"}
GFLAGS_VERSION=${GFLAGS_VERSION:-"2.2.1"}
SPDLOG_VERSION=${SPDLOG_VERSION:-"0.12.0"}

NUM_JOBS=${NUM_JOBS:-"4"}

echo "--- Building dependencies with ${NUM_JOBS} parallel jobs ---"

# Create a temporary directory for downloads and builds
BUILD_DIR=$(mktemp -d)
cd "${BUILD_DIR}"

echo "Building libsodium v${SODIUM_VERSION}..."
wget "https://github.com/jedisct1/libsodium/archive/${SODIUM_VERSION}.tar.gz" -O "libsodium-${SODIUM_VERSION}.tar.gz"
tar -xf "libsodium-${SODIUM_VERSION}.tar.gz"
cd "libsodium-${SODIUM_VERSION}"
./configure && make -j${NUM_JOBS} && make install
cd ..

echo "Building ZeroMQ (libzmq) v${ZMQ_VERSION}..."
wget "https://github.com/zeromq/libzmq/releases/download/v${ZMQ_VERSION}/zeromq-${ZMQ_VERSION}.tar.gz"
tar -xf "zeromq-${ZMQ_VERSION}.tar.gz"
cd "zeromq-${ZMQ_VERSION}"
./configure --with-libsodium && make -j${NUM_JOBS} && make install
cd ..

echo "Installing cppzmq v${CPPZMQ_VERSION}..."
wget "https://github.com/zeromq/cppzmq/archive/v${CPPZMQ_VERSION}.tar.gz" -O cppzmq.tar.gz
tar -xf cppzmq.tar.gz
cp "cppzmq-${CPPZMQ_VERSION}/cppzmq.hpp" /usr/local/include/

echo "Building Cap'n Proto v${CAPNP_VERSION}..."
wget "https://capnproto.org/capnproto-c++-${CAPNP_VERSION}.tar.gz"
tar -xf "capnproto-c++-${CAPNP_VERSION}.tar.gz"
cd "capnproto-c++-${CAPNP_VERSION}"
./configure && make -j${NUM_JOBS} && make install
cd ..

echo "Building gflags v${GFLAGS_VERSION}..."
wget "https://github.com/gflags/gflags/archive/v${GFLAGS_VERSION}.tar.gz" -O gflags.tar.gz
tar -xf gflags.tar.gz
cd "gflags-${GFLAGS_VERSION}" && mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=/usr/local && make -j${NUM_JOBS} && make install
cd ../..

echo "Building spdlog v${SPDLOG_VERSION}..."
wget "https://github.com/gabime/spdlog/archive/v${SPDLOG_VERSION}.tar.gz" -O spdlog.tar.gz
tar -xf spdlog.tar.gz
cd "spdlog-${SPDLOG_VERSION}" && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && make -j${NUM_JOBS} && make install
cd ../..

# Clean up the temporary build directory
rm -rf "${BUILD_DIR}"

# Refresh shared library cache
ldconfig

echo "--- All dependencies installed successfully ---"