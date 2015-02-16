include(FindPackageHandleStandardArgs)

find_library(LibOVR_LIBRARIES ovr)
set(LibOVR_LIBRARIES ${LibOVR_LIBOVRA})

find_path(LibOVR_INCLUDE OVR.h)
set(LibOVR_INCLUDE_DIRS ${LibOVR_INCLUDE})

find_path(LibOVR_SRC OVR_CAPI.h)
set(LibOVR_INCLUDE_DIRS ${LibOVR_INCLUDE_DIRS} ${LibOVR_SRC})

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
