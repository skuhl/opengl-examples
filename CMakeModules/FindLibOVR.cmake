include(FindPackageHandleStandardArgs)

find_library(LibOVR_LIBRARIES libovr.a)
find_path(LibOVR_INCLUDE_DIRS OVR.h OVR_CAPI.h)

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
