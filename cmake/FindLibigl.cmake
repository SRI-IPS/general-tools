# Find libigl, a simple c++ geometry processing library.

FIND_PATH(LIBIGL_INCLUDE_DIRS igl/igl_inline.h
	PATHS /usr/include
	      /usr/local/include
	      $ENV{A17_ROOT}/ext/libigl/include
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBIGL DEFAULT_MSG LIBIGL_INCLUDE_DIRS)

