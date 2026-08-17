# dispatch_nodes - Example Nodes

Simple talker and listener example nodes demonstrating the `dispatch` middleware's publish/subscribe pattern, similar to ROS `rostopic` examples.

## Overview

These examples demonstrate:

- Creating a `Node` and registering publishers/subscribers
- Using Cap'n Proto messages for serialization
- Automatic service discovery via UDP multicast
- Periodic publishing with `Repeater`

## Message Type

The examples use a simple `Chatter` message defined in `a17/capnp_msgs/dispatch_nodes/chatter.capnp`:

```capnp
struct Chatter {
  timestamp @0 :UInt64;
  sender    @1 :Text;
  message   @2 :Text;
}
```

## Running the Examples

### Python

**Terminal 1 - Start the talker:**

```bash
export PYTHONPATH=$A17_ROOT/install/py:$PYTHONPATH
python a17/dispatch_nodes/talker.py
```

**Terminal 2 - Start the listener:**

```bash
export PYTHONPATH=$A17_ROOT/install/py:$PYTHONPATH
python a17/dispatch_nodes/listener.py
```

### C++

**Build:**

```bash
cd a17/dispatch_nodes
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=$A17_ROOT/install
make
```

**Terminal 1 - Start the talker:**

```bash
export LD_LIBRARY_PATH=$A17_ROOT/install/lib:$LD_LIBRARY_PATH
./dispatch_talker
```

**Terminal 2 - Start the listener:**

```bash
export LD_LIBRARY_PATH=$A17_ROOT/install/lib:$LD_LIBRARY_PATH
./dispatch_listener
```

### Bazel

```bash
# Python
bazel run //a17/dispatch_nodes:talker_py
bazel run //a17/dispatch_nodes:listener_py

# C++
bazel run //a17/dispatch_nodes:talker
bazel run //a17/dispatch_nodes:listener
```

## Command-Line Options

Both C++ executables support:

- `--node-name NAME` - Override the node name (default: TALKER or LISTENER)
- `--device-name NAME` - Set device name prefix for topics
- `--help` - Display help

Python examples support the same via `absl.flags`:

- `--node_name=NAME`
- `--device_name=NAME`

## Topic Naming

Topics are automatically prefixed with `device_name/node_name/`. For example:

- `TALKER` node publishes on `TALKER/CHATTER` (no device name)
- With `--device-name=P3`, topic becomes `P3/TALKER/CHATTER`

## Service Discovery

Both examples use the default UDP multicast address (`224.0.88.1:8888`) for service discovery. The listener automatically discovers the talker's publisher topic without any manual configuration.

To use a custom multicast address:

```bash
export DISPATCH_PORT=8888
export DISPATCH_MULTICAST=224.0.88.1
```
