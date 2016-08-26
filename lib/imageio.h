/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 
Read and write numerous image file formats via ImageMagick's
MagickCore library. For a list of supported file formats, see
http://www.imagemagick.org/script/formats.php
 
Although GraphicsMagick is similar to ImageMagick, this code only
works with ImageMagick.
 
All functions provided by imageio.c assume that images are stored
in 1D arrays (row major order, origin at the bottom left of the
image). If you have an RGB image (i.e., a 3 component image), the
first three bytes in the array is the color of the bottom-left
corner.
 
The ImageMagick library license allows people to use the library in
code that is published under a different license than the
ImageMagick license:
http://www.imagemagick.org/script/license.php

@author Scott Kuhl
*/


#pragma once

#ifndef _WIN32
/* These variables resolve compile-time warning messages recent
 * versions of ImageMagick */
#define MAGICKCORE_HDRI_ENABLE 0
#define MAGICKCORE_QUANTUM_DEPTH 16
#endif

/* TODO: This changes to MagickCore/MagickCore.h in ImageMagick 7 */
#include <magick/MagickCore.h>

#ifdef __cplusplus
extern "C" {
#endif


/** Calculates the appropriate index into the 1D array from a 2D coordinate.
   @param x The x coordinate (0=left)
   @param y The y coordinate (0=bottom)
   @param component Which component do we want the index of (red=0, green=1, etc)
   @param width The width of the image in pixels
   @param totalComponents The number of components in the image.
   @return The index to be used with a 1D array.
*/
#define XY2INDEX(x,y,component, width, totalComponents) \
    ( ((x)+(y)*(width))*(totalComponents)+(component) )

/** Set this variable to 1 to cause imageio.c functions to print
 * diagnostic information. */
#define IMAGEIO_DEBUG 0

/** The imageio_info struct is used for both writing and reading files with imageio.c. If you are writing an image, the struct provides information about the array of data that you want to write to disk. If you are reading an image file from disk, it provides information about the dimensions of the image. */
typedef struct {

	unsigned long width; /**< Width of the image in pixels. Should be
	                      * set to the size of the image stored in the
	                      * array when used with imageout(). This
	                      * value will be filled in automatically when
	                      * used with imagein(). @see height */
	unsigned long height; /**< Height of the image in pixels. @see width */
	char *comment;  /**< A comment that was read (or is to be written)
	                 * to an image. If you are writing an image and
	                 * don't want to set a comment, set this variable
	                 * to NULL.*/

	/** Specifies which components are in the array that we are
       writing or reading. "map" indicates what color channels are in
       the array which we want written to an image file (or that we
       want the image file to be read into.
       
       Example values for map:<br>
       RGB<br>
       RGBA<br>
       RGBP  (rgb image + 1 padding)<br>
       I     (I=grayscale)<br>
       IA    (grayscale+alpha)<br>
       CMYK

       If an alpha value is in the data which is being written or read from
       disk, the alpha values are not pre-multiplied.  I.e., (0,1,0,.5)
       means full red with half transparency.  In some file formats, this
       value is pre-multiplied to (0,.5,0,.5).  If such a file format is
       being used, ImageMagick performs the conversion for us.
    */
	char *map;

	/** Type of array: CharPixel, DoublePixel, FloatPixel, IntegerPixel,
       LongPixel, or ShortPixel.  Range of char/int/short are all
       [0--MaxValueOfType].  Range of float and double are [0--1]. */
    StorageType type;


    /* Information only used for file OUTPUT: */
	char *filename; /**< Name of the file that we want imagein()
	                 * to read or the name of the file that we
	                 * want imageout() to write to. As long as
	                 * the filename extension is supported, the
	                 * code will write the image correctly to
	                 * disk. */
    unsigned long depth; /**< Number of bits per color channel that
                          * should be written to the file. This
                          * variable is only used when writing files
                          * to disk, not when reading files. */

    int quality; /**< Quality for the image output (0--100). For lossy
                  * file formats a small number means that the output
                  * file will be highly compressed. For lossless
                  * formats, a small number means the output file will
                  * be less compressed. */


    ColorspaceType colorspace; /**< Colorspace conversion. This should
                                * typically be set to sRGBColorspace
                                * (with imagemagick 6.7.5-5 or
                                * higher). However, it can be used to
                                * apply colorspace conversions or to
                                * write CMYK images to files if the
                                * file format supports it. */

} imageio_info;



void* imagein(imageio_info *iio_info);
int imageout(const imageio_info *iio_info, void* array);
char* image_label(const char *label, int *width, int *height, float color[3], float bgcolor[4], double pointsize);

#ifdef __cplusplus
} // end extern "C"
#endif
