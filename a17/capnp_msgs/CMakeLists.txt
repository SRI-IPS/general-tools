cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 14)

project(A17CapnpMsgs VERSION 0.1.0)

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

set(generated_capnp_dir "${generated_dir}/capnp_msgs")
set(generated_capnp_workspace_dir "${generated_capnp_dir}/a17/capnp_msgs")

# ------------------------------------------------------------------------------
# Import dependencies
configure_file("cmake/Deps.cmake.in" "${deps_config}" @ONLY)
include("${deps_config}")

#add_subdirectory("converters")

# ------------------------------------------------------------------------------
# Generate cpp sources
file(GLOB_RECURSE capnp_files "${CMAKE_CURRENT_SOURCE_DIR}/*.capnp")
set(capnp_cc_files "")
set(capnp_h_files "")
set(capnp_files_relsource "")
foreach(capnp_file ${capnp_files})
  file(RELATIVE_PATH capnp_file_relpath "${CMAKE_CURRENT_SOURCE_DIR}" "${capnp_file}")
  list(APPEND capnp_files_relsource "${capnp_file_relpath}")

  string(REGEX REPLACE "\.capnp$" "\.capnp\.c++" capnp_cc_file ${capnp_file_relpath})
  get_filename_component(capnp_cc_file "${capnp_cc_file}" REALPATH BASE_DIR "${generated_capnp_workspace_dir}")
  list(APPEND capnp_cc_files "${capnp_cc_file}")

  string(REGEX REPLACE "\.capnp$" "\.capnp\.h" capnp_h_file ${capnp_file_relpath})
  get_filename_component(capnp_h_file "${capnp_h_file}" REALPATH BASE_DIR "${generated_capnp_workspace_dir}")
  list(APPEND capnp_h_files "${capnp_h_file}")

  add_custom_command(
    OUTPUT
      ${capnp_h_file}
      ${capnp_cc_file}
    DEPENDS
      ${capnp_file}
    WORKING_DIRECTORY
      "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND
      mkdir -p "${generated_capnp_workspace_dir}" && "${CAPNP_EXECUTABLE}" compile
    ARGS
      "-o${CAPNPC_CPP}:${generated_capnp_workspace_dir}"
      "${capnp_file_relpath}"
  )
endforeach()

# ------------------------------------------------------------------------------
# Install capnp files to python path
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
  DESTINATION "${python_install_dir}/a17/capnp_msgs"
  FILES_MATCHING PATTERN "*.capnp")

# ------------------------------------------------------------------------------
# Install python sources
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
  DESTINATION "${python_install_dir}/a17/capnp_msgs"
  FILES_MATCHING PATTERN "*.py")

# ------------------------------------------------------------------------------
# capnmsgs lib
add_library(capnp_msgs SHARED
  ${capnp_cc_files})
target_include_directories(capnp_msgs PUBLIC
  $<BUILD_INTERFACE:${generated_capnp_dir}>
  $<INSTALL_INTERFACE:${include_install_dir}>)
target_link_libraries(capnp_msgs
  capnproto)
install(TARGETS capnp_msgs
  EXPORT "${targets_export_name}"
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  RUNTIME DESTINATION "bin")
install(DIRECTORY "${generated_capnp_dir}/"
        DESTINATION "${include_install_dir}"
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
