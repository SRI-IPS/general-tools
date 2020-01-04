# find precompiled capnp messages

# assumes capnp_msgs root is at the same directory level as dispatch root, could alternatively install capnp_msgs into a location # that is specified by an env var e.g, $ENV(X)/include/
find_path(PRE_CAPNPMSGS_INCLUDE_DIR capnp_msgs/common.capnp.h PATHS $ENV{A17_ROOT}/deploy/include)
find_library(PRE_CAPNPMSGS_LIBRARY NAMES capnpmsgs PATHS $ENV{A17_ROOT}/deploy/lib)

find_package(capnproto REQUIRED)

set(CAPNPMSGS_LIBRARIES ${PRE_CAPNPMSGS_LIBRARY} ${CAPNPROTO_LIBRARIES})
set(CAPNPMSGS_INCLUDE_DIRS ${PRE_CAPNPMSGS_INCLUDE_DIR} ${CAPNPROTO_INCLUDE_DIR})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CAPNPMSGS DEFAULT_MSG CAPNPMSGS_INCLUDE_DIRS CAPNPMSGS_LIBRARIES)

if(CAPNPMSGS_FOUND)
	set(FOUND TRUE)
endif()
