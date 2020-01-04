cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 14)

project(A17Maths VERSION 0.1.0 LANGUAGES CXX)

if(DEFINED ENV{A17_ROOT})
  include("$ENV{A17_ROOT}/cmake/A17DefaultPaths.cmake")
endif()

# Needed for MacOS compatibility. This allows the installed library to be linked against from
# outside the executable path of the linking project. Without this, dyld_library_paths must be
# manually set. See: https://cmake.org/Wiki/CMake_RPATH_handling
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

# ------------------------------------------------------------------------------
# Set vars
set(namespace "a17::")

set(python_install_dir "py")
set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(deps_config_install_file "${PROJECT_NAME}Deps.cmake")
set(deps_config "${generated_dir}/${deps_config_install_file}")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(targets_export_install_file "${targets_export_name}.cmake")

# NOTE: This will break if this file is moved from //a17/maths.
get_filename_component(workspace_root "../.."
  REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB python_src_files "*.py")

# ------------------------------------------------------------------------------
# Import dependencies
configure_file("cmake/Deps.cmake.in" "${deps_config}" @ONLY)
include("${deps_config}")

# ------------------------------------------------------------------------------
# maths lib
add_library(maths INTERFACE)
target_include_directories(maths INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${include_install_dir}>)
target_link_libraries(maths INTERFACE
  eigen3)
install(TARGETS maths
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")

# ------------------------------------------------------------------------------
# maths_constants lib
add_library(maths_constants
  "constants.cpp")
target_include_directories(maths_constants INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${include_install_dir}>)
install(TARGETS maths_constants
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")

# ------------------------------------------------------------------------------
# matrix_utils lib
add_library(matrix_utils INTERFACE)
target_include_directories(matrix_utils INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${include_install_dir}>)
target_link_libraries(matrix_utils INTERFACE
  eigen3)
install(TARGETS matrix_utils
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")

# ------------------------------------------------------------------------------
# angle_utils lib
add_library(angle_utils
  "angle_unroller.cpp"
  "angle_utils.cpp")
target_link_libraries(angle_utils INTERFACE
  maths_constants)
target_include_directories(angle_utils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${include_install_dir}>)
install(TARGETS angle_utils
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")

# ------------------------------------------------------------------------------
# Install python sources
install(FILES ${python_src_files}
  DESTINATION "${python_install_dir}/a17/maths")

# ------------------------------------------------------------------------------
# unittests executable
set(TEST_NAME unittests_${PROJECT_NAME})
add_executable(${TEST_NAME}
  "angle_unroller_test.cpp"
  "angle_utils_test.cpp"
  "quaternion_utils_test.cpp"
  "matrix_utils_test.cpp"
  "unittests_main.cpp")
target_include_directories(${TEST_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${workspace_root}>)
target_link_libraries(${TEST_NAME}
  angle_utils
  catch
  eigen3)

# ------------------------------------------------------------------------------
# Unit tests
enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

# ------------------------------------------------------------------------------
# Install headers
# NOTE: The trailing slash is important. Otherwise the files will be installed in a sub-directory
# with the name of the last path element (e.g. "cpp").
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
  DESTINATION "${include_install_dir}/a17/maths"
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
