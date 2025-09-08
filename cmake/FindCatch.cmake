# Find catch

# This script finds the Catch (v1) testing framework.
# It searches in the project's source tree and in any install prefixes.

find_path(_CATCH_INCLUDE_DIR
    NAMES catch.hpp
    PATHS
        # Look in the source tree (corrected path)
        "$ENV{A17_ROOT}/third_party/Catch/single_include"
    # Look in standard install locations (e.g., /workspace/install/include)
    PATH_SUFFIXES
        include
)

set(CATCH_INCLUDE_DIRS ${_CATCH_INCLUDE_DIR})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CATCH DEFAULT_MSG CATCH_INCLUDE_DIRS)

if(CATCH_FOUND)
    add_library(Catch::Catch INTERFACE IMPORTED)
    set_target_properties(Catch::Catch PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CATCH_INCLUDE_DIRS}")
endif()
