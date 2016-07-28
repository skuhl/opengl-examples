include(FindPackageHandleStandardArgs)

MACRO(FFMPEG_FIND varname shortname headername)

    FIND_PATH(FFMPEG_${varname}_INCLUDE_DIRS lib${shortname}/${headername}
        PATHS
        ${FFMPEG_ROOT}/include
		~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        PATH_SUFFIXES ffmpeg
        DOC "Location of FFMPEG Headers"
    )

	# TODO: Change PATH to DIRECTORY in get_filename_component all
        # MTU computers are upgraded beyond cmake 2.8.11
	# https://cmake.org/cmake/help/v3.0/command/get_filename_component.html

	# Include files are all in /usr/include/libavformat/avformat.h --- but we only want the /usr/include part:
	get_filename_component(FFMPEG_${varname}_INCLUDE_DIRS ${FFMPEG_${varname}_INCLUDE_DIRS} PATH )
	LIST(APPEND FFMPEG_INCLUDE_DIRS FFMPEG_${varname}_INCLUDE_DIRS)
	
    FIND_LIBRARY(FFMPEG_${varname}_LIBRARIES
        NAMES ${shortname}
        PATHS
        ${FFMPEG_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
		/usr/local/lib
        /usr/local/lib64
        /usr/lib
        /usr/lib64
        DOC "Location of FFMPEG Libraries"
    )
	LIST(APPEND FFMPEG_LIBRARIES ${FFMPEG_${varname}_LIBRARIES})

ENDMACRO(FFMPEG_FIND)

FFMPEG_FIND(LIBAVFORMAT avformat avformat.h)
FFMPEG_FIND(LIBAVDEVICE avdevice avdevice.h)
FFMPEG_FIND(LIBAVCODEC  avcodec  avcodec.h)
FFMPEG_FIND(LIBAVUTIL   avutil   avutil.h)
FFMPEG_FIND(LIBSWSCALE  swscale  swscale.h) 



find_package_handle_standard_args(FFmpeg DEFAULT_MSG
										 FFMPEG_LIBAVUTIL_LIBRARIES
										 FFMPEG_LIBAVFORMAT_LIBRARIES
										 FFMPEG_LIBAVDEVICE_LIBRARIES
										 FFMPEG_LIBAVCODEC_LIBRARIES
 										 FFMPEG_LIBSWSCALE_LIBRARIES
										 FFMPEG_LIBAVFORMAT_INCLUDE_DIRS			 
										 FFMPEG_LIBAVDEVICE_INCLUDE_DIRS
										 FFMPEG_LIBAVCODEC_INCLUDE_DIRS
										 FFMPEG_LIBAVUTIL_INCLUDE_DIRS
 										 FFMPEG_LIBSWSCALE_INCLUDE_DIRS 									
										)

