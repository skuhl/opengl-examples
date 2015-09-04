set(FILES_IN_LIBKUHL kuhl-util.c kuhl-nodep.c vecmat.c dgr.c mousemove.c hmd-dsight-orient.c projmat.c viewmat.c vrpn-help.cpp kalman.c font-helper.c msg.c list.c queue.c tdl-util.c)

if(ImageMagick_FOUND)
	set(FILES_IN_LIBKUHL ${FILES_IN_LIBKUHL} imageio.c)
endif()

add_library(kuhl STATIC ${FILES_IN_LIBKUHL})
set_target_properties(kuhl PROPERTIES COMPILE_DEFINITIONS "${PREPROC_DEFINE}")

if(APPLE)
	# Some Mac OSX machines need this to ensure that freeglut.h is found.
	target_include_directories(kuhl PUBLIC "/opt/X11/include/freetype2/")
endif()
