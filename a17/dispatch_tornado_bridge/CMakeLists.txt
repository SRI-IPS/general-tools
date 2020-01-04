cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(A17DispatchTornadoBridge VERSION 0.1.0 LANGUAGES CXX)

if(DEFINED ENV{A17_ROOT})
  include("$ENV{A17_ROOT}/cmake/A17DefaultPaths.cmake")
endif()

set(python_install_dir "py")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
  DESTINATION "${python_install_dir}/a17/dispatch_tornado_bridge"
  FILES_MATCHING PATTERN "*.py"
  PATTERN "__pycache__" EXCLUDE
  PATTERN "build" EXCLUDE)
