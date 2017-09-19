cmake_minimum_required(VERSION 2.6)

#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")

# Use homebrew gcc if it is installed.
#if(APPLE AND EXISTS /usr/local/bin/gcc-5)
#set(CMAKE_C_COMPILER "/usr/local/bin/gcc-5")
#set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-5")
#endif()


#set(CMAKE_C_COMPILER "clang")
#set(CMAKE_CXX_COMPILER "clang++")

project("opengl-examples" C CXX)  # This project uses C and C++

if(WIN32)
	set(OTHER_LIBRARIES_DIR "C:/Users/kuhl/Desktop/opengl-libs")
endif()


if(NOT WIN32)
# Add "-pg" to to both the C_FLAGS and the CXX_FLAGS to enable profiling
# -O2 or -O3 is necessary to get the compiler to inline the inlined functions used by vecmat.c and vecmat.h
# -march=native will cause the compiler to use features that are available on the current CPU (SSE, etc)---but will make the resulting executable less portable.
#   -march=native is removed because it doesn't work on IVS as of May 2015
# -pedantic forces warnings when non-C99 extensions are used.

# Options which we may add in future (not supported by older compilers):
# -Wshadow should probably be added eventually
# -Wvla should be included because Visual Studio does not support VLAs, so we avoid using them. They are optional in C11 even though they were part of C99.
set (COMMON_FLAGS " -Wall -Wextra -pedantic -g -Wformat -Wno-unused-parameter -Wno-unused-function") # used for C and C++
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -std=gnu99 -Wno-deprecated-declarations")

# Use no-variadic-macros to prevent C++ programs from warning about the variadic msg() macro
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=c++0x -Wno-variadic-macros")
endif() # end if not windows


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

# Write programs out to "bin" directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Include rpath
# SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# use color messages
include(${CMAKE_SOURCE_DIR}/CMakeModules/colorMessage.cmake)


function(kuhl_add_library_path path)
	message(STATUS "Looking for: ${path}")
	file(GLOB POSSIBLE_PATHS ${path})
	foreach(possible_path ${POSSIBLE_PATHS})
		if(EXISTS ${possible_path})
			message(STATUS "      Found: ${possible_path}")
			set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${possible_path} CACHE INTERNAL "")
		endif()
	endforeach()
endfunction(kuhl_add_library_path)

function(kuhl_add_include_path path)
	message(STATUS "Looking for: ${path}")
	file(GLOB POSSIBLE_PATHS ${path})
	foreach(possible_path ${POSSIBLE_PATHS})
		if(EXISTS ${possible_path})
			message(STATUS "      Found: ${possible_path}")
			set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${possible_path} CACHE INTERNAL "")
		endif()
	endforeach()
endfunction(kuhl_add_include_path)

function(kuhl_add_paths path)
	# On the MTU lab machines, /local/kuhl-public-share/opengl has
	# listing disabled. This apparently causes cmake to treat the
	# directory as if it does not exist. I'm commenting this out until
	# we make the folder listable or until cmake works as we expect.
	#
	#if(NOT EXISTS ${path})
	#	return()
	#endif()

	# GLFW
	if(NOT GLFW_glfw_LIBRARY)
		kuhl_add_library_path(${path}/glfw/src)
		kuhl_add_include_path(${path}/glfw/include)
		if(WIN32) # GLFW - Windows
			kuhl_add_include_path(${path}/glfw-*/include)
			kuhl_add_library_path(${path}/glfw-*/src/Debug)   # compiled from source
			kuhl_add_library_path(${path}/glfw-*/src/Release) # compiled from source
			kuhl_add_library_path(${path}/glfw-*/lib-vc2015)  # precompiled
			kuhl_add_library_path(${path}/glfw-*/lib-vc2013)  # precompiled
			kuhl_add_library_path(${path}/glfw-*/lib-vc2012)  # precompiled
		endif()
	endif()
	
	# GLEW
	if(NOT GLEW_LIBRARY)
		kuhl_add_library_path(${path}/glew/lib)
		kuhl_add_include_path(${path}/glew/include)
		if(WIN32) # GLEW - Windows
			kuhl_add_library_path(${path}/glew-*/lib/Release/Win32)
			kuhl_add_include_path(${path}/glew-*/include)
		endif()
	endif()

	# Assimp
	if(NOT ASSIMP_LIBRARIES)
		kuhl_add_library_path(${path}/assimp/lib)
		kuhl_add_include_path(${path}/assimp/include)
		if(WIN32) # ASSIMP - Windows
			kuhl_add_library_path(${path}/assimp-*/lib/Debug)
			kuhl_add_include_path(${path}/assimp-*/include)
		endif()
	endif()
	
	# ImageMagick
	if(NOT DEFINED ImageMagick_MagickCore_LIBRARY)
		kuhl_add_library_path(${path}/ImageMagick/.libs)
		kuhl_add_include_path(${path}/ImageMagick/)

		kuhl_add_include_path("C:/Program Files (x86)/ImageMagick-6.*/include")
		kuhl_add_library_path("C:/Program Files (x86)/ImageMagick-6.*/")

		# macOS homebrew. Need to add this path since default path
		# contains ImageMagick 7
		kuhl_add_library_path("/usr/local/opt/ImageMagick@6/lib")
		kuhl_add_include_path("/usr/local/opt/ImageMagick@6/include/ImageMagick-6")
	endif()

	# VRPN
	if(NOT DEFINED VRPN_LIBRARY_VRPN)
		if(WIN32)
			kuhl_add_library_path(${path}/vrpn/pc_win32/Debug)
			kuhl_add_library_path(${path}/vrpn/quat/pc_win32/Debug)
			kuhl_add_include_path(${path}/vrpn)
			kuhl_add_include_path(${path}/vrpn/quat)
		else()
			kuhl_add_library_path(${path}/vrpn/build)
			kuhl_add_library_path(${path}/vrpn/build/quat)
			kuhl_add_include_path(${path}/vrpn)
			kuhl_add_include_path(${path}/vrpn/quat)
		endif()
	endif()

	# OVR - Linux
	if(NOT DEFINED OVR_LIBRARIES)
		if(WIN32)
			kuhl_add_include_path(${path}/OculusSDK/LibOVR/Include)
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				kuhl_add_library_path(${path}/OculusSDK/LibOVR/Lib/Windows/x64/Debug/VS*/)
				#kuhl_add_library_path(${path}/OculusSDK/LibOVR/Lib/Windows/x64/Release)
			else()
				kuhl_add_library_path(${path}/OculusSDK/LibOVR/Lib/Windows/Win32/Debug/VS*)
				#kuhl_add_library_path(${path}/OculusSDK/LibOVR/Lib/Windows/Win32/Release)
			endif()
		else()
			kuhl_add_library_path(${path}/ovr_sdk_linux_0.5.0.1/LibOVR/Lib/Linux/x86_64/Release)
			kuhl_add_library_path(${path}/ovr_sdk_linux_0.5.0.1/LibOVR/Lib/Linux/x86_64/Debug)
			kuhl_add_include_path(${path}/ovr_sdk_linux_0.5.0.1/LibOVR/Include)
			kuhl_add_include_path(${path}/ovr_sdk_linux_0.5.0.1/LibOVR/Src)
		endif()
	endif()
	
endfunction(kuhl_add_paths)

if(NOT WIN32)
	kuhl_add_paths(/home/kuhl/public-ogl) # CCSR, self-administered machines
	kuhl_add_paths(/local/kuhl-public-share/opengl) # MTU campus linux lab machines
	kuhl_add_paths(/research/kuhl/public-ogl) # ivs
endif()
kuhl_add_paths(${CMAKE_CURRENT_SOURCE_DIR}/..) # Alongside this folder
kuhl_add_paths(${CMAKE_CURRENT_SOURCE_DIR}/../opengl-libs) # Alongside this folder in a folder named opengl-libs

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

# --- GLFW3 ---
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIR})

# --- GLEW ---
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})


# --- ImageMagick (recommended, optional) ---
#
# See help/IMAGEMAGICK_INSTALL.txt for information about the versions
# that this code works with.
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

# --- FreeType ---
find_package(Freetype)
if (FREETYPE_FOUND)
	include_directories(${FREETYPE_INCLUDE_DIRS})
	set(FREETYPE_FOUND_DEFINITION "KUHL_UTIL_USE_FREETYPE")
else()
	set(FREETYPE_FOUND_DEFINITION "")
endif()


# --- VRPN ---
find_package(VRPN)
if(VRPN_FOUND)
	include_directories(${VRPN_INCLUDE_DIRS})
	set(MISSING_VRPN_DEFINITION "")
else()
	set(MISSING_VRPN_DEFINITION "MISSING_VRPN")
endif()

# --- FFMPEG ---
find_package(FFmpeg)
if(FFMPEG_FOUND)
	include_directories(${FFMPEG_INCLUDE_DIRS})
	set(HAVE_FFMPEG_DEFINITION "HAVE_FFMPEG")
	else()
	set(HAVE_FFMPEG_DEFINITION "")
endif()

# --- pthreads (required for DGR) ---
set(CMAKE_THREADS_PREFER_PTHREAD TRUE)   # prefer pthread over other threading libraries
# set(THREADS_PREFER_PTHREAD_FLAG TRUE)   # prefer -pthread compiler flag over just using -lpthread, but it might not be supported by all compilers.
find_package(Threads)

# --- LibOVR (Oculus Rift) ---
set(MISSING_OVR_DEFINITION "MISSING_OVR")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(OVR)
	if(OVR_FOUND)
		# LibOVR relies on a LibOVRRT library at run time. It must be in /usr/lib or /usr/local/lib or in the same directory as the binary.
		# Here, we make "cmake ." copy the .so files into the bin folder.
		add_custom_target(copyOVR)
		# TODO: The PATH variable below should be renamed to DIRECTORY once all computers use versions newer than cmake 2.8.11 (Rekhi lab uses 2.8.11)
		get_filename_component(ovrSoDir ${OVR_LIBRARY_SO} PATH)
		file(GLOB SO_FILES "${ovrSoDir}/*.so" "${ovrSoDir}/*.so.*")
		foreach(SOFILE ${SO_FILES})
			add_custom_command(TARGET copyOVR PRE_BUILD
				COMMAND ${CMAKE_COMMAND} -E
				copy ${SOFILE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		endforeach()

		include_directories(${OVR_INCLUDE_DIRS})
		set(MISSING_OVR_DEFINITION "")
    endif()
elseif(WIN32)
	find_package(OVR)
	if(OVR_FOUND)
		include_directories(${OVR_INCLUDE_DIRS})
		set(MISSING_OVR_DEFINITION "")
	endif()
endif()

# Set the preprocessor flags.
set(PREPROC_DEFINE "MOUSEMOVE_GLFW;${FREETYPE_FOUND_DEFINITION};${ASSIMP_FOUND_DEFINITION};${MISSING_VRPN_DEFINITION};${MISSING_OVR_DEFINITION};${IMAGEMAGICK_FOUND_DEFINITION};${HAVE_FFMPEG_DEFINITION}")

# Look in lib folder for libraries and header files
include_directories("lib")


# Process cmake files in the subdirectories.
# build libkuhl.a
add_subdirectory(${PROJECT_SOURCE_DIR}/lib) 
# build sample programs
add_subdirectory(${PROJECT_SOURCE_DIR}/samples)
# build fake vrpn server
add_subdirectory(${PROJECT_SOURCE_DIR}/vrpn)
# build DGR relay
add_subdirectory(${PROJECT_SOURCE_DIR}/dgr)
# build self tests
add_subdirectory(${PROJECT_SOURCE_DIR}/selftests)

