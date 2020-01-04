cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 14)

project(third_party VERSION 0.1.0 LANGUAGES CXX)

# following may not be needed here
if(DEFINED ENV{A17_ROOT})
  include("$ENV{A17_ROOT}/cmake/A17DefaultPaths.cmake")
endif()

# Needed for MacOS compatibility. This allows the installed library to be linked against from
# outside the executable path of the linking project. Without this, dyld_library_paths must be
# manually set. See: https://cmake.org/Wiki/CMake_RPATH_handling
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)
# enable -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# allow duplicate names (only works for Makefile generators)
#set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS ON)

# build third_party
add_subdirectory( mav_trajectory_generation )
add_subdirectory( occmapping )
add_subdirectory( Catch )
add_subdirectory( ewok )
add_subdirectory( gohlke )
