# dispatch

`dispatch` is a C++ and Python middleware library for building distributed systems. It simplifies network programming by providing high-level abstractions for common messaging patterns over ZeroMQ and Cap'n Proto.

## Features

* **Simple API**: A `Node` object acts as the main entry point for creating publishers, subscribers, clients, and servers.
* **Service Discovery**: A built-in `Directory` uses UDP multicast to allow nodes to automatically discover each other's services (topics) on the network.
* **Efficient Serialization**: Uses Cap'n Proto for fast, zero-copy serialization of messages.
* **Asynchronous Core**: Built on Boost.Asio for high-performance, non-blocking I/O.
* **Dual Build Systems**: Supports both **CMake** and **Bazel**.
* **C++ and Python**: Provides APIs for both languages.

## Prerequisites

### C++ Dependencies

You will need the following libraries installed:

* **Boost** (>= 1.65.1, components: `system`, `regex`)
* **ZeroMQ** (libzmq >= 4.2.5)
* **cppzmq** (Header-only ZeroMQ C++ bindings)
* **azmq** (Header-only Boost.Asio integration for ZeroMQ)
* **Cap'n Proto** (capnp, capnpc >= 0.6.1)
* **spdlog** (>= 1.3.1, for logging)
* **gflags** (>= 2.2.2, for command-line flag parsing)
* **Catch** (for running tests)

We recommend using a package manager like vcpkg, Conan, or your system's package manager (e.g., `apt`, `brew`) to install them.

### Python Dependencies

* `pycapnp`
* `pyzmq`
* `tornado`
* `netifaces`

You can install these using pip:

```sh
pip install pycapnp pyzmq tornado netifaces
```

## Building

First, clone the repository recursively to get the necessary submodules:

```sh
git clone --recursive <repository-url>
```

### Building with CMake

This is the recommended way to build the C++ library and run its tests.

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make install
```

To run the unit tests:

```bash
cd build
./unittests_A17Dispatch
```

### Environment Setup (Linux/macOS)

After building, you may need to set your `PYTHONPATH` and `LD_LIBRARY_PATH` to run applications that use the `dispatch` library from your build directory.

First, set an environment variable pointing to your workspace root (e.g., the directory containing the `a17` folder):

```bash
export A17_ROOT=/path/to/your/workspace
```

Then, update your library and Python paths. This example assumes you have also built other `a17` components like `capnp_msgs`.

```bash
export PYTHONPATH=$A17_ROOT/a17/capnp_msgs/build/py:$A17_ROOT/a17/dispatch/build/py:$PYTHONPATH
export LD_LIBRARY_PATH=$A17_ROOT/a17/capnp_msgs/build/lib:$A17_ROOT/a17/dispatch/build/lib:$LD_LIBRARY_PATH
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
