cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(Gohlke VERSION 0.1.0 LANGUAGES CXX)

if(DEFINED ENV{A17_ROOT})
  include("$ENV{A17_ROOT}/cmake/A17DefaultPaths.cmake")
endif()

# ------------------------------------------------------------------------------
# Set vars
set(python_install_dir "py")
file(GLOB python_src_files "*.py")

# ------------------------------------------------------------------------------
# Install python sources
# NOTE: The trailing slash is important. Otherwise the files will be installed in a sub-directory
# with the name of the last path element (e.g. "cpp").
install(FILES ${python_src_files}
  DESTINATION "${python_install_dir}/third_party/gohlke")
