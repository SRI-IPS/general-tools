cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(A17DispatchTornadoBridge VERSION 0.1.0)

# This project only installs Python files.
# The destination path is relative to CMAKE_INSTALL_PREFIX.
set(python_install_dir "py")

# Install the specific python files instead of the whole directory
# to avoid installing unintended files (like this CMakeLists.txt).
install(FILES
    "__init__.py"
    "dispatch_tornado_bridge.py"
    DESTINATION "${python_install_dir}/a17/dispatch_tornado_bridge"
)
