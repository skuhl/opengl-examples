include(FindPackageHandleStandardArgs)

find_library(OVR_LIBRARIES libovr.a)
find_path(OVR_INCLUDE_DIR_LIB OVR.h)
find_path(OVR_INCLUDE_DIR_SRC OVR_CAPI_GL.h)

find_library(OVR_udev udev)
find_library(OVR_pthread pthread)
find_library(OVR_gl GL)
find_library(OVR_x11 X11)
find_library(OVR_xrandr Xrandr)
find_library(OVR_rt rt)

find_package_handle_standard_args(OVR DEFAULT_MSG
	OVR_LIBRARIES
	OVR_INCLUDE_DIR_LIB
	OVR_INCLUDE_DIR_SRC
	OVR_udev
	OVR_pthread
	OVR_gl
	OVR_x11
	OVR_xrandr
	OVR_rt)

# append the two include directories together.
set(OVR_INCLUDE_DIRS ${OVR_INCLUDE_DIR_LIB} ${OVR_INCLUDE_DIR_SRC})
set(OVR_LIBRARIES ${OVR_LIBRARIES} ${OVR_udev} ${OVR_pthread} ${OVR_gl} ${OVR_x11} ${OVR_xrandr} ${OVR_rt})
