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

if(OTHER_LIBRARIES_DIR)
# Windows
set(CMAKE_LIBRARY_PATH ${OTHER_LIBRARIES_DIR}/assimp-3.2/lib/Debug)
set(CMAKE_INCLUDE_PATH ${OTHER_LIBRARIES_DIR}/assimp-3.2/include)
endif()


FIND_LIBRARY(ASSIMP_LIBRARIES NAMES assimp assimp-vc130-mtd)
FIND_PATH(ASSIMP_INCLUDE_DIRS assimp/mesh.h)

# We don't need to find zlib if we are on windows...
if(WIN32)
find_package_handle_standard_args(ASSIMP DEFAULT_MSG
    ASSIMP_LIBRARIES
    ASSIMP_INCLUDE_DIRS
)
else()
FIND_PACKAGE(ZLIB)
find_package_handle_standard_args(ASSIMP DEFAULT_MSG
    ASSIMP_LIBRARIES
    ASSIMP_INCLUDE_DIRS
   ZLIB_LIBRARIES
)
endif()

