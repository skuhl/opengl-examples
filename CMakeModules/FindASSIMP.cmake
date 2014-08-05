include(FindPackageHandleStandardArgs)

FIND_LIBRARY(ASSIMP_LIBRARIES assimp)
FIND_PATH(ASSIMP_INCLUDE_DIRS assimp/mesh.h)


if(ASSIMP_SOURCE)
find_package_handle_standard_args(ASSIMP DEFAULT_MSG
    ASSIMP_INCLUDE_DIRS
    ASSIMP_SOURCE
)
else()
find_package_handle_standard_args(ASSIMP DEFAULT_MSG
    ASSIMP_LIBRARIES
    ASSIMP_INCLUDE_DIRS
)
endif()

mark_as_advanced( ASSIMP_FOUND )
