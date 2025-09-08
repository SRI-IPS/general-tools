if(NOT TARGET azmq)
  find_path(_AZMQ_INCLUDE_DIR
    NAME actor.hpp
    PATHS
        # Look in the source tree (corrected path)
        "$ENV{A17_ROOT}/pkg/linux/include/azmq"
        "/usr/local/include/azmq"
  )
  set(azmq_INCLUDE_DIRS ${_AZMQ_INCLUDE_DIR})
  add_library(azmq INTERFACE IMPORTED)
  set_target_properties(azmq PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${azmq_INCLUDE_DIRS}")
endif()
