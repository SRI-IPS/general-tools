# A17 Tools - C++ and Python Libraries

This repository contains a collection of C++ and Python libraries designed for building robust, high-performance distributed systems. The core component is **`dispatch`**, a powerful messaging and service discovery middleware.

## Key Components

*   **`a17/dispatch`**: A custom middleware library for service-oriented communication.
    *   **Protocols**: Built on top of ZeroMQ for networking and Cap'n Proto for efficient data serialization.
    *   **Patterns**: Provides high-level abstractions for Publish/Subscribe and Request/Reply messaging patterns.
    *   **Service Discovery**: Features a decentralized service discovery mechanism using UDP multicast, allowing nodes to dynamically find each other on the network.
    *   **Languages**: Offers both C++ and Python APIs for seamless integration.
    *   **Execution Model**: Uses a high-performance, asynchronous event loop powered by Boost.Asio.

*   **`a17/utils`**: A collection of general-purpose C++ utilities, including a memory-pool buffer allocator used by `dispatch`.

*   **`a17/capnp_msgs`**: A set of common Cap'n Proto message definitions.

*   **`third_party`**: Contains dependencies and helper tools, including:
    *   Catch2 for C++ unit testing.
    *   A tool for generating a `compile_commands.json` database for use with clang-based editors.

## Build Systems

This project supports both **CMake** and **Bazel** for building the C++ components.

### Building with CMake

CMake is the primary supported build system. Each component contains its own `CMakeLists.txt` file.

```bash
# Example for building the dispatch library
cd a17/dispatch
mkdir build && cd build
cmake ..
make
```

See the `a17/dispatch/README.md` for more detailed instructions on managing dependencies.

### Building with Bazel

Bazel build files are also provided for integration into Bazel-based workflows.

```bash
# Build all targets
bazel build //...

# Run all tests
bazel test //...
```

## License

The code in the `a17` directory is licensed under the **BSD 3-Clause "New" or "Revised" License**. See the LICENSE file for the full text.

## Getting Started

We recommend starting with the `dispatch` library. Please see the detailed documentation in its directory:

*   **`a17/dispatch/README.md`**
