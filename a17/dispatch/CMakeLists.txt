cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(A17Dispatch VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the custom cmake module path for finding dependencies.
# For backward compatibility, we first check for the A17_ROOT environment variable.
if(DEFINED ENV{A17_ROOT})
  # include("$ENV{A17_ROOT}/cmake/A17DefaultPaths.cmake")
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{A17_ROOT}/cmake")
else()
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../../cmake")
endif()

# Needed for MacOS compatibility. This allows the installed library to be linked against from
# outside the executable path of the linking project. Without this, dyld_library_paths must be
# manually set. See: https://cmake.org/Wiki/CMake_RPATH_handling
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)
# enable -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------------------
# Set vars
set(namespace "a17::")

set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(python_install_dir "py")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(deps_config_install_file "${PROJECT_NAME}Deps.cmake")
set(deps_config "${generated_dir}/${deps_config_install_file}")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(targets_export_install_file "${targets_export_name}.cmake")

# ------------------------------------------------------------------------------
# Import dependencies
##configure_file("cmake/Deps.cmake.in" "${deps_config}" @ONLY)
##include("${deps_config}")
find_package(A17Utils REQUIRED) # Local project dependency
find_package(AZMQ REQUIRED) # Header-only
find_package(Boost REQUIRED COMPONENTS regex system)
find_package(CapnProto REQUIRED)
find_package(Gflags REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(Threads REQUIRED)

# For testing
find_package(Catch REQUIRED)
find_package(A17CapnpMsgs REQUIRED)

# ------------------------------------------------------------------------------
# Build dispatch lib
add_library(dispatch SHARED
  "address.cpp"
  "client.cpp"
  "defs.cpp"
  "directory.cpp"
  "directory_topic.cpp"
  "listener.cpp"
  "message_helpers.cpp"
  "node.cpp"
  "reply_server.cpp"
  "request_client.cpp"
  "server.cpp"
  "smart_capnp_builder.cpp"
  "smart_capnp_reader.cpp"
  "smart_message_reader.cpp"
  "socket.cpp"
  "socket_types.cpp"
  "subscriber.cpp"
  "sub_server.cpp"
  "topic.cpp")
target_include_directories(dispatch PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(dispatch PUBLIC
  a17::utils       # from A17Utils
  azmq::azmq       # from AZMQ
  Boost::regex
  Boost::system
  CapnProto::capnp
  gflags::gflags
  spdlog::spdlog
  Threads::Threads
  ZeroMQ::libzmq)
install(TARGETS dispatch
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
  DESTINATION "${include_install_dir}/a17/dispatch"
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

# ------------------------------------------------------------------------------
# Install python sources
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
  DESTINATION "${python_install_dir}/a17/dispatch"
  FILES_MATCHING PATTERN "*.py")

# ------------------------------------------------------------------------------
# ${TEST_NAME} exe
set(TEST_NAME unittests_${PROJECT_NAME})
add_executable(${TEST_NAME}
  "unittests_main.cpp"
  "address_test.cpp"
  "messages_test.cpp"
  "directory_test.cpp"
  "socket_test.cpp"
  "topic_map_test.cpp"
  "topic_test.cpp"
)
target_link_libraries(${TEST_NAME} PRIVATE
  dispatch
  a17::capnp_msgs
  catch)

include(CTest)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

# ------------------------------------------------------------------------------
# Version config
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${version_config}"
  COMPATIBILITY SameMajorVersion)
install(FILES "${version_config}"
  DESTINATION "${config_install_dir}")

# ------------------------------------------------------------------------------
# Deps config
# The deps_config file was already generated above.
install(FILES "${deps_config}"
  DESTINATION "${config_install_dir}")

# ------------------------------------------------------------------------------
# Package config
configure_package_config_file("cmake/Config.cmake.in"
  "${project_config}"
  INSTALL_DESTINATION "${config_install_dir}")
install(FILES "${project_config}"
  DESTINATION "${config_install_dir}")

# ------------------------------------------------------------------------------
# Targets config
install(EXPORT "${targets_export_name}"
  NAMESPACE "${namespace}"
  DESTINATION "${config_install_dir}")
