# capnp_msgs

This component contains the shared [Cap'n Proto](https://capnproto.org/) message definitions used across the A17 project. It provides a central, version-controlled repository for all data structures that are serialized and sent over the network via `dispatch`.

The build system automatically generates C++ and Python sources from the `.capnp` schema files, making them available for use in other components.

## Prerequisites

This project is designed to be built within the provided Docker development environment, which handles all dependencies. Please see the top-level `README.md` for instructions on setting up the environment.

The key dependency for this module is **Cap'n Proto** (version `0.8.0` as defined in `install_dependencies.sh`).

## Build

This component is not intended to be built standalone. It is automatically built and installed as part of the main project build script.

From the `/workspace` directory inside the development container, run:

```bash
/workspace/build_project.sh
```

This script will:

1. Invoke the Cap'n Proto compiler (`capnpc`) to generate C++ header and source files from the `.capnp` schemas.
2. Compile the generated sources into a shared library (`libcapnp_msgs.so`).
3. Install the C++ headers, library, and the original `.capnp` schema files (for Python) into the `/workspace/install` directory.

## Defining New Messages

To add a new message definition to the project, follow these steps.

### 1. Generate a Unique File ID

Every `.capnp` file **must** begin with a unique 64-bit ID. This ID is used by Cap'n Proto to identify file schemas and prevent conflicts.

Generate a new ID by running the `capnp id` command:

```bash
# Run this inside the dev container or on a machine with capnp installed
capnp id
```

This will output a new, unique ID, for example: `@0xc315697a54491397;`

### 2. Create the `.capnp` File

Create a new file (e.g., `my_new_message.capnp`) in the `a17/capnp_msgs` directory.

### 3. Define the Message Structure

Paste the generated ID at the very top of the file. Then, define your data structures using the Cap'n Proto schema language. For a complete guide on the syntax, refer to the official Cap'n Proto Schema Language documentation.

**Example: `my_new_message.capnp`**

```capnp
@0xc315697a54491397; # Paste your unique ID here

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("a17::capnp_msgs");

struct MySensorData {
  timestamp @0 :UInt64;
  sensorName @1 :Text;
  readings @2 :List(Float64);
}
```

### 4. Re-run the Build

After adding your new file, simply re-run the main build script. CMake will automatically detect the new `.capnp` file and generate the corresponding C++ and Python code.

```bash
/workspace/build_project.sh
```

Your new message `MySensorData` will now be available to use in both C++ (as `a17::capnp_msgs::MySensorData`) and Python.
