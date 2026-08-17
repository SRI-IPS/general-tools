cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 14)

project(A17ChatterNode VERSION 0.1.0 LANGUAGES CXX)

if(DEFINED ENV{A17_ROOT})
  include("$ENV{A17_ROOT}/cmake/A17DefaultPaths.cmake")
endif()

# Needed for MacOS compatibility. This allows the installed library to be linked against from
# outside the executable path of the linking project. Without this, dyld_library_paths must be
# manually set. See: https://cmake.org/wiki/CMake_RPATH_handling
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

# ------------------------------------------------------------------------------
# Set vars
set(python_install_dir "py")

# NOTE: This will break if this file is moved from //a17/dispatch_nodes/chatter_node.
get_filename_component(workspace_root "../../.."
  REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# ------------------------------------------------------------------------------
# Import dependencies
find_package(A17Dispatch REQUIRED CONFIG)
find_package(A17CapnpMsgs REQUIRED CONFIG)

# ------------------------------------------------------------------------------
# dispatch_talker executable (C++)
add_executable(dispatch_talker
  "talker.cpp")
target_link_libraries(dispatch_talker
  a17::dispatch
  a17::capnp_msgs)
install(TARGETS dispatch_talker DESTINATION "bin")

# ------------------------------------------------------------------------------
# dispatch_listener executable (C++)
add_executable(dispatch_listener
  "listener.cpp")
target_link_libraries(dispatch_listener
  a17::dispatch
  a17::capnp_msgs)
install(TARGETS dispatch_listener DESTINATION "bin")

# ------------------------------------------------------------------------------
# Install Python sources
file(GLOB python_src_files "*.py")
install(FILES ${python_src_files}
  DESTINATION "${python_install_dir}/a17/dispatch_nodes/chatter_node")
