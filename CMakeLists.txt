cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

#set(CMAKE_C_COMPILER "clang")
#set(CMAKE_CXX_COMPILER "clang++")

project("opengl-examples" C CXX)  # This project uses C and C++
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -g -Wno-unused-parameter -Wno-unused-function")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

####################################
# Edit the following areas to add or remove programs to compile
#
# If you add a new name here, there must be an .c file with the same
# name that contains a main() function.
####################################
# Programs that need ASSIMP
set(NEED_ASSIMP ogl3-oculus ogl3-assimp ogl3-slerp ogl3-explode)
# Programs that don't rely on libraries
set(NEED_NOTHING ogl3-triangle ogl3-triangle-color ogl3-triangle-shade ogl3-prerend ogl3-picker ogl3-teartest ogl3-texture) # ogl2-triangle ogl2-slideshow ogl2-texture


# use color messages
include(${CMAKE_SOURCE_DIR}/CMakeModules/colorMessage.cmake)


# Use glxinfo or glewinfo to determine OpenGL version if they are
# available. Fail if a high enough version of OpenGL is not supported.
find_file(GLXINFO glxinfo)
find_file(GLEWINFO glewinfo)
if(UNIX AND NOT APPLE AND (GLXINFO OR GLEWINFO))
	# Print out graphics card information
	execute_process(COMMAND ${GLXINFO} COMMAND grep "OpenGL renderer string" COMMAND cut -d ":" -f 2 OUTPUT_VARIABLE openglrenderingstring OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
	if(openglrenderingstring)
		message("OpenGL renderer -${openglrenderingstring}")
	endif()

	# Check if the supported OpenGL version is sufficient using either glxinfo or glewinfo commands
	execute_process(COMMAND ${GLXINFO} COMMAND grep "OpenGL core profile version string" COMMAND cut -d " " -f 6 OUTPUT_VARIABLE openglversion OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
	if(NOT openglversion AND GLEWINFO)
		execute_process(COMMAND ${GLEWINFO} COMMAND grep "OpenGL version " COMMAND cut -d " " -f 3 OUTPUT_VARIABLE openglversion OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
	endif()

	if(openglversion)
		message("OpenGL core profile version - ${openglversion}")
		if(openglversion VERSION_LESS "3.2")
			message(WARNING "This software expect OpenGL version 3.2 or higher; some or all programs may fail to run correctly.")
		endif()
	endif()
endif()

# Get custom Windows library installation paths
if (WIN32)
	include(Windows.cmake)
	
	# GLUT installation
	set(GLUT_ROOT_PATH "${WINDOWS_LIBRARY_PATH}${WINDOWS_GLUT_FOLDER}/" ${GLUT_ROOT_PATH})
	set(CMAKE_LIBRARY_PATH "${GLUT_ROOT_PATH}lib" ${CMAKE_LIBRARY_PATH})
	set(CMAKE_INCLUDE_PATH "${GLUT_ROOT_PATH}include" ${CMAKE_INCLUDE_PATH})

	# GLEW installation
	set(GLEW_ROOT_PATH "${WINDOWS_LIBRARY_PATH}${WINDOWS_GLEW_FOLDER}/")
	set(CMAKE_LIBRARY_PATH "${GLEW_ROOT_PATH}lib" ${CMAKE_LIBRARY_PATH})
	set(CMAKE_INCLUDE_PATH "${GLEW_ROOT_PATH}include" ${CMAKE_INCLUDE_PATH})

	# ASSIMP installation
	set(ASSIMP_ROOT_PATH "${WINDOWS_LIBRARY_PATH}${WINDOWS_ASSIMP_FOLDER}/")
	set(CMAKE_LIBRARY_PATH "${ASSIMP_ROOT_PATH}lib" ${CMAKE_LIBRARY_PATH})
	set(CMAKE_INCLUDE_PATH "${ASSIMP_ROOT_PATH}include" ${CMAKE_INCLUDE_PATH})

	# FreeType installation
	set(FREETYPE_ROOT_PATH "${WINDOWS_LIBRARY_PATH}${WINDOWS_FREETYPE_FOLDER}/")
	set(CMAKE_LIBRARY_PATH "${FREETYPE_ROOT_PATH}" ${CMAKE_LIBRARY_PATH})
	set(CMAKE_INCLUDE_PATH "${FREETYPE_ROOT_PATH}include" ${CMAKE_INCLUDE_PATH})
endif()

# SITE_NAME sets the HOSTNAME variable to the name of this computer.
SITE_NAME(HOSTNAME)

# Doxygen documentation
option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" ON)
if(BUILD_DOCUMENTATION)
	find_package(Doxygen)
	if(DOXYGEN_FOUND AND (NOT (${DOXYGEN_EXECUTABLE} MATCHES "DOXYGEN_EXECUTABLE-NOTFOUND")) AND EXISTS ${CMAKE_SOURCE_DIR}/Doxyfile)
		#-- Add a custom target to run Doxygen when ever the project is built
		add_custom_target (docs
		    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
		    SOURCES ${CMAKE_SOURCE_DIR}/Doxyfile)
		# Add or remove 'ALL' after 'docs' above to enable/disable doxygen generation when running "make"
		
		# Don't print warning if Doxygen doesn't work since many people don't use them.
	endif()
endif()

if (NOT WIN32)
	# === Libraries that we use ===
	# --- math library ---
	find_library(M_LIB m)
endif()

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
if(UNIX AND NOT APPLE)
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
	set(ASSIMP_FOUND_DEFINITION "KUHL_UTIL_USE_ASSIMP")
else()
	set(ASSIMP_FOUND_DEFINITION "")
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
set(CMAKE_THREADS_PREFER_PTHREAD TRUE)   # prefer pthread over other threading libraries
# set(THREADS_PREFER_PTHREAD_FLAG TRUE)   # prefer -pthread compiler flag over just using -lpthread, but it might not be supported by all compilers.
find_package(Threads)
# find_packge(Threads) seems to fail on CCSR, try to find it ourselves:
#if(NOT Threads_FOUND)
#find_library(CMAKE_THREAD_LIBS_INIT NAMES pthread PATHS "/lib64" "/lib" "/usr/lib" )
#message("Last attempt to find pthreads: " ${CMAKE_THREAD_LIBS_INIT})
#endif()

# --- LibOVR (Oculus Rift) ---
# Rekhi Linux lab
set(CMAKE_LIBRARY_PATH "/home/campus11/kuhl/public-cs4611/ovr_sdk_linux_0.4.4/LibOVR/Lib/Linux/Release/x86_64" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "/home/campus11/kuhl/public-cs4611/ovr_sdk_linux_0.4.4/LibOVR/Include" ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH "/home/campus11/kuhl/public-cs4611/ovr_sdk_linux_0.4.4/LibOVR/Src" ${CMAKE_INCLUDE_PATH})
# Installed in this directory's parent directory
set(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ovr_sdk_linux_0.4.4/LibOVR/Lib/Linux/Release/x86_64" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ovr_sdk_linux_0.4.4/LibOVR/Include" ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ovr_sdk_linux_0.4.4/LibOVR/Src" ${CMAKE_INCLUDE_PATH})
# An alternative directory name that OVR could be installed in (same style as: https://github.com/jherico/OculusSDK )
set(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../OculusSDK/LibOVR/Lib/Linux/Release/x86_64" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../OculusSDK/LibOVR/Include" ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../OculusSDK/LibOVR/Src" ${CMAKE_INCLUDE_PATH})

find_package(OVR)
if(OVR_FOUND)
	include_directories(${OVR_INCLUDE_DIRS})
	set(MISSING_OVR_DEFINITION "")
else()
	set(MISSING_OVR_DEFINITION "MISSING_OVR")
endif()


# Set the preprocessor flags.
set(PREPROC_DEFINE "MOUSEMOVE_GLUT;${ASSIMP_FOUND_DEFINITION};${MISSING_VRPN_DEFINITION};${MISSING_OVR_DEFINITION}")

# Construct a list of programs that we want to compile based on which libraries are available.
set(PROGRAMS_TO_MAKE ${NEED_NOTHING})
if(ASSIMP_FOUND)
	set(PROGRAMS_TO_MAKE ${PROGRAMS_TO_MAKE} ${NEED_ASSIMP})
else()
	message(WARNING "ASSIMP was not found, not compiling: ${NEED_ASSIMP}")
endif()

# Create a static library containing all files that we regularly use.
include_directories("lib")
set(FILES_IN_LIBKUHL lib/kuhl-util.c lib/vecmat.c lib/dgr.c lib/mousemove.c lib/hmd-dsight-orient.c lib/projmat.c lib/viewmat.c lib/vrpn-help.cpp)
add_library(kuhl STATIC ${FILES_IN_LIBKUHL})
set_target_properties(kuhl PROPERTIES COMPILE_DEFINITIONS "${PREPROC_DEFINE}")

# Compile the list of programs.
foreach(arg ${PROGRAMS_TO_MAKE})
	if(EXISTS ${arg}.cpp)   # Figure out if the program is a c or cpp file
		set(SOURCE_FILE ${arg}.cpp)
	else()
		set(SOURCE_FILE ${arg}.c)
	endif()
	add_executable(${arg} ${SOURCE_FILE})

	target_link_libraries(${arg} kuhl)
	if(VRPN_FOUND)  # Add VRPN to the list if it is available
		target_link_libraries(${arg} ${VRPN_LIBRARIES})
	endif()
	if(OVR_FOUND) # Add Oculus LibOVR to the list if it is available
		target_link_libraries(${arg} ${OVR_LIBRARIES})
	endif()
	if(ASSIMP_FOUND)
		target_link_libraries(${arg} ${ASSIMP_LIBRARIES})
	endif()

	target_link_libraries(${arg} ${GLEW_LIBRARIES} ${M_LIB} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} )

	set_target_properties(${arg} PROPERTIES LINKER_LANGUAGE "CXX")
	set_target_properties(${arg} PROPERTIES COMPILE_DEFINITIONS "${PREPROC_DEFINE}")
	add_dependencies(${arg} kuhl)
endforeach()


# Build dgr-relay
if(Threads_FOUND)
	# === dgr-relay ===
	add_executable(dgr-relay dgr-relay.cpp)
	target_link_libraries(dgr-relay ${CMAKE_THREAD_LIBS_INIT})
else()
	message(WARNING "Not compiling dgr-relay because pthreads was not found on this system.")
endif()
