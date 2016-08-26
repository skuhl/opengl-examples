/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include "windows-compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "imageio.h"

/** Converts an ImageMagick StorageType enum into the size of that
 * storage type in bytes. This function is intended to be used inside
 * of imageio.c only.
 *
 * @param t The input StorageType to get the size of.  @return The size
 * of the StorageType in bytes.
 */
static int imageio_type_to_bytes(StorageType t)
{
	switch(t)
	{
		case CharPixel:    return sizeof(char);
		case DoublePixel:  return sizeof(double);
		case FloatPixel:   return sizeof(float);
		case IntegerPixel: return sizeof(int);
		case LongPixel:    return sizeof(long);
		case ShortPixel:   return sizeof(short);
		default:
			fprintf(stderr, "imageio_type_to_bytes: Wrong type\n");
			return 0;
	}
}

/** ImageMagick defaults with the first pixel at the upper left
 * corner. OpenGL and other packages put the first pixel at the bottom
 * left corner.  This code flips the image depending on how imageio
 * was compiled. This function is intended for use within imageio.c
 * only. */
static Image* imageio_flip(Image *image)
{
	/* Flip image so origin is at bottom left. */
	ExceptionInfo exception;
	GetExceptionInfo(&exception);
	Image *flipped = FlipImage(image, &exception);
	DestroyImage(image);
	MagickError(exception.severity, exception.reason, exception.description);
	DestroyExceptionInfo(&exception);
	return flipped;
}

/** Writes the given image stored in "array" out to the filename
 * specified in iio_info. Returns 1 if successful. Prints an error
 * message and returns 0 if there is a failure. */
int imageout(const imageio_info *iio_info, void* array)
{
	Image *image = NULL;
	ImageInfo *image_info = CloneImageInfo((ImageInfo*)NULL);
	ExceptionInfo exception;

	if(IMAGEIO_DEBUG) {
		printf("imageout %s: Trying to write file with %lu bit/channel.\n", iio_info->filename, iio_info->depth);
		printf("imageout %s: You provided %d bit/channel %s.\n",
		       iio_info->filename, 8*imageio_type_to_bytes(iio_info->type), iio_info->map);
		printf("imageout %s: Dimensions: %lu x %lu\n", iio_info->filename, iio_info->width, iio_info->height);
	}

	//if(!IsMagickInstantiated())
	MagickCoreGenesis(NULL, MagickTrue);

	GetExceptionInfo(&exception);
	image = ConstituteImage(iio_info->width, iio_info->height, iio_info->map,
	                        iio_info->type, array, &exception);

	/* Tell imagemagick the colorspace of our data. If the colorspace
	   is CMYK and the output file supports it, imagemagick may output
	   a CMYK file. */
	image->colorspace = iio_info->colorspace;

	/* If sRGB colorspace is used, and if the image actually just
	   contains grayscale pixels, ImageMagick may then just output a
	   GrayScale image instead of a true RGB image for some types of
	   file formats (example: tif). The code below forces the output
	   image to always be RGB and not grayscale.

	   This fix is useful because if you use this feature to make many
	   tif screenshots which will later be reconstructed into a video
	   via ffmpeg, ffmpeg seems to only read RGB images. */
	if(image->colorspace == sRGBColorspace)
		image_info->type = TrueColorType;
	
	MagickError(exception.severity, exception.reason, exception.description);
	image = imageio_flip(image);

	SyncAuthenticPixels(image, &image->exception);

	/* Set image comment */
	if(iio_info->comment != NULL)
		SetImageProperty(image, "comment", iio_info->comment);

	/* Set other image information. */
	image_info->quality = iio_info->quality;
	image_info->depth   = iio_info->depth;
	image->depth        = iio_info->depth;
	strncpy(image_info->filename, iio_info->filename, MaxTextExtent-1);
	strncpy(image->filename, iio_info->filename, MaxTextExtent-1);
	/* Write the image out to a file */
	if(WriteImage(image_info, image) == MagickFalse)
	{
		fprintf(stderr, "imageout %s: ERROR %s\n", iio_info->filename, image->exception.reason);
		return 0;
	}
	DestroyImageInfo(image_info);
	DestroyImage(image);
	DestroyExceptionInfo(&exception);
	// No real need to destroy the MagickCore environment, the user
	// might try to load another texture later...
	//MagickCoreTerminus();

	if(IMAGEIO_DEBUG)
		printf("imageout %s: DONE\n", iio_info->filename);

	return 1;
}


/** Reads an image from disk and returns the resulting image data in
 * an array.
 *
 * @param iio_info An imageio_info object.
 *
 * @return An array of pixels or NULL if there is an error reading the
 * file. The array of pixels should be free()'d by the caller when
 * finished with the image.  If iio_info->comment != NULL after
 * imagein returns, the caller should eventually free() it. */
void* imagein(imageio_info *iio_info)
{
	int bytes_per_pixel   = strlen(iio_info->map) * imageio_type_to_bytes(iio_info->type);

	if(IMAGEIO_DEBUG)
	{
		printf("imagein  %s: Requested %d bit/channel %s\n", iio_info->filename,
		       8*imageio_type_to_bytes(iio_info->type), iio_info->map);
		printf("imagein  %s: Reading file...\n", iio_info->filename);
	}

	/* read in the image */
	MagickCoreGenesis(NULL, MagickTrue);
	ExceptionInfo exception;
	GetExceptionInfo(&exception);
	
	ImageInfo *image_info = CloneImageInfo((ImageInfo*)NULL);
	strncpy(image_info->filename, iio_info->filename, MaxTextExtent-1);
	Image *image = ReadImage(image_info, &exception);
	if(image == NULL)
	{
		fprintf(stderr, "imagein  %s: ERROR %s\n", iio_info->filename, exception.reason);
		return NULL;
	}

	/* Loading a non-transparent texture after one with an alpha
	   component seems sometimes causes the non-traparent texture to
	   incorrectly be transparent. The following code attempts to fix
	   this. */
	if(GetImageAlphaChannel(image) == MagickFalse)
		SetImageOpacity(image, 0);

	/* Since ImageMagick 6.7.5-5 (circa 2012), RGB files without a
	  defined colorspace will be marked as being sRGB under the
	  assumption that an RGB would be intended to be displayed on an
	  sRGB monitor. http://www.imagemagick.org/discourse-server/viewtopic.php?f=2&t=20501

	  Therefore, for many files, this colorspace transformation line
	  won't actually cause any conversion. CMYK files, however, would
	  get transformed into sRGB here.

	  It seems that the GRAYColorspace->sRGBColorspace affects the
	  gamma of the image in a strange way---so we skip the transform
	  for that colorspace.

	  See ColorspaceType enum in ImageMagick's colorspace.h to
	  convert colorspace numbers into names.
	*/
	if(image->colorspace != iio_info->colorspace &&
	   image->colorspace != GRAYColorspace)
	{
		printf("imagein  %s: Applying colorspace transformation (%d to %d)...\n", iio_info->filename, image->colorspace, iio_info->colorspace);
		TransformImageColorspace(image, iio_info->colorspace);
	}
	
	
	image = imageio_flip(image);

	if(IMAGEIO_DEBUG)
	{
		printf("imagein  %s: Finished reading from disk.\n", iio_info->filename);
		printf("imagein  %s: Dimensions: %lu x %lu\n", iio_info->filename,
		       image->columns, image->rows);
	}

	/* allocate array to copy data into */
	char *array = malloc(bytes_per_pixel*image->columns*image->rows);
	if(array == NULL)
	{
		printf("imagein  %s: Failed to allocate enough memory.\n", iio_info->filename);
		return NULL;
	}

	/* Copy the data into the array */
	ExportImagePixels(image, 0, 0, image->columns, image->rows, iio_info->map,
	                  iio_info->type, array, &exception);

	MagickError(exception.severity, exception.reason, exception.description);
	SyncAuthenticPixels(image, &image->exception);

	/* Copy information from the file into our iio_info struct */
	iio_info->width = image->columns;
	iio_info->height = image->rows;
	iio_info->quality = -1;  /* N/A */
	iio_info->depth = image->depth;

	const char* imagecomment = GetImageProperty(image, "comment");
	if(imagecomment != NULL)
	{
		// allocate room to copy \0 in string:
		iio_info->comment = (char*) malloc(sizeof(char)*strlen(imagecomment)+1);
		// memory is leaked unless caller frees this:
		strcpy(iio_info->comment, imagecomment);
	}
	else
		iio_info->comment = NULL;

	DestroyImageInfo(image_info);
	DestroyImage(image);
	DestroyExceptionInfo(&exception);
	// No real need to destroy the MagickCore environment, the user
	// might try to load another texture later...
	//MagickCoreTerminus();

	if(IMAGEIO_DEBUG)
		printf("imagein  %s: DONE.\n", iio_info->filename);

	return array;
}


/** Converts a string into an image with the string written on it with
 * a default font. This function returns an RGBA array that represents
 * the pixels. This method is useful to add labels or text overlays in
 * OpenGL programs.
 *
 * @param label The text that you want written on a texture (all on one line).
 * @param width A pointer that will be filled in with the width of the resulting image.
 * @param height A pointer that will be filled in with the height of the resulting image.
 * @param color The color of the text rendered onto the image.
 * @param bgcolor The background color of the image (can be transparent).
 * @param pointsize The size of the font. A larger pointsize will result in larger textures.
 * @return An RGBA array in row-major order that contains the data in the image.
 */
char* image_label(const char *label, int* width, int* height, float color[3], float bgcolor[4], double pointsize)
{
	*width=0;
	*height=0;
	/* Since we are using a copy of ImageMagick that is not at the standard
	   location on CCSR, we need to tell the imagemagick library where to
	   look for config files. Without this, imagemagick prints errors about
	   fonts when you try to make text labels.
	*/
#ifdef __linux__
	setenv("MAGICK_CONFIGURE_PATH", "/home/kuhl/public-vrlab/ImageMagick/config", 1);
#endif


//	if(!IsMagickInstantiated())
	MagickCoreGenesis(NULL, MagickTrue);
	ExceptionInfo exception;
	GetExceptionInfo(&exception);
	MagickPixelPacket background;
	GetMagickPixelPacket(NULL, &background);

	background.red     = (Quantum) (QuantumRange*(bgcolor[0]));
	background.green   = (Quantum) (QuantumRange*(bgcolor[1]));
	background.blue    = (Quantum) (QuantumRange*(bgcolor[2]));
	// Opacity in values are flipped in imagemagick
	background.opacity = (Quantum) (QuantumRange*(1-bgcolor[3]));
	background.matte = MagickTrue;

	// Get a small image to draw on 
	ImageInfo* image_info = CloneImageInfo((ImageInfo*)NULL);
	Image* image = NewMagickImage(image_info, 10, 10, &background);
	DrawInfo* draw_info = CloneDrawInfo(image_info, (DrawInfo*) NULL);

	CloneString(&draw_info->text, label);
	// Set text color, size, position
	draw_info->pointsize = pointsize;
	draw_info->gravity = SouthEastGravity;
	CloneString(&draw_info->geometry, "+0+0");
	draw_info->fill.red   = (Quantum) (QuantumRange*color[0]);
	draw_info->fill.green = (Quantum) (QuantumRange*color[1]);
	draw_info->fill.blue  = (Quantum) (QuantumRange*color[2]);
	// disable antialiasing
	// draw_info->text_antialias = MagickFalse;
	// Opacity in values are flipped in imagemagick
	draw_info->fill.opacity = 0; // opaque
	
	// Figure out how big we'll need to draw
	TypeMetric metric;
	GetTypeMetrics(image, draw_info, &metric);
	*width = (int) metric.width;
	*height = (int) metric.height;

	/* If user passed in an empty string as the label, then width and
	 * height might get set to 0 which could later lead to a
	 * crash. Create a 1x1 pixel image in this case instead. */
	if(*width == 0)
		*width = 1;
	if(*height == 0)
		*height = 1;
	// printf("Texture dimensions: %d %d\n", *width, *height);

// Figure out how to wrap text:
//	char *caption = AcquireString(label);
//	FormatMagickCaption(image, draw_info, MagickTrue, &metric, &caption);
//	CloneString(&draw_info->text, caption);

	// Clear old image
	DestroyImage(image);
	DestroyImageInfo(image_info);

	// Get new image
	image_info = AcquireImageInfo();
	image = NewMagickImage(image_info, *width, *height, &background);

	/* TODO: If we draw a solid background and transparent text, we
	 * don't get a texture where the text is see-through. */
	AnnotateImage(image, draw_info);
//	DisplayImages(image_info, image); //(works best if image isn't transparent

	image = imageio_flip(image);
	char *array = malloc(4*(*width)*(*height));
	ExportImagePixels(image, 0, 0, *width, *height, "RGBA",
	                  CharPixel, array, &exception);
	DestroyImageInfo(image_info);
	DestroyImage(image);
	DestroyDrawInfo(draw_info);
	DestroyExceptionInfo(&exception);

	// No real need to destroy the MagickCore environment, the user
	// might try to load another texture later...
	//MagickCoreTerminus();
	return array;
}
