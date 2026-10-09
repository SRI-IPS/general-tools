#!/bin/bash
#
# This script downloads and builds the C++ dependencies required by the project.
# It is designed to be run on a Debian-based system (like Ubuntu 20.04)
# where build-essential, cmake, wget, etc., are already installed.

set -e

# --- Installation Prefix ---
# The first argument to the script is the install prefix. Default to /usr/local if not provided.
INSTALL_PREFIX=${1:-/usr/local}
echo "--- Installing all dependencies to ${INSTALL_PREFIX} ---"

# --- Configuration ---
# Use environment variables if they are set, otherwise use these defaults.
SODIUM_VERSION=${SODIUM_VERSION:-"1.0.10"}
ZMQ_VERSION=${ZMQ_VERSION:-"4.2.2"}
CPPZMQ_VERSION=${CPPZMQ_VERSION:-"4.2.1"}
CAPNP_VERSION=${CAPNP_VERSION:-"0.8.0"}
GFLAGS_VERSION=${GFLAGS_VERSION:-"2.2.1"}
SPDLOG_VERSION=${SPDLOG_VERSION:-"0.12.0"}
GLOG_VERSION=${GLOG_VERSION:-"0.3.5"}
EIGEN_VERSION=${EIGEN_VERSION:-"3.4.0"}
AZMQ_VERSION=${AZMQ_VERSION:-"v1.0.3"}

NUM_JOBS=${NUM_JOBS:-"4"}

echo "--- Building dependencies with ${NUM_JOBS} parallel jobs ---"

# Create a temporary directory for downloads and builds
BUILD_DIR=$(mktemp -d) && trap 'rm -rf -- "$BUILD_DIR"' EXIT
cd "${BUILD_DIR}"

echo "Building libsodium v${SODIUM_VERSION}..."
wget "https://github.com/jedisct1/libsodium/archive/${SODIUM_VERSION}.tar.gz" -O "libsodium-${SODIUM_VERSION}.tar.gz"
tar -xf "libsodium-${SODIUM_VERSION}.tar.gz"
cd "libsodium-${SODIUM_VERSION}"
./configure --prefix="${INSTALL_PREFIX}" && make -j${NUM_JOBS} && make install
cd ..

echo "Building ZeroMQ (libzmq) v${ZMQ_VERSION}..."
wget "https://github.com/zeromq/libzmq/releases/download/v${ZMQ_VERSION}/zeromq-${ZMQ_VERSION}.tar.gz"
tar -xf "zeromq-${ZMQ_VERSION}.tar.gz"
cd "zeromq-${ZMQ_VERSION}"
# Use PKG_CONFIG_PATH to help find libsodium in our custom prefix
PKG_CONFIG_PATH="${INSTALL_PREFIX}/lib/pkgconfig" ./configure --prefix="${INSTALL_PREFIX}" --with-libsodium=yes && make -j${NUM_JOBS} && make install
cd ..

echo "Installing cppzmq v${CPPZMQ_VERSION}..."
wget "https://github.com/zeromq/cppzmq/archive/v${CPPZMQ_VERSION}.tar.gz" -O cppzmq.tar.gz
tar -xf cppzmq.tar.gz
cp "cppzmq-${CPPZMQ_VERSION}/cppzmq.hpp" "${INSTALL_PREFIX}/include/"

echo "Building Cap'n Proto v${CAPNP_VERSION}..."
wget "https://capnproto.org/capnproto-c++-${CAPNP_VERSION}.tar.gz"
tar -xf "capnproto-c++-${CAPNP_VERSION}.tar.gz"
cd "capnproto-c++-${CAPNP_VERSION}"
./configure --prefix="${INSTALL_PREFIX}" && make -j${NUM_JOBS} && make install
cd ..

echo "Building gflags v${GFLAGS_VERSION}..."
wget "https://github.com/gflags/gflags/archive/v${GFLAGS_VERSION}.tar.gz" -O gflags.tar.gz
tar -xf gflags.tar.gz
cd "gflags-${GFLAGS_VERSION}" && mkdir -p build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" && make -j${NUM_JOBS} && make install
cd ../..

echo "Building spdlog v${SPDLOG_VERSION}..."
wget "https://github.com/gabime/spdlog/archive/v${SPDLOG_VERSION}.tar.gz" -O spdlog.tar.gz
tar -xf spdlog.tar.gz
cd "spdlog-${SPDLOG_VERSION}" && mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" && make -j${NUM_JOBS} && make install
cd ../..

echo "Building glog v${GLOG_VERSION}..."
wget "https://github.com/google/glog/archive/v${GLOG_VERSION}.tar.gz" -O glog.tar.gz
tar -xf glog.tar.gz
cd "glog-${GLOG_VERSION}"
./configure --prefix="${INSTALL_PREFIX}" && make -j${NUM_JOBS} && make install
cd ..

# Eigen (header-only)
echo "Installing Eigen v${EIGEN_VERSION}..."
wget "https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.tar.gz"
tar -xf "eigen-${EIGEN_VERSION}.tar.gz"
cp -r "eigen-${EIGEN_VERSION}/Eigen" "${INSTALL_PREFIX}/include/"

# AZMQ (header-only)
echo "Installing AZMQ ${AZMQ_VERSION}..."
wget "https://github.com/zeromq/azmq/archive/refs/tags/${AZMQ_VERSION}.tar.gz" -O "azmq.tar.gz"
tar -xf "azmq.tar.gz"
# The directory name is azmq-1.0.3, so we strip the 'v' from the version tag.
cp -r "azmq-${AZMQ_VERSION#v}/azmq" "${INSTALL_PREFIX}/include/"

# Refresh shared library cache
ldconfig

echo "--- All dependencies installed successfully ---"