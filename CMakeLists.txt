cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

#set(CMAKE_C_COMPILER "clang")
#set(CMAKE_CXX_COMPILER "clang++")

project("opengl-examples" C CXX)  # This project uses C and C++
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -g -Wno-unused-parameter -Wno-unused-function")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)


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
	include(CMakeModules/Windows.cmake)
	
	# GLUT installation
	set(GLUT_ROOT_PATH "${WINDOWS_LIBRARY_PATH}${WINDOWS_GLUT_FOLDER}/" ${GLUT_ROOT_PATH})
	set(CMAKE_LIBRARY_PATH "${GLUT_ROOT_PATH}lib" ${CMAKE_LIBRARY_PATH})
	set(CMAKE_INCLUDE_PATH "${GLUT_ROOT_PATH}include" ${CMAKE_INCLUDE_PATH})

	# GLEW installation
	set(GLEW_ROOT_PATH "${WINDOWS_LIBRARY_PATH}${WINDOWS_GLEW_FOLDER}/")
	set(CMAKE_LIBRARY_PATH "${GLEW_ROOT_PATH}lib" ${CMAKE_LIBRARY_PATH})
	set(CMAKE_INCLUDE_PATH "${GLEW_ROOT_PATH}include" ${CMAKE_INCLUDE_PATH})

	# ZLIB installation
	set(ZLIB_ROOT_PATH "${WINDOWS_LIBRARY_PATH}${WINDOWS_ZLIB_FOLDER}/")
	set(CMAKE_LIBRARY_PATH "${ZLIB_ROOT_PATH}" ${CMAKE_LIBRARY_PATH})
	set(CMAKE_INCLUDE_PATH "${ZLIB_ROOT_PATH}" ${CMAKE_INCLUDE_PATH})
	
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
	# --- math library ---
	find_library(M_LIB m)
endif()

# --- OpenGL ---
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# --- Freeglut (version 2.6 or higher) ---
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
find_package(ImageMagick COMPONENTS MagickCore)
if(ImageMagick_FOUND)
	include_directories(${ImageMagick_INCLUDE_DIRS})
	set(IMAGEMAGICK_FOUND_DEFINITION "KUHL_UTIL_USE_IMAGEMAGICK")
else()
	set(IMAGEMAGICK_FOUND_DEFINITION "")
endif()


# --- ASSIMP (recommended, optional) ---
find_package(ASSIMP)
if(ASSIMP_FOUND)
	include_directories(${ASSIMP_INCLUDE_DIRS})
	set(ASSIMP_FOUND_DEFINITION "KUHL_UTIL_USE_ASSIMP")
else()
	set(ASSIMP_FOUND_DEFINITION "")
endif()


# --- VRPN ---
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
find_package(OVR)
if(OVR_FOUND)
	include_directories(${OVR_INCLUDE_DIRS})
	set(MISSING_OVR_DEFINITION "")
else()
	set(MISSING_OVR_DEFINITION "MISSING_OVR")
endif()


# Set the preprocessor flags.
set(PREPROC_DEFINE "MOUSEMOVE_GLUT;${ASSIMP_FOUND_DEFINITION};${MISSING_VRPN_DEFINITION};${MISSING_OVR_DEFINITION};${IMAGEMAGICK_FOUND_DEFINITION}")


# Look in lib folder for libraries and header files
include_directories("lib")

# Write programs out to "bin" directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Process cmake files in the subdirectories.
# build libkuhl.a
add_subdirectory("lib") 
# build sample programs
add_subdirectory("samples")
# build fake vrpn server
add_subdirectory("vrpn-fake")
# build DGR relay
add_subdirectory("dgr")



