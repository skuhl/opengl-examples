if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    message(FATAL_ERROR "Don't run cmake here. Run it in the root folder of this repository.")
endif()
cmake_minimum_required(VERSION 2.6)


set(FILES_IN_LIBKUHL kuhl-util.c kuhl-nodep.c vecmat.c dgr.c mousemove.c projmat.c viewmat.c vrpn-help.cpp kalman.c font-helper.c msg.c list.c queue.c tdl-util.c serial.c orient-sensor.c)

if(ImageMagick_FOUND)
	set(FILES_IN_LIBKUHL ${FILES_IN_LIBKUHL} imageio.c)
endif()

add_library(kuhl STATIC ${FILES_IN_LIBKUHL})
set_target_properties(kuhl PROPERTIES COMPILE_DEFINITIONS "${PREPROC_DEFINE}")

if(APPLE)
	# Some Mac OSX machines need this to ensure that freetype.h is found.
	target_include_directories(kuhl PUBLIC "/opt/X11/include/freetype2/")
endif()
