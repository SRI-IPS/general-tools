# Originally based on this example:
# https://github.com/IvanSafonov/grpc-cmake-example

# Locate and configure the Google Protocol Buffers library
# Adds the following targets:
#  protobuf::libprotobuf - Protobuf library
#  protobuf::libprotobuf-lite - Protobuf lite library
#  protobuf::libprotoc - Protobuf Protoc Library
#  protobuf::protoc - protoc executable

include(CMakeParseArguments)

# Generates C++ sources from .proto files
# args:
#  SRCS - variable to define with autogenerated source files
#  HDRS - variable to define with autogenerated header files
#  WORKSPACE_ROOT - sources will be created in subfolders relative to this directory
#                   defaults to the current source directory
#  INSTALL_DIR - directory where the source files will be created
#                defaults to the current binary directory
#  IMPORT_DIRS - additional directories that will be searched for imported proto files
#  PROTOS - .proto files
function(protobuf_generate_cpp)
  set(_options)
  set(_one_value_args SRCS HDRS WORKSPACE_ROOT INSTALL_DIR)
  set(_multi_value_args IMPORT_DIRS PROTOS)
  cmake_parse_arguments(_PGC "${options}" "${_one_value_args}" "${_multi_value_args}" ${ARGN})

  if(NOT _PGC_SRCS)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without SRCS")
    return()
  endif()

  if(NOT _PGC_HDRS)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without HDRS")
    return()
  endif()

  if(NOT _PGC_WORKSPACE_ROOT)
    set(_PGC_WORKSPACE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  if(NOT _PGC_INSTALL_DIR)
    set(_PGC_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  endif()

  if(NOT _PGC_PROTOS)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without PROTOS")
    return()
  endif()

  list(APPEND _protobuf_include_args -I ${_PGC_WORKSPACE_ROOT})

  foreach(IMPORT_DIR ${_PGC_IMPORT_DIRS})
    get_filename_component(ABS_IMPORT_DIR ${IMPORT_DIR} ABSOLUTE)
    list(FIND _protobuf_include_args ${ABS_IMPORT_DIR} _contains_already)
    if(${_contains_already} EQUAL -1)
      list(APPEND _protobuf_include_args -I ${ABS_IMPORT_DIR})
    endif()
  endforeach()

  file(MAKE_DIRECTORY "${_PGC_INSTALL_DIR}")

  set(${_PGC_SRCS})
  set(${_PGC_HDRS})
  foreach(FIL ${_PGC_PROTOS})
    file(RELATIVE_PATH REL_FIL ${_PGC_WORKSPACE_ROOT} ${FIL})
    get_filename_component(REL_PATH ${REL_FIL} DIRECTORY)
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    set(CPP_OUT_DIR "${_PGC_INSTALL_DIR}/${REL_PATH}")

    set(SRC_FIL "${CPP_OUT_DIR}/${FIL_WE}.pb.cc")
    set(HDR_FIL "${CPP_OUT_DIR}/${FIL_WE}.pb.h")

    list(APPEND ${_PGC_SRCS} ${SRC_FIL})
    list(APPEND ${_PGC_HDRS} ${HDR_FIL})

    add_custom_command(
      OUTPUT ${SRC_FIL} ${HDR_FIL}
      COMMAND protobuf::protoc
      ARGS --cpp_out ${_PGC_INSTALL_DIR} ${_protobuf_include_args} ${ABS_FIL}
      DEPENDS ${ABS_FIL} protobuf::protoc
      COMMENT "Running C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${_PGC_SRCS}} ${${_PGC_HDRS}} PROPERTIES GENERATED TRUE)
  set(${_PGC_SRCS} ${${_PGC_SRCS}} PARENT_SCOPE)
  set(${_PGC_HDRS} ${${_PGC_HDRS}} PARENT_SCOPE)
endfunction()

# Find the include directory
find_path(PROTOBUF_INCLUDE_DIR google/protobuf/service.h)
mark_as_advanced(PROTOBUF_INCLUDE_DIR)

# The Protobuf library
if(NOT TARGET protobuf::libprotobuf)
  find_library(PROTOBUF_LIBRARY NAMES protobuf)
  mark_as_advanced(PROTOBUF_LIBRARY)
  add_library(protobuf::libprotobuf UNKNOWN IMPORTED)
  set_target_properties(protobuf::libprotobuf PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES pthread
    IMPORTED_LOCATION ${PROTOBUF_LIBRARY})
endif()

# The Protobuf lite library
if(NOT TARGET protobuf::libprotobuf-lite)
  find_library(PROTOBUF_LITE_LIBRARY NAMES protobuf-lite)
  mark_as_advanced(PROTOBUF_LITE_LIBRARY)
  add_library(protobuf::libprotobuf-lite UNKNOWN IMPORTED)
  set_target_properties(protobuf::libprotobuf-lite PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES pthread
    IMPORTED_LOCATION ${PROTOBUF_LITE_LIBRARY})
endif()

# The Protobuf Protoc Library
if(NOT TARGET protobuf::libprotoc)
  find_library(PROTOBUF_PROTOC_LIBRARY NAMES protoc)
  mark_as_advanced(PROTOBUF_PROTOC_LIBRARY)
  add_library(protobuf::libprotoc UNKNOWN IMPORTED)
  set_target_properties(protobuf::libprotoc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES protobuf::libprotobuf
    IMPORTED_LOCATION ${PROTOBUF_PROTOC_LIBRARY})
endif()

# Find the protoc Executable
if(NOT TARGET protobuf::protoc)
  find_program(PROTOBUF_PROTOC_EXECUTABLE NAMES protoc)
  mark_as_advanced(PROTOBUF_PROTOC_EXECUTABLE)
  add_executable(protobuf::protoc IMPORTED)
  set_target_properties(protobuf::protoc PROPERTIES
    IMPORTED_LOCATION ${PROTOBUF_PROTOC_EXECUTABLE})
endif()

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Protobuf DEFAULT_MSG
  PROTOBUF_LIBRARY PROTOBUF_INCLUDE_DIR PROTOBUF_PROTOC_EXECUTABLE)
