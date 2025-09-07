cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(A17Utils VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# enable -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

# ------------------------------------------------------------------------------
# Set vars
set(namespace "a17::")

set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(deps_config_install_file "${PROJECT_NAME}Deps.cmake")
set(deps_config "${generated_dir}/${deps_config_install_file}")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(targets_export_install_file "${targets_export_name}.cmake")

# ------------------------------------------------------------------------------
# Import dependencies
#configure_file("cmake/Deps.cmake.in" "${deps_config}" @ONLY)
#include("${deps_config}")
find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)
find_package(Catch REQUIRED)
find_package(Threads REQUIRED)

# ------------------------------------------------------------------------------
# utils lib
set(util_sources
  "buffer_pool.cpp"
  "character_device.cpp"
  "nearest_interval_bin.cpp"
  "pid.cpp"
  "rate_measure.cpp"
  "repeater.cpp"
  "serial_port.cpp"
  "spdlog.cpp"
  "udp_socket.cpp"
  "watchdog.cpp")

add_library(utils ${util_sources})
target_include_directories(utils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${include_install_dir}>)
target_link_libraries(utils PUBLIC
  Boost::system
  spdlog::spdlog)
install(TARGETS utils
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")
# NOTE: The trailing slash is important. Otherwise the files will be installed in a sub-directory
# with the name of the last path element (e.g. "cpp").
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
  DESTINATION "${include_install_dir}/a17/utils"
  FILES_MATCHING PATTERN "*.h")

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

# ------------------------------------------------------------------------------
# ${TEST_NAME} executable
set(TEST_NAME unittests_${PROJECT_NAME})
add_executable(${TEST_NAME}
  "buffer_pool_test.cpp"
  "pid_test.cpp"
  "rate_measure_test.cpp"
  "unittests_main.cpp"
  "watchdog_test.cpp")
target_include_directories(${TEST_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>) # This seems to not exist, but leaving for compatibility
target_link_libraries(${TEST_NAME} PRIVATE
  utils
  catch
  Threads::Threads)

# ------------------------------------------------------------------------------
# Unit tests
include(CTest)
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
