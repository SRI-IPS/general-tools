if(DEFINED ENV{A17_ROOT})
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{A17_ROOT}/deploy" CACHE PATH "default install path" FORCE)
  endif()
  list(APPEND CMAKE_MODULE_PATH
    "$ENV{A17_ROOT}/cmake" "$ENV{A17_ROOT}/pkg/linux" "$ENV{A17_ROOT}/deploy")
  list(APPEND CMAKE_PREFIX_PATH
    "$ENV{A17_ROOT}/cmake" "$ENV{A17_ROOT}/pkg/linux" "$ENV{A17_ROOT}/deploy")
  link_directories("$ENV{A17_ROOT}/pkg/linux/lib")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()
