# IVS has freeglut 2.4 installed, and 2.8 at the location below:
set(CMAKE_LIBRARY_PATH /research/kuhl/public-ogl/glut/src/.libs ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /research/kuhl/public-ogl/glut/include/ ${CMAKE_INCLUDE_PATH})
# freeglut version 3 on newell.cs.  Note that the library is in a different subfolder starting with freeglut 3.0.0
set(CMAKE_LIBRARY_PATH /home/kuhl/public-ogl/glut/lib ${CMAKE_LIBRARY_PATH})


# - try to find glut library and include files
#  GLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
#  GLUT_LIBRARIES, the libraries to link against
#  GLUT_FOUND, If false, do not try to use GLUT.
# Also defined, but not for general use are:
#  GLUT_glut_LIBRARY = the full path to the glut library.
#  GLUT_Xmu_LIBRARY  = the full path to the Xmu library.
#  GLUT_Xi_LIBRARY   = the full path to the Xi Library.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (WIN32)
  find_path( GLUT_INCLUDE_DIR NAMES GL/glut.h
    PATHS  ${GLUT_ROOT_PATH}/include )
  find_library( GLUT_glut_LIBRARY NAMES glut glut32 freeglut
    PATHS
    ${OPENGL_LIBRARY_DIR}
    ${GLUT_ROOT_PATH}/Release
    )
else ()

  if (APPLE)
    find_path(GLUT_INCLUDE_DIR glut.h ${OPENGL_LIBRARY_DIR})
    find_library(GLUT_glut_LIBRARY GLUT DOC "GLUT library for OSX")
    find_library(GLUT_cocoa_LIBRARY Cocoa DOC "Cocoa framework for OSX")

    if(GLUT_cocoa_LIBRARY AND NOT TARGET GLUT::Cocoa)
      add_library(GLUT::Cocoa UNKNOWN IMPORTED)
      # Cocoa should always be a Framework, but we check to make sure.
      if(GLUT_cocoa_LIBRARY MATCHES "/([^/]+)\\.framework$")
        set_target_properties(GLUT::Cocoa PROPERTIES
          IMPORTED_LOCATION "${GLUT_cocoa_LIBRARY}/${CMAKE_MATCH_1}")
      else()
        set_target_properties(GLUT::Cocoa PROPERTIES
          IMPORTED_LOCATION "${GLUT_cocoa_LIBRARY}")
      endif()
    endif()
  else ()

    if (BEOS)

      set(_GLUT_INC_DIR /boot/develop/headers/os/opengl)
      set(_GLUT_glut_LIB_DIR /boot/develop/lib/x86)

    else()

      find_library( GLUT_Xi_LIBRARY Xi
        /usr/openwin/lib
        )

      find_library( GLUT_Xmu_LIBRARY Xmu
        /usr/openwin/lib
        )

      if(GLUT_Xi_LIBRARY AND NOT TARGET GLUT::Xi)
        add_library(GLUT::Xi UNKNOWN IMPORTED)
        set_target_properties(GLUT::Xi PROPERTIES
          IMPORTED_LOCATION "${GLUT_Xi_LIBRARY}")
      endif()

      if(GLUT_Xmu_LIBRARY AND NOT TARGET GLUT::Xmu)
        add_library(GLUT::Xmu UNKNOWN IMPORTED)
        set_target_properties(GLUT::Xmu PROPERTIES
          IMPORTED_LOCATION "${GLUT_Xmu_LIBRARY}")
      endif()

    endif ()

    find_path( GLUT_INCLUDE_DIR GL/glut.h
      /usr/include/GL
      /usr/openwin/share/include
      /usr/openwin/include
      /opt/graphics/OpenGL/include
      /opt/graphics/OpenGL/contrib/libglut
      ${_GLUT_INC_DIR}
      )

    find_library( GLUT_glut_LIBRARY glut
      /usr/openwin/lib
      ${_GLUT_glut_LIB_DIR}
      )

    unset(_GLUT_INC_DIR)
    unset(_GLUT_glut_LIB_DIR)

  endif ()

endif ()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLUT REQUIRED_VARS GLUT_glut_LIBRARY GLUT_INCLUDE_DIR)

if (GLUT_FOUND)
  # Is -lXi and -lXmu required on all platforms that have it?
  # If not, we need some way to figure out what platform we are on.
  set( GLUT_LIBRARIES
    ${GLUT_glut_LIBRARY}
    ${GLUT_Xmu_LIBRARY}
    ${GLUT_Xi_LIBRARY}
    ${GLUT_cocoa_LIBRARY}
    )

  if(NOT TARGET GLUT::GLUT)
    add_library(GLUT::GLUT UNKNOWN IMPORTED)
    set_target_properties(GLUT::GLUT PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${GLUT_INCLUDE_DIR}")
    if(GLUT_glut_LIBRARY MATCHES "/([^/]+)\\.framework$")
      set_target_properties(GLUT::GLUT PROPERTIES
        IMPORTED_LOCATION "${GLUT_glut_LIBRARY}/${CMAKE_MATCH_1}")
    else()
      set_target_properties(GLUT::GLUT PROPERTIES
        IMPORTED_LOCATION "${GLUT_glut_LIBRARY}")
    endif()

    if(TARGET GLUT::Xmu)
      set_property(TARGET GLUT::GLUT APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES GLUT::Xmu)
    endif()

    if(TARGET GLUT::Xi)
      set_property(TARGET GLUT::GLUT APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES GLUT::Xi)
    endif()

    if(TARGET GLUT::Cocoa)
      set_property(TARGET GLUT::GLUT APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES GLUT::Cocoa)
    endif()
  endif()

  #The following deprecated settings are for backwards compatibility with CMake1.4
  set (GLUT_LIBRARY ${GLUT_LIBRARIES})
  set (GLUT_INCLUDE_PATH ${GLUT_INCLUDE_DIR})
endif()

mark_as_advanced(
  GLUT_INCLUDE_DIR
  GLUT_glut_LIBRARY
  GLUT_Xmu_LIBRARY
  GLUT_Xi_LIBRARY
  )
