if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    message(FATAL_ERROR "Don't run cmake here. Run it in the root folder of this repository. Then run 'make selftests'")
endif()
cmake_minimum_required(VERSION 2.6)


####################################
# Edit the following areas to add or remove programs to compile
#
# If you add a new name here, there must be an .c file with the same
# name that contains a main() function.
####################################
# Programs that need ASSIMP
set(NEED_ASSIMP )
# Programs that don't rely on ASSIMP
set(NEED_NOTHING selftest-euler selftest-euler-matrix selftest-matrix-inverse)


# IMPORTANT: If ASSIMP is installed, NEED_NOTHING will link against
# assimp. This is required because the kuhl_geometry struct changes
# sizes depending on if assimp is present---and the library and the
# sample programs must be compiled using the same definition of that
# struct!


# Construct a list of programs that we want to compile based on which libraries are available.
set(PROGRAMS_TO_MAKE ${NEED_NOTHING})
if(ASSIMP_FOUND)
	set(PROGRAMS_TO_MAKE ${PROGRAMS_TO_MAKE} ${NEED_ASSIMP})
else()
	message(WARNING "ASSIMP was not found, not compiling: ${NEED_ASSIMP}")
endif()



# Compile the list of programs.
foreach(arg ${PROGRAMS_TO_MAKE})
	if(EXISTS samples/${arg}.cpp)   # Figure out if the program is a c or cpp file
		set(SOURCE_FILE ${arg}.cpp)
	else()
		set(SOURCE_FILE ${arg}.c)
	endif()
	add_executable(${arg} EXCLUDE_FROM_ALL ${SOURCE_FILE})

	target_link_libraries(${arg} kuhl)
	if(VRPN_FOUND)  # Add VRPN to the list if it is available
		target_link_libraries(${arg} ${VRPN_LIBRARIES})
	endif()
	if(OVR_FOUND) # Add Oculus LibOVR to the list if it is available
		target_link_libraries(${arg} ${OVR_LIBRARIES} ${CMAKE_DL_LIBS})
	endif()
	if(ImageMagick_FOUND)
		target_link_libraries(${arg} ${ImageMagick_LIBRARIES})
	endif()
	if(ASSIMP_FOUND)
		# Link to assimp if we found it, even if it isn't needed for
		# this program. We need to do this because the library will
		# require assimp even if the program doesn't.
		target_link_libraries(${arg} ${ASSIMP_LIBRARIES})
	endif()
	if(FREETYPE_FOUND)
		target_link_libraries(${arg} ${FREETYPE_LIBRARIES})
	endif()

	target_link_libraries(${arg} ${GLEW_LIBRARIES} ${M_LIB} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES} )
	if(APPLE)
		# Some Mac OSX machines need this to ensure that freeglut.h is found.
		target_include_directories(${arg} PUBLIC "/opt/X11/include/freetype2/")
	endif()

	set_target_properties(${arg} PROPERTIES LINKER_LANGUAGE "CXX")
	set_target_properties(${arg} PROPERTIES COMPILE_DEFINITIONS "${PREPROC_DEFINE}")

endforeach()
add_custom_target(selftests DEPENDS ${PROGRAMS_TO_MAKE})
