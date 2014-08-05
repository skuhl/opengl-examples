include(FindPackageHandleStandardArgs)

if(${CMAKE_HOST_UNIX})
    find_path(GLEW_INCLUDE_DIRS GL/glew.h)
    find_library(GLEW_LIBRARIES GLEW)
endif()

if(GLEW_SOURCE)
find_package_handle_standard_args(GLEW DEFAULT_MSG
    GLEW_INCLUDE_DIRS
    GLEW_SOURCE
)
else()
find_package_handle_standard_args(GLEW DEFAULT_MSG
    GLEW_LIBRARIES
    GLEW_INCLUDE_DIRS
)
endif()

mark_as_advanced( GLEW_FOUND )
