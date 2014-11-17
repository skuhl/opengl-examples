cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

project("opengl-examples" C CXX)  # This project uses C and C++
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -g -Wno-unused-parameter -Wno-unused-function")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)


# SITE_NAME sets the HOSTNAME variable to the name of this computer.
SITE_NAME(HOSTNAME)


# Doxygen documentation
option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" ON)
if(BUILD_DOCUMENTATION)
	find_package(Doxygen)
	if(DOXYGEN_FOUND)
	#-- Add a custom target to run Doxygen when ever the project is built
	add_custom_target (docs 
                     COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
                     SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
	# Add or remove 'ALL' after 'docs' above to enable/disable doxygen generation when running "make"
	else()
		message("Not creating Doxygen documents because doxygen was not found on this system.")
	endif()
endif()


# === Libraries that we use ===
# --- math library ---
find_library(M_LIB m)

# --- OpenGL ---
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# --- Freeglut (version 2.6 or higher) ---
# IVS has freeglut 2.4 installed, and 2.8 at the location below:
set(CMAKE_LIBRARY_PATH /research/kuhl/public-vrlab/glut/src/.libs ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /research/kuhl/public-vrlab/glut/include/ ${CMAKE_INCLUDE_PATH})
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})

# Make sure freeglut installation is new enough!
include(CheckSymbolExists)
if(NOT APPLE)
  CHECK_SYMBOL_EXISTS(GLUT_CORE_PROFILE "${GLUT_INCLUDE_DIR}/GL/freeglut.h;${GLUT_INCLUDE_DIR}/GL/freeglut_ext.h" FREEGLUT_NEW_ENOUGH)
  if(NOT FREEGLUT_NEW_ENOUGH)
    message(FATAL_ERROR "freeglut version 2.6 or higher is required.")
  endif()
endif()

# --- GLEW ---
# Search paths for CCSR machine
set(CMAKE_LIBRARY_PATH "/usr/local/glew/1.9.0/lib" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "/usr/local/glew/1.9.0/include" ${CMAKE_INCLUDE_PATH})

# Search paths for IVS machine - NOTE: The copy of GLEW in /opt/viz is too old---haven't investigated exactly which version is needed.
set(CMAKE_LIBRARY_PATH "/research/kuhl/public-vrlab/glew/lib" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "/research/kuhl/public-vrlab/glew/include" ${CMAKE_INCLUDE_PATH})

# Search paths for Rekhi Lab machines
set(CMAKE_LIBRARY_PATH "/home/campus11/kuhl/public-cs4611/glew/lib" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "/home/campus11/kuhl/public-cs4611/glew/include" ${CMAKE_INCLUDE_PATH})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# Check for an newer extension that we use for glGetProgram(). If GLEW
# is too old, it might not know about the extension and we could get
# compile errors.
include(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(GL_ARB_get_program_binary "${GLEW_INCLUDE_DIRS}/GL/glew.h" GLEW_NEW_ENOUGH)
if(NOT GLEW_NEW_ENOUGH)
  message(FATAL_ERROR "glew version 1.5.6 or higher is required.")
endif()

# --- ImageMagick (recommended, optional) ---
# ImageMagick 6.7.5-5 is recommended for colorspaces to be handled correctly.

# IVS - installed version is too old.
set(CMAKE_LIBRARY_PATH "/research/kuhl/public-vrlab/ImageMagick-6.8.9-5/magick/.libs/" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "/research/kuhl/public-vrlab/ImageMagick-6.8.9-5/" ${CMAKE_INCLUDE_PATH})
# CCSR - installed version is too old.
set(CMAKE_LIBRARY_PATH "/home/kuhl/public-vrlab/ImageMagick-6.8.9-6/magick/.libs/" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "/home/kuhl/public-vrlab/ImageMagick-6.8.9-6/" ${CMAKE_INCLUDE_PATH})

find_package(ImageMagick COMPONENTS MagickCore)


if(ImageMagick_FOUND)
	include_directories(${ImageMagick_INCLUDE_DIRS})
endif()


# --- ASSIMP (recommended, optional) ---
# Search paths for CCSR
set(CMAKE_LIBRARY_PATH /home/kuhl/public-vrlab/assimp/lib ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /home/kuhl/public-vrlab/assimp/include ${CMAKE_INCLUDE_PATH})
# Search paths for IVS
set(CMAKE_LIBRARY_PATH /research/kuhl/public-vrlab/assimp/lib ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /research/kuhl/public-vrlab/assimp/include ${CMAKE_INCLUDE_PATH})
# Search path for Rekhi lab
set(CMAKE_LIBRARY_PATH /home/campus11/kuhl/public-cs4611/assimp/lib ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /home/campus11/kuhl/public-cs4611/assimp/include ${CMAKE_INCLUDE_PATH})


find_package(ASSIMP)
if(ASSIMP_FOUND)
	include_directories(${ASSIMP_INCLUDE_DIRS})
endif()



# --- VRPN ---
# Search paths for CCSR
set(CMAKE_LIBRARY_PATH /home/kuhl/public-vrlab/vrpn/build ${CMAKE_LIBRARY_PATH})
set(CMAKE_LIBRARY_PATH /home/kuhl/public-vrlab/vrpn/build/quat ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /home/kuhl/public-vrlab/vrpn/ ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH /home/kuhl/public-vrlab/vrpn/quat ${CMAKE_INCLUDE_PATH})
# Search paths for IVS
set(CMAKE_LIBRARY_PATH /research/kuhl/public-vrlab/vrpn/build ${CMAKE_LIBRARY_PATH})
set(CMAKE_LIBRARY_PATH /research/kuhl/public-vrlab/vrpn/build/quat ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /research/kuhl/public-vrlab/vrpn/ ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH /research/kuhl/public-vrlab/vrpn/quat ${CMAKE_INCLUDE_PATH})
# Search path for Rekhi lab
set(CMAKE_LIBRARY_PATH /home/campus11/kuhl/public-cs4611/vrpn/build ${CMAKE_LIBRARY_PATH})
set(CMAKE_LIBRARY_PATH /home/campus11/kuhl/public-cs4611/vrpn/build/quat ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /home/campus11/kuhl/public-cs4611/vrpn/ ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH /home/campus11/kuhl/public-cs4611/vrpn/quat ${CMAKE_INCLUDE_PATH})

find_package(VRPN)
if(VRPN_FOUND)
	include_directories(${VRPN_INCLUDE_DIRS})
	set(MISSING_VRPN_DEFINITION "")
else()
	set(MISSING_VRPN_DEFINITION "MISSING_VRPN")
endif()




# --- pthreads (required for DGR) ---
find_package(Threads)
# find_packge(Threads) seems to fail on CCSR, try to find it ourselves: 
#if(NOT Threads_FOUND)
#find_library(CMAKE_THREAD_LIBS_INIT NAMES pthread PATHS "/lib64" "/lib" "/usr/lib" )
#message("Last attempt to find pthreads: " ${CMAKE_THREAD_LIBS_INIT})
#endif()

# Collect libraries that all programs use together into one cmake variable
set(COMMON_LIBRARIES ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES}  ${M_LIB})
if(VRPN_FOUND)
	set(COMMON_LIBRARIES ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES}  ${VRPN_LIBRARIES} ${M_LIB})
endif()


# Programs that need ASSIMP and ImageMagick
set(NEED_ASSIMP_AND_IM  ogl3-assimp ogl3-slerp)
# Programs that only need ImageMagick
set(NEED_IM ogl2-slideshow ogl2-texture ogl3-texture)
# Programs that don't rely on libraries
set(NEED_NOTHING ogl2-triangle ogl3-triangle ogl3-triangle-color ogl3-triangle-shade ogl3-prerend ogl3-picker)

# make compilation rule for all programs that need ASSIMP and ImageMagick
if(ASSIMP_FOUND AND ImageMagick_FOUND)
	foreach(arg ${NEED_ASSIMP_AND_IM})
		add_executable(${arg} ${arg}.c  kuhl-util.c dgr.c mousemove.c projmat.c viewmat.c vrpn-help.cpp imageio.c)
		target_link_libraries(${arg} ${COMMON_LIBRARIES} ${GLUT_LIBRARIES} ${ASSIMP_LIBRARIES} ${ImageMagick_LIBRARIES})
		set_target_properties(${arg} PROPERTIES COMPILE_DEFINITIONS "KUHL_UTIL_USE_ASSIMP;KUHL_UTIL_USE_IMAGEMAGICK;MOUSEMOVE_GLUT;${MISSING_VRPN_DEFINITION}")
	endforeach()
else()
	if(NOT ASSIMP_FOUND)
		message("ASSIMP library not found, not compiling: ${NEED_ASSIMP_AND_IM}")
	endif()
	if(NOT ImageMagick_FOUND)
		message("ImageMagick library not found, not compiling: ${NEED_ASSIMP_AND_IM}")	
	endif()
endif()

# Make compilatoin rule for all programs that only need ImageMagick
if(ImageMagick_FOUND)
	foreach(arg ${NEED_IM})
		add_executable(${arg} ${arg}.c kuhl-util.c dgr.c mousemove.c projmat.c viewmat.c vrpn-help.cpp imageio.c)
		target_link_libraries(${arg} ${COMMON_LIBRARIES} ${GLUT_LIBRARIES} ${ImageMagick_LIBRARIES})
		set_target_properties(${arg} PROPERTIES COMPILE_DEFINITIONS "KUHL_UTIL_USE_IMAGEMAGICK;MOUSEMOVE_GLUT;${MISSING_VRPN_DEFINITION}")
	endforeach()
else()
	message("ImageMagick library not found, not compiling: ${NEED_IM}")
endif()

foreach(arg ${NEED_NOTHING})
	add_executable(${arg} ${arg}.c kuhl-util.c dgr.c mousemove.c projmat.c viewmat.c vrpn-help.cpp)
	target_link_libraries(${arg} ${COMMON_LIBRARIES} ${GLUT_LIBRARIES})
	set_target_properties(${arg} PROPERTIES COMPILE_DEFINITIONS "MOUSEMOVE_GLUT;${MISSING_VRPN_DEFINITION}")
endforeach()

if(Threads_FOUND)
	# === dgr-relay ===
	add_executable(dgr-relay dgr-relay.cpp)
	target_link_libraries(dgr-relay ${CMAKE_THREAD_LIBS_INIT})
else()
	message("Not compiling dgr-relay because pthreads was not found on this system.")
endif()
