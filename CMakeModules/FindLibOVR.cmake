include(FindPackageHandleStandardArgs)

find_library(LibOVR_LIBOVRA libovr.a)
set(LibOVR_LIBRARIES ${LibOVR_LIBOVRA})
find_path(LibOVR_OVRH OVR.h)
set(LibOVR_INCLUDE_DIRS ${LibOVR_OVRH})

if(LibOVR_SOURCE)
find_package_handle_standard_args(LibOVR DEFAULT_MSG
    LibOVR_INCLUDE_DIRS
    LibOVR_SOURCE
)
else()
find_package_handle_standard_args(LibOVR DEFAULT_MSG
    LibOVR_LIBRARIES
    LibOVR_INCLUDE_DIRS
)
endif()

message("hello" ${LibOVR_LIBRARIES})
message("hello" ${LibOVR_INCLUDE_DIRS})

set(LibOVR_FOUND "1")
# mark_as_advanced( LibOVR_FOUND )

