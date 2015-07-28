
# IVS - installed version is too old.
set(CMAKE_LIBRARY_PATH "/research/kuhl/public-ogl/ImageMagick-6.8.9-5/magick/.libs/" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "/research/kuhl/public-ogl/ImageMagick-6.8.9-5/" ${CMAKE_INCLUDE_PATH})
# CCSR - installed version is too old.
set(CMAKE_LIBRARY_PATH "/home/kuhl/public-ogl/ImageMagick-6.8.9-6/magick/.libs/" ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH "/home/kuhl/public-ogl/ImageMagick-6.8.9-6/" ${CMAKE_INCLUDE_PATH})



# This file is here because the FindImageMagick.cmake file that comes
# with older versions of cmake does not work as well as this
# one. Specifically, older versions of this file do not correctly
# detect libraries with suffixes 6Q16 (in libMagickCore-6.Q16.so).
#
# This file is based on the file that is packed with CMake and
# therefore is licensed under the license used by CMake.
#
# - Find the ImageMagick binary suite.
# This module will search for a set of ImageMagick tools specified
# as components in the FIND_PACKAGE call. Typical components include,
# but are not limited to (future versions of ImageMagick might have
# additional components not listed here):
#
#  animate
#  compare
#  composite
#  conjure
#  convert
#  display
#  identify
#  import
#  mogrify
#  montage
#  stream
#
# If no component is specified in the FIND_PACKAGE call, then it only
# searches for the ImageMagick executable directory. This code defines
# the following variables:
#
#  ImageMagick_FOUND                  - TRUE if all components are found.
#  ImageMagick_EXECUTABLE_DIR         - Full path to executables directory.
#  ImageMagick_<component>_FOUND      - TRUE if <component> is found.
#  ImageMagick_<component>_EXECUTABLE - Full path to <component> executable.
#  ImageMagick_VERSION_STRING         - the version of ImageMagick found
#                                       (since CMake 2.8.8)
#
# ImageMagick_VERSION_STRING will not work for old versions like 5.2.3.
#
# There are also components for the following ImageMagick APIs:
#
#  Magick++
#  MagickWand
#  MagickCore
#
# For these components the following variables are set:
#
#  ImageMagick_FOUND                    - TRUE if all components are found.
#  ImageMagick_INCLUDE_DIRS             - Full paths to all include dirs.
#  ImageMagick_LIBRARIES                - Full paths to all libraries.
#  ImageMagick_<component>_FOUND        - TRUE if <component> is found.
#  ImageMagick_<component>_INCLUDE_DIRS - Full path to <component> include dirs.
#  ImageMagick_<component>_LIBRARIES    - Full path to <component> libraries.
#
# Example Usages:
#  find_package(ImageMagick)
#  find_package(ImageMagick COMPONENTS convert)
#  find_package(ImageMagick COMPONENTS convert mogrify display)
#  find_package(ImageMagick COMPONENTS Magick++)
#  find_package(ImageMagick COMPONENTS Magick++ convert)
#
# Note that the standard FIND_PACKAGE features are supported
# (i.e., QUIET, REQUIRED, etc.).

#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2014 Kitware, Inc.
# Copyright 2000-2011 Insight Software Consortium
# Copyright 2007-2008 Miguel A. Figueroa-Villanueva <miguelf at ieee dot org>
# Copyright 2012 Rolf Eike Beer <eike@sf-mail.de>
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.

# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.

# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# ------------------------------------------------------------------------------

# The above copyright and license notice applies to distributions of
# CMake in source and binary form.  Some source files contain additional
# notices of original copyright by their contributors; see each source
# for details.  Third-party software packages supplied with CMake under
# compatible licenses provide their own copyright notices documented in
# corresponding subdirectories.

# ------------------------------------------------------------------------------

# CMake was initially developed by Kitware with the following sponsorship:

#  * National Library of Medicine at the National Institutes of Health
#    as part of the Insight Segmentation and Registration Toolkit (ITK).

#  * US National Labs (Los Alamos, Livermore, Sandia) ASC Parallel
#    Visualization Initiative.

#  * National Alliance for Medical Image Computing (NAMIC) is funded by the
#    National Institutes of Health through the NIH Roadmap for Medical Research,
#    Grant U54 EB005149.

#  * Kitware, Inc.
#=============================================================================


#---------------------------------------------------------------------
# Helper functions
#---------------------------------------------------------------------
function(FIND_IMAGEMAGICK_API component header)
  set(ImageMagick_${component}_FOUND FALSE PARENT_SCOPE)

  find_path(ImageMagick_${component}_INCLUDE_DIR
    NAMES ${header}
    PATHS
      ${ImageMagick_INCLUDE_DIRS}
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/include"
    PATH_SUFFIXES
      ImageMagick ImageMagick-6
    DOC "Path to the ImageMagick arch-independent include dir."
    )
  find_path(ImageMagick_${component}_ARCH_INCLUDE_DIR
    NAMES magick/magick-baseconfig.h
    HINTS
      ${PC_${component}_INCLUDEDIR}
      ${PC_${component}_INCLUDE_DIRS}
    PATHS
      ${ImageMagick_INCLUDE_DIRS}
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/include"
    PATH_SUFFIXES
      ImageMagick ImageMagick-6
    DOC "Path to the ImageMagick arch-specific include dir."
    )
  find_library(ImageMagick_${component}_LIBRARY
    NAMES ${ARGN}
    PATHS
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]/lib"
    DOC "Path to the ImageMagick Magick++ library."
    )

  # old version have only indep dir
  if(ImageMagick_${component}_INCLUDE_DIR AND ImageMagick_${component}_LIBRARY)
    set(ImageMagick_${component}_FOUND TRUE PARENT_SCOPE)

    # Construct per-component include directories.
    set(ImageMagick_${component}_INCLUDE_DIRS
      ${ImageMagick_${component}_INCLUDE_DIR}
      )
    if(ImageMagick_${component}_ARCH_INCLUDE_DIR)
      list(APPEND ImageMagick_${component}_INCLUDE_DIRS
        ${ImageMagick_${component}_ARCH_INCLUDE_DIR})
    endif()
    list(REMOVE_DUPLICATES ImageMagick_${component}_INCLUDE_DIRS)
    set(ImageMagick_${component}_INCLUDE_DIRS
      ${ImageMagick_${component}_INCLUDE_DIRS} PARENT_SCOPE)

    # Add the per-component include directories to the full include dirs.
    list(APPEND ImageMagick_INCLUDE_DIRS ${ImageMagick_${component}_INCLUDE_DIRS})
    list(REMOVE_DUPLICATES ImageMagick_INCLUDE_DIRS)
    set(ImageMagick_INCLUDE_DIRS ${ImageMagick_INCLUDE_DIRS} PARENT_SCOPE)

    list(APPEND ImageMagick_LIBRARIES
      ${ImageMagick_${component}_LIBRARY}
      )
    set(ImageMagick_LIBRARIES ${ImageMagick_LIBRARIES} PARENT_SCOPE)
  endif()
endfunction()

function(FIND_IMAGEMAGICK_EXE component)
  set(_IMAGEMAGICK_EXECUTABLE
    ${ImageMagick_EXECUTABLE_DIR}/${component}${CMAKE_EXECUTABLE_SUFFIX})
  if(EXISTS ${_IMAGEMAGICK_EXECUTABLE})
    set(ImageMagick_${component}_EXECUTABLE
      ${_IMAGEMAGICK_EXECUTABLE}
       PARENT_SCOPE
       )
    set(ImageMagick_${component}_FOUND TRUE PARENT_SCOPE)
  else()
    set(ImageMagick_${component}_FOUND FALSE PARENT_SCOPE)
  endif()
endfunction()

#---------------------------------------------------------------------
# Start Actual Work
#---------------------------------------------------------------------
# Try to find a ImageMagick installation binary path.
find_path(ImageMagick_EXECUTABLE_DIR
  NAMES mogrify${CMAKE_EXECUTABLE_SUFFIX}
  PATHS
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ImageMagick\\Current;BinPath]"
  DOC "Path to the ImageMagick binary directory."
  NO_DEFAULT_PATH
  )
find_path(ImageMagick_EXECUTABLE_DIR
  NAMES mogrify${CMAKE_EXECUTABLE_SUFFIX}
  )

# Find each component. Search for all tools in same dir
# <ImageMagick_EXECUTABLE_DIR>; otherwise they should be found
# independently and not in a cohesive module such as this one.
#unset(ImageMagick_REQUIRED_VARS)
#unset(ImageMagick_DEFAULT_EXECUTABLES)
set(ImageMagick_FOUND TRUE)
foreach(component ${ImageMagick_FIND_COMPONENTS}
    # DEPRECATED: forced components for backward compatibility
    convert mogrify import montage composite
    )
  if(component STREQUAL "Magick++")
    FIND_IMAGEMAGICK_API(Magick++ Magick++.h
      Magick++ CORE_RL_Magick++_ Magick++-6.Q16 Magick++-Q16 Magick++-6.Q8 Magick++-Q8 Magick++-6.Q16HDRI Magick++-Q16HDRI Magick++-6.Q8HDRI Magick++-Q8HDRI
      )
    list(APPEND ImageMagick_REQUIRED_VARS ImageMagick_Magick++_LIBRARY)
  elseif(component STREQUAL "MagickWand")
    FIND_IMAGEMAGICK_API(MagickWand wand/MagickWand.h
      Wand MagickWand CORE_RL_wand_ MagickWand-6.Q16 MagickWand-Q16 MagickWand-6.Q8 MagickWand-Q8 MagickWand-6.Q16HDRI MagickWand-Q16HDRI MagickWand-6.Q8HDRI MagickWand-Q8HDRI
      )
    list(APPEND ImageMagick_REQUIRED_VARS ImageMagick_MagickWand_LIBRARY)
  elseif(component STREQUAL "MagickCore")
    FIND_IMAGEMAGICK_API(MagickCore magick/MagickCore.h
      CORE_RL_magick_ MagickCore-6.Q16 MagickCore-Q16 MagickCore-6.Q8 MagickCore-Q8 MagickCore-6.Q16HDRI MagickCore-Q16HDRI MagickCore-6.Q8HDRI MagickCore-Q8HDRI MagickCore Magick
      )
    list(APPEND ImageMagick_REQUIRED_VARS ImageMagick_MagickCore_LIBRARY)
  else()
    if(ImageMagick_EXECUTABLE_DIR)
      FIND_IMAGEMAGICK_EXE(${component})
    endif()
ENDIF(component STREQUAL "Magick++")
  
  IF(NOT ImageMagick_${component}_FOUND)
    LIST(FIND ImageMagick_FIND_COMPONENTS ${component} is_requested)
    IF(is_requested GREATER -1)
      SET(ImageMagick_FOUND FALSE)
    ENDIF(is_requested GREATER -1)
  ENDIF(NOT ImageMagick_${component}_FOUND)
ENDFOREACH(component)

SET(ImageMagick_INCLUDE_DIRS ${ImageMagick_INCLUDE_DIRS})
SET(ImageMagick_LIBRARIES ${ImageMagick_LIBRARIES})

#---------------------------------------------------------------------
# Standard Package Output
#---------------------------------------------------------------------
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  ImageMagick DEFAULT_MSG ImageMagick_LIBRARIES
  )
# Maintain consistency with all other variables.
set(ImageMagick_FOUND ${IMAGEMAGICK_FOUND})

#---------------------------------------------------------------------
# DEPRECATED: Setting variables for backward compatibility.
#---------------------------------------------------------------------
set(IMAGEMAGICK_BINARY_PATH          ${ImageMagick_EXECUTABLE_DIR}
    CACHE PATH "Path to the ImageMagick binary directory.")
set(IMAGEMAGICK_CONVERT_EXECUTABLE   ${ImageMagick_convert_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's convert executable.")
set(IMAGEMAGICK_MOGRIFY_EXECUTABLE   ${ImageMagick_mogrify_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's mogrify executable.")
set(IMAGEMAGICK_IMPORT_EXECUTABLE    ${ImageMagick_import_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's import executable.")
set(IMAGEMAGICK_MONTAGE_EXECUTABLE   ${ImageMagick_montage_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's montage executable.")
set(IMAGEMAGICK_COMPOSITE_EXECUTABLE ${ImageMagick_composite_EXECUTABLE}
    CACHE FILEPATH "Path to ImageMagick's composite executable.")
mark_as_advanced(
  IMAGEMAGICK_BINARY_PATH
  IMAGEMAGICK_CONVERT_EXECUTABLE
  IMAGEMAGICK_MOGRIFY_EXECUTABLE
  IMAGEMAGICK_IMPORT_EXECUTABLE
  IMAGEMAGICK_MONTAGE_EXECUTABLE
  IMAGEMAGICK_COMPOSITE_EXECUTABLE
  )
