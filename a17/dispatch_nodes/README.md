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
├── CMakeLists.txt   # CMake build
├── BUILD.bazel      # Bazel build
├── talker.py        # Python talker
├── listener.py      # Python listener
├── talker.cpp       # C++ talker
└── listener.cpp     # C++ listener
```

More complex nodes may also include:

```
├── main.cpp            # C++ entry point with CLI parsing
├── node.h / node.cpp   # C++ implementation class (inherits dispatch::Node)
├── dispatch_client.py  # Python client library
└── scripts/            # Start/stop scripts
```

## Running Examples

See the README.md inside each example directory for build and run instructions.

## Adding New Examples

1. Create a new subdirectory under `a17/dispatch_nodes/`
2. Add your Cap'n Proto messages to `a17/capnp_msgs/dispatch_nodes/` (if needed)
3. Follow the patterns in `chatter_node/` for build files
4. Update this README.md to list your new example
