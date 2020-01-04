# Find hokoyu lidar urg c library

find_path(_urg_library_INCLUDE Urg_driver.h PATHS $ENV{A17_ROOT}/pkg/linux/include/urg_cpp)
find_program(_urg_library_BINARY urg_cpp-config PATHS $ENV{A17_ROOT}/pkg/linux/bin)
find_library(_urg_library_LIBRARY NAMES urg_cpp PATHS $ENV{A17_ROOT}/pkg/linux/lib/)

set(urg_library_INCLUDE_DIRS ${_urg_library_INCLUDE})
set(urg_library_LIBRARIES ${_urg_library_LIBRARY})
set(urg_library_LIBRARY_DIRS $ENV{A17_ROOT}/pkg/linux/lib)
set(urg_library_BINARY_DIRS ${_urg_library_BINARY})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(urg_library DEFAULT_MSG urg_library_BINARY_DIRS urg_library_INCLUDE_DIRS urg_library_LIBRARY_DIRS urg_library_LIBRARIES)

if(urg_library_FOUND)
  set(FOUND TRUE)
endif()
