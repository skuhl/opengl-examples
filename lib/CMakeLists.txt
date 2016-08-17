if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    message(FATAL_ERROR "Don't run cmake here. Run it in the root folder of this repository.")
endif()
cmake_minimum_required(VERSION 2.6)


set(FILES_IN_LIBKUHL kuhl-util.c kuhl-nodep.c vecmat.c dgr.c mousemove.c viewmat.cpp vrpn-help.cpp kalman.c font-helper.c msg.c list.c queue.c tdl-util.c serial.c orient-sensor.c cfg_parse.c kuhl-config.c video.c bufferswap.c dispmode.cpp dispmode-desktop.cpp dispmode-frustum.cpp dispmode-hmd.cpp dispmode-anaglyph.cpp camcontrol.cpp camcontrol-mouse.cpp camcontrol-vrpn.cpp camcontrol-orientsensor.cpp sensorfuse.c)

# tack on the Oculus linux files if appropriate
if(OVR_FOUND AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(FILES_IN_LIBKUHL ${FILES_IN_LIBKUHL} camcontrol-oculus-linux.cpp dispmode-oculus-linux.cpp)
endif()

if(OVR_FOUND AND ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(FILES_IN_LIBKUHL ${FILES_IN_LIBKUHL} camcontrol-oculus-windows.cpp dispmode-oculus-windows.cpp)
endif()


if(ImageMagick_FOUND)
	set(FILES_IN_LIBKUHL ${FILES_IN_LIBKUHL} imageio.c)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(FILES_IN_LIBKUHL ${FILES_IN_LIBKUHL} windows-compat.c)
endif()

add_library(kuhl STATIC ${FILES_IN_LIBKUHL})
set_target_properties(kuhl PROPERTIES COMPILE_DEFINITIONS "${PREPROC_DEFINE}")

if(APPLE)
	# Some Mac OSX machines need this to ensure that freetype.h is found.
	target_include_directories(kuhl PUBLIC "/opt/X11/include/freetype2/")
endif()
