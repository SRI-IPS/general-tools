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

* **Boost** (>= 1.65, components: `system`, `regex`)
* **ZeroMQ** (libzmq)
* **cppzmq** (Header-only ZeroMQ C++ bindings)
* **azmq** (Header-only Boost.Asio integration for ZeroMQ)
* **Cap'n Proto** (capnp, capnpc)
* **spdlog** (for logging)
* **gflags** (for command-line flag parsing)
* **Catch2** (for running tests)

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

```sh
./unittests_A17Dispatch
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
