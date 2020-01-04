cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 14)

project(OccMapping VERSION 1.0.0 LANGUAGES CXX)

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
set(namespace "occmapping::")

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
# occmapping lib
add_library(occmapping
  "src/probabilitylookup.cpp"
  "src/robustoctree.cpp"
  "src/robustoctreenode.cpp")
target_include_directories(occmapping PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
  $<INSTALL_INTERFACE:${include_install_dir}>)
target_link_libraries(occmapping PUBLIC
  occmappingoctomap
  opencv3)
install(TARGETS occmapping
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")
# NOTE: The trailing slash is important. Otherwise the files will be installed in a sub-directory
# with the name of the last path element (e.g. "cpp").
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/inc/"
  DESTINATION "${include_install_dir}/occmapping"
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
