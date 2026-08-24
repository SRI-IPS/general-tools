# dispatch_nodes - Example Nodes

Example nodes demonstrating the `dispatch` middleware. Each subdirectory contains a self-contained example node.

## Examples

| Directory | Description |
|-----------|-------------|
| `chatter_node/` | Simple talker/listener pub/sub example (like ROS `rostopic`) |

## Structure

Simple nodes (like `chatter_node/`) contain:

```
chatter_node/
├── CMakeLists.txt          # CMake build
├── BUILD.bazel             # Bazel build
├── unittests_main.cpp      # Catch test runner entry point
├── chatter_node_test.cpp   # Unit tests (Catch framework)
├── talker.py               # Python talker
├── listener.py             # Python listener
├── talker.cpp              # C++ talker
└── listener.cpp            # C++ listener
```

More complex nodes may also include:

```
├── main.cpp            # C++ entry point with CLI parsing
├── node.h / node.cpp   # C++ implementation class (inherits dispatch::Node)
├── dispatch_client.py  # Python client library
└── scripts/            # Start/stop scripts
```

## Testing

Each example node includes unit tests using the [Catch](https://github.com/catchorg/Catch2) framework, following the same pattern as `a17/dispatch`. The CI (`build_project.sh`) builds each example and runs its `unittests_*` binary.

## Running Examples

See the README.md inside each example directory for build and run instructions.

## Adding New Examples

1. Create a new subdirectory under `a17/dispatch_nodes/`
2. Add your Cap'n Proto messages to `a17/capnp_msgs/dispatch_nodes/` (if needed)
3. Follow the patterns in `chatter_node/` for build files
4. Update this README.md to list your new example
