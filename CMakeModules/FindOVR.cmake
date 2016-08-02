include(FindPackageHandleStandardArgs)


if(WIN32)
	find_library(OVR_LIBRARIES LibOVR.lib)
	find_path(OVR_INCLUDE_DIR OVR_CAPI_GL.h)
	find_package_handle_standard_args(OVR DEFAULT_MSG OVR_LIBRARIES OVR_INCLUDE_DIR)

	set(OVR_INCLUDE_DIRS ${OVR_INCLUDE_DIR})

else()

# library was named libovr.a in 0.4.4 and libOVR.a in 0.5.0.1
find_library(OVR_LIBRARIES libOVR.a libovr.a)
find_library(OVR_LIBRARY_SO libOVRRT64_0.so.5)
find_path(OVR_INCLUDE_DIR_LIB OVR.h)
find_path(OVR_INCLUDE_DIR_SRC OVR_CAPI_GL.h)

# find_library(OVR_udev udev)
find_library(OVR_pthread pthread)
find_library(OVR_gl GL)
find_library(OVR_x11 X11)
find_library(OVR_xrandr Xrandr)
find_library(OVR_rt rt)


find_package_handle_standard_args(OVR DEFAULT_MSG
	OVR_LIBRARIES
	OVR_INCLUDE_DIR_LIB
	OVR_INCLUDE_DIR_SRC
	OVR_LIBRARY_SO
	OVR_pthread
	OVR_gl
	OVR_x11
	OVR_xrandr
	OVR_rt)

# TODO: Change PATH to DIRECTORY below when Rekhi lab uses a version of cmake newer than 2.8.11
get_filename_component(OVR_LIBRARY_DIR ${OVR_LIBRARY_SO} PATH)

# append the two include directories together.
set(OVR_INCLUDE_DIRS ${OVR_INCLUDE_DIR_LIB} ${OVR_INCLUDE_DIR_SRC})
set(OVR_LIBRARIES ${OVR_LIBRARIES} -Wl,-rpath=${OVR_LIBRARY_DIR} ${OVR_LIBRARY_SO} ${OVR_pthread} ${OVR_gl} ${OVR_x11} ${OVR_xrandr} ${OVR_rt})
endif()
