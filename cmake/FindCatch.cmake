# Find catch

find_path(_CATCH_INCLUDE_DIR catch.hpp 
	PATHS 
		$ENV{A17_ROOT}/src/third_party/Catch/single_include
		)

set(CATCH_INCLUDE_DIRS ${_CATCH_INCLUDE_DIR} ${_CATCH_INCLUDE_DIR}/../include/reporters)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CATCH DEFAULT_MSG CATCH_INCLUDE_DIRS)

if(CATCH_FOUND)
	set(FOUND TRUE)
endif()
