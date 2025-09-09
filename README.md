# A17 Tools - C++ and Python Libraries

[![C++ and Python CI](https://github.com/SRI-IPS/general-tools/actions/workflows/ci.yml/badge.svg)](https://github.com/SRI-IPS/general-tools/actions/workflows/ci.yml)

This repository contains a collection of C++ and Python libraries designed for building robust, high-performance distributed systems. The core component is **`dispatch`**, a powerful messaging and service discovery middleware.

## Key Components

* **`a17/dispatch`**: A custom middleware library for service-oriented communication.
  * **Protocols**: Built on top of ZeroMQ for networking and Cap'n Proto for efficient data serialization.
  * **Patterns**: Provides high-level abstractions for Publish/Subscribe and Request/Reply messaging patterns.
  * **Service Discovery**: Features a decentralized service discovery mechanism using UDP multicast, allowing nodes to dynamically find each other on the network.
  * **Languages**: Offers both C++ and Python APIs for seamless integration.
  * **Execution Model**: Uses a high-performance, asynchronous event loop powered by Boost.Asio.

* **`a17/utils`**: A collection of general-purpose C++ utilities, including a memory-pool buffer allocator used by `dispatch`.

* **`a17/capnp_msgs`**: A set of common Cap'n Proto message definitions.

* **`third_party`**: Contains dependencies and helper tools, including:
  * Catch for C++ unit testing.
  * A tool for generating a `compile_commands.json` database for use with clang-based editors.

## Build Systems

This project can be built natively on Linux/macOS or within a provided Docker container.

### Development with Docker (Recommended)

For a consistent and hassle-free development environment, we recommend using Docker. This avoids the need to manually install dependencies on your host machine.

**Prerequisites:**

* [Docker](https://docs.docker.com/get-docker/)

* **Linux Post-installation:** To run Docker commands without `sudo`, you must add your user to the `docker` group. This is a one-time setup.

```bash
sudo usermod -aG docker $USER
```

After running this command, you must **log out and log back in** for the new group membership to be applied. For more details, see the official Docker documentation.

**Usage:**

1. Make the helper script executable:

    ```bash
    chmod +x docker-dev.sh
    ```

2. Run the script to start the development container:

    ```bash
    ./docker-dev.sh
    ```

This will first build the Docker image (a one-time process) and then launch a bash shell inside the container. Your project directory is mounted at `/workspace`.

Inside the container, you can now run the CMake or Bazel builds as you normally would. For example:

```bash
# To run the CMake build for dispatch
cd a17/dispatch && mkdir build && cd build
cmake .. && make && ./unittests_A17Dispatch
```

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

### Automated Build & Test (for CI)

If you want to run all builds and tests in one non-interactive command, use the `run-ci.sh` script. This is ideal for continuous integration or for quickly verifying the project state.

1. Make the script executable:

    ```bash
    chmod +x run-ci.sh
    ```

2. Run the script:

    ```bash
    ./run-ci.sh
    ```

This will execute the CMake and Bazel builds and run all associated unit tests inside the Docker container.

## License

The code in the `a17` directory is licensed under the **BSD 3-Clause "New" or "Revised" License**. See the LICENSE file for the full text.

## Getting Started

We recommend starting with the `dispatch` library. Please see the detailed documentation in its directory:

* **`a17/dispatch/README.md`**
