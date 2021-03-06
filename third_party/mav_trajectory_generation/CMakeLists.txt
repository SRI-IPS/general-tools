cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)

project(MavTrajectoryGeneration VERSION 0.3.5 LANGUAGES CXX)

if(DEFINED ENV{A17_ROOT})
  include("$ENV{A17_ROOT}/cmake/A17DefaultPaths.cmake")
endif()

# Added to correctly inform rpath of runtime location of shared library when linked against
# See for more info: https://cmake.org/Wiki/CMake_RPATH_handling
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

# ------------------------------------------------------------------------------
# Set vars
set(namespace "mav_trajectory_generation::")

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
configure_file("cmake/Deps.cmake.in" "${deps_config}" @ONLY)
include("${deps_config}")

# ------------------------------------------------------------------------------
# Build mav_trajectory_generation lib.
add_library(mav_trajectory_generation
  src/motion_defines.cpp
  src/polynomial.cpp
  src/rpoly.cpp
  src/segment.cpp
  src/timing.cpp
  src/trajectory.cpp
  src/vertex.cpp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries(mav_trajectory_generation
  eigen
  glog::glog)

# Install mav_trajectory_generation lib and headers.
install(TARGETS mav_trajectory_generation
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")

# NOTE: The trailing slash is important. Otherwise the files will be installed in a sub-directory
# with the name of the last path element (e.g. "cpp").
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/mav_trajectory_generation"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

# Add timing evaluation executable
add_executable(polynomial_timing_evaluation
  src/polynomial_timing_evaluation.cpp)

target_link_libraries(polynomial_timing_evaluation
  mav_trajectory_generation
  glog::glog)

install(TARGETS polynomial_timing_evaluation
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")

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
  "test/main.cpp"
  "test/test_polynomial.cpp"
  "test/test_polynomial_optimization.cpp")
target_include_directories(${TEST_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test>)
target_link_libraries(${TEST_NAME}
  catch
  mav_trajectory_generation)

# ------------------------------------------------------------------------------
# Unit tests
enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
