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
* **libsodium** (`1.0.10`)
* **ZeroMQ** (libzmq `4.2.2`) - Note: Requires **libsodium** for CURVE security features.
* **cppzmq** (`4.2.1`, header-only)
* **azmq** (`v1.0.3`, header-only)
* **Cap'n Proto** (`0.8.0`)
* **gflags** (`2.2.1`)
* **glog** (`0.3.5`)
* **spdlog** (`0.12.0`)
* **Eigen** (`3.4.0`, header-only)
* **Catch** (Included in `third_party` for tests)

The project provides scripts to install these dependencies from source (`install_dependencies.sh`) or from system packages (`install_apt_dependencies.sh`). For a reproducible build, using the source build script is recommended.

### Python Dependencies

The Python dependencies are listed in the `requirements.txt` file at the root of the repository. Key runtime dependencies include:

* `pycapnp` (`0.6.4`)
* `pyzmq`
* `tornado` (`6.1`)
* `netifaces` (`0.10.6`)
* `numpy` (`1.19.5`)

You can install these using pip:

```sh
pip install pycapnp pyzmq tornado netifaces
```

## Building

The project is designed to be built inside the provided Docker development environment. Please see the top-level `README.md` for instructions on setting up and entering the container.

All build commands below should be run from the `/workspace` directory inside the container.

### Building with CMake

This is the primary build method used by the project. The `build_project.sh` script orchestrates the build of all components in the correct order.

To build and test `dispatch` along with all its internal dependencies (`a17-utils`, `a17-capnp_msgs`, etc.), run the main build script from the workspace root:

```bash
/workspace/build_project.sh
```

This script will build all components and place the final libraries and headers into `/workspace/install`. It also runs the unit tests for each component, including `unittests_A17Dispatch`.

### Environment Setup

After a successful build using `build_project.sh`, the required libraries and Python modules will be in the `/workspace/install` directory. To run applications that use `dispatch`, you may need to update your environment variables:

```bash
export PYTHONPATH=/workspace/install/lib/python3.8/site-packages:$PYTHONPATH
export LD_LIBRARY_PATH=/workspace/install/lib:$LD_LIBRARY_PATH
```

### Building with Bazel

Bazel build files are also provided.
To build the library:

```sh
bazel build //a17/dispatch/... 
```

To run the tests:

```sh
bazel test //a17/dispatch/... 
```
