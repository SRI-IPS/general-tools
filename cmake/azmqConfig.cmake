if(NOT TARGET azmq)
  set(azmq_INCLUDE_DIRS "$ENV{A17_ROOT}/pkg/linux/include")
  add_library(azmq INTERFACE IMPORTED)
  set_target_properties(azmq PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${azmq_INCLUDE_DIRS}")
endif()
