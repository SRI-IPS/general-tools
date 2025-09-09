# dispatch

`dispatch` is a C++ and Python middleware library for building distributed systems. It simplifies network programming by providing high-level abstractions for common messaging patterns over ZeroMQ and Cap'n Proto.

## Features

* **Simple API**: A `Node` object acts as the main entry point for creating publishers, subscribers, clients, and servers.
* **Service Discovery**: A built-in `Directory` uses UDP multicast to allow nodes to automatically discover each other's services (topics) on the network.
* **Efficient Serialization**: Uses Cap'n Proto for fast, zero-copy serialization of messages.
* **Asynchronous Core**: Built on Boost.Asio for high-performance, non-blocking I/O.
* **Build System**: Primarily supports **CMake**. (Bazel files are present but the main build script `build_project.sh` uses CMake).
* **C++ and Python**: Provides APIs for both languages.

## Prerequisites

### C++ Dependencies

You will need the following libraries installed:

* **Boost** (System dependency, requires components: `system`, `regex`, `asio`)
* **libsodium**
* **ZeroMQ** (libzmq) - Note: Requires **libsodium** for CURVE security features.
* **cppzmq** (Header-only ZeroMQ C++ bindings)
* **azmq** (Header-only Boost.Asio integration for ZeroMQ)
* **Cap'n Proto** (capnp, capnpc)
* **gflags**
* **glog**
* **spdlog**
* **Eigen** (Header-only)
* **Catch** (Included in `third_party` for unit tests)

The project provides two scripts for handling these prerequisites inside the development environment:

1. **`install_dependencies.sh` (Recommended for Reproducibility)**: This script downloads and builds specific, tested versions of each library from source. This is the method used for CI and guarantees a consistent build. The versions are: `libsodium-1.0.10`, `zeromq-4.2.2`, `capnproto-0.8.0`, `gflags-2.2.1`, `glog-0.3.5`, `spdlog-0.12.0`, `eigen-3.4.0`.

2. **`install_apt_dependencies.sh` (For System Integration)**: This script installs the versions available in the standard Ubuntu 20.04 `apt` repositories. These versions may be newer or older than the source-built ones but are generally compatible. This method is faster but less reproducible.

### Python Dependencies

All Python dependencies for the project are listed in the `requirements.txt` file at the root of the repository. Key runtime dependencies for `dispatch` include:

* `pycapnp`
* `pyzmq`
* `tornado`
* `netifaces`
* `numpy`

You can install all required Python packages using `pip`. From the `/workspace` directory inside the development container, run:

    pip install -r $A17_ROOT/requirements.txt

## Building

The project is designed to be built inside the provided Docker development environment. Please see the top-level `README.md` for instructions on setting up and entering the container.

All build commands below should be run from the project root (`/workspace` or `$A17_ROOT`) inside the container.

### Building with CMake

This is the primary build method. The `build_project.sh` script orchestrates the build of all components in the correct order.

To build and test `dispatch` along with all its internal dependencies (`a17-utils`, `a17-capnp_msgs`, etc.), run the main build script from the workspace root:

    $A17_ROOT/build_project.sh

This script will build all components and place the final libraries and headers into `$A17_ROOT/install`. It also runs the unit tests for each component, including `unittests_A17Dispatch`.

### Environment Setup

After a successful build using `build_project.sh`, the required libraries and Python modules will be in the `$A17_ROOT/install` directory. To run applications that use `dispatch`, you may need to update your environment variables:

    export PYTHONPATH=$A17_ROOT/install/py:$PYTHONPATH
    export LD_LIBRARY_PATH=$A17_ROOT/install/lib:$LD_LIBRARY_PATH

### Building with Bazel

Bazel build files are also provided.
To build the library:

    bazel build //a17/dispatch/... 

To run the tests:

    bazel test //a17/dispatch/... 
