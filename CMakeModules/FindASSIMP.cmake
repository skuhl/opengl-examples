include(FindPackageHandleStandardArgs)

# Search paths for CCSR
set(CMAKE_LIBRARY_PATH /home/kuhl/public-ogl/assimp/lib ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /home/kuhl/public-ogl/assimp/include ${CMAKE_INCLUDE_PATH})
# Search paths for IVS
set(CMAKE_LIBRARY_PATH /research/kuhl/public-ogl/assimp/lib ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /research/kuhl/public-ogl/assimp/include ${CMAKE_INCLUDE_PATH})
# Search path for Rekhi lab
set(CMAKE_LIBRARY_PATH /home/campus11/kuhl/public-ogl/assimp/lib ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /home/campus11/kuhl/public-ogl/assimp/include ${CMAKE_INCLUDE_PATH})


# The assimp library is linked to zlib
FIND_PACKAGE(ZLIB)
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
    ZLIB_LIBRARIES
)
endif()

