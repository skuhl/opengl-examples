/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file A slideshow program written in OpenGL 2.0
 *
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"

#define SCROLL_SPEED 30  // number of seconds to scroll past one screen-width of image.
#define MAX_TILES 100
#define SLIDESHOW_WAIT 10 // time in seconds to wait when autoadvance is turned on.
#define MAX_TILES 100

int autoAdvance = 0; // Automatically advance from image to image after a time out.
double lastAdvance = 0; // Time (in ms from when our program started) that we advanced picture.
float scrollAmount = 0; // How far has the image scrolled?

/* The current image that we are displaying. */
GLuint numTiles=0;
GLuint texNames[MAX_TILES];
float aspectRatio;

int alreadyDisplayedTexture = 0; // Used to determine if we need to reload currentTexture.
int currentTexture = 0; // The texture to be displayed.
int totalTextures = 0;
char **globalargv = NULL;

/* Readfile uses imageio to read in an image, and binds it to an OpenGL
 * texture name.  Requires OpenGL 2.0 or better.
 *
 * filename: name of file to load
 *
 * texName: A pointer to where the OpenGL texture name should be stored.
 * (Remember that the "texture name" is really just some unsigned int).
 *
 * returns: aspect ratio of the image in the file.
 */
float readfile(char *filename, GLuint *texName, GLuint *numTiles)
{
	static int verbose=1;  // change this to 0 to print out less info

	/* Try to load the image. */
#if KUHL_UTIL_USE_IMAGEMAGICK
	imageio_info iioinfo;
	iioinfo.filename = filename;
	iioinfo.type = CharPixel;
	iioinfo.map = "RGBA";
	iioinfo.colorspace = sRGBColorspace;
	unsigned char *image = (unsigned char*) imagein(&iioinfo);
	int width  = (int)iioinfo.width;
	int height = (int)iioinfo.height;
#else
	int width  = -1;
	int height = -1;
	int comp = -1;
	int requestedComponents = STBI_rgb_alpha;
	stbi_set_flip_vertically_on_load(1);
	unsigned char *image = (unsigned char*) stbi_load(filename, &width, &height, &comp, requestedComponents);
#endif

	if(image == NULL)
	{
		msg(MSG_FATAL, "Unable to read image: %s\n", filename);
		return -1;
	}

	/* "image" is a 1D array of characters (unsigned bytes) with four
	 * bytes for each pixel (red, green, blue, alpha). The data in "image"
	 * is in row major order. The first 4 bytes are the color information
	 * for the lowest left pixel in the texture. */
	float original_aspectRatio = (float)width/height;
	if(verbose)
		msg(MSG_INFO, "Finished reading %s (%dx%d)\n", filename, width, height);

	/* OpenGL only supports textures if they are small enough. If
	 * the texture is too large, we need to split it into smaller
	 * textures and render each texture on its own
	 * quad. Typically, the limit is 4096 pixels in each
	 * dimension. This code will always split the image vertically
	 * in half...and then decide how many tiles are necessary
	 * horizontally. This allows for long horizontal panoramas but
	 * not long vertical panoramas. */
	
	/* Since we display image as being two tiles tall, make sure
	 * image isn't too tall. */
	if(height > 4096*2)
	{
		msg(MSG_FATAL, "Source image must <= 8192 pixels tall.");
		exit(EXIT_FAILURE);
	}

	int subimgH = height/2; // height of a the tiles
	int workingWidth = width;
	*numTiles = 1; // number of tiles in horizontal direction
	/* Calculate number of tiles horizontally we need. */
	while(workingWidth > 4096)
	{
		/* If image is too wide, split the width of the tile
		 * in half---doubling the number of tiles in the
		 * horizontal direction */
		*numTiles = *numTiles * 2;
		workingWidth = workingWidth / 2;
	}
	int subimgW = workingWidth;

	if(*numTiles > MAX_TILES/2.0)
	{
		msg(MSG_FATAL, "Too many tiles");
		exit(EXIT_FAILURE);
	}

	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA8, subimgW, subimgH,
	             0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	int tmp;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tmp);

	if(tmp == 0)
	{
		msg(MSG_FATAL, "%s: File is too large (%d x %d). I can't load it!\n", filename, subimgW, subimgH);
		free(image);
		exit(EXIT_FAILURE);
	}


	/* Generate all of the textures that we need. */
	glGenTextures(*numTiles*2, texName);

	for(GLuint curTile=0; curTile < *numTiles*2; curTile = curTile+2)
	{
		/* Prepare to copy the data from the array into the
		 * texture, tell OpenGL to skip pixels appropriately
		 * so that each tile gets the right part of the
		 * image. */
		glPixelStorei(GL_UNPACK_ROW_LENGTH,  width);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, curTile/2.0*subimgW);
		glPixelStorei(GL_UNPACK_SKIP_ROWS,   0);

		glBindTexture(GL_TEXTURE_2D, texName[curTile]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, subimgW, subimgH,
		             0, GL_RGBA, GL_UNSIGNED_BYTE, image);

		glPixelStorei( GL_UNPACK_SKIP_PIXELS, curTile/2.0*subimgW );
		glPixelStorei( GL_UNPACK_SKIP_ROWS, subimgH);

		glBindTexture(GL_TEXTURE_2D, texName[curTile+1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, subimgW, subimgH,
		             0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	}

	free(image);
	return original_aspectRatio;
}

int getNextTexture()
{
	int next = currentTexture+1;
	if(next >= totalTextures)
		return 0;
	return next;
}

int getPrevTexture()
{
	int next = currentTexture-1;
	if(next < 0)
		return totalTextures-1;
	return next;
}

/* Deletes any tiles that are already loaded and then loads up the
 * current texture image. */
void loadTexture(int textureIndex)
{
	if(numTiles > 0)
		glDeleteTextures(numTiles*2, texNames);

	scrollAmount = 0;
	aspectRatio = readfile(globalargv[textureIndex], texNames, &numTiles);
	lastAdvance = glfwGetTime();
}

void display(void)
{
	viewmat_begin_frame();
	viewmat_begin_eye(0);
	
	dgr_setget("currentTex", &currentTexture, sizeof(int));
	dgr_setget("scrollAmount", &scrollAmount, sizeof(float));

	//kuhl_limitfps(100);

	/* If the texture has changed since we were previously in display() */
	if(alreadyDisplayedTexture != currentTexture)
	{
		// Load the new texture
		loadTexture(currentTexture);
		// Keep a record of which texture we are currently displaying
		// so we can detect when DGR changes currentTexture on a
		// slave.
		alreadyDisplayedTexture = currentTexture;
	}

	/* The view frustum is an orthographic frustum for this
	 * application. The size of the frustum doesn't matter much, but
	 * the aspect ratio of the frustum should match the aspect ratio
	 * of the screen/window. */
	float frustum[6], masterFrustum[6];
	
	/* The following two methods will get the master view frustum and
	 * the current process view frustum. If we are running in a
	 * standalone version, these two frustums will be the same. */
	viewmat_get_master_frustum(masterFrustum);
	viewmat_get_frustum(frustum, 0); // frustum of this process (master or slave)

	/* Set this view frustum for this process. */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(frustum[0],frustum[1],
	        frustum[2],frustum[3],
	        -1, 1);
	glMatrixMode(GL_MODELVIEW);

	// Middle of master frustum in the vertical direction. It divides the top
	// tiles from the bottom tiles.
	float masterFrustumMid = (masterFrustum[2]+masterFrustum[3])/2;
	// Dimensions of the master view frustum
	float masterFrustumWidth  = masterFrustum[1]-masterFrustum[0];
	float masterFrustumHeight = masterFrustum[3]-masterFrustum[2];

	// The width of the quad (in frustum units). Since the image will
	// be stretched to fit the screen vertically, and our units are in
	// frustum units, the width of the tile is the height of the
	// frustum times the aspect ratio divided by the number of tiles
	// in the horizontal direction.
	float quadWidth = aspectRatio * masterFrustumHeight;
	float tileWidth = quadWidth/numTiles;

// TODO: Maybe just scale the image vertically if the image almost fits in the screen horizontally?

	double SecSincePictureDisplayed = glfwGetTime()-lastAdvance;
	int scrollStatus = 0; // 0=don't need to scroll or done scrolling, 1=currently scrolling
	if(masterFrustumWidth < quadWidth)// do we need to scroll on this image
	{
		// Do we still need to scroll?
		if(scrollAmount < quadWidth-masterFrustumWidth)
			scrollStatus = 1;

		if(scrollStatus == 1)
		{
			// Wait a few seconds before scrolling. It takes a while
			// for all slaves on IVS to get the images.
			if(SecSincePictureDisplayed > 5)
				scrollAmount = ((SecSincePictureDisplayed-5) / SCROLL_SPEED)*masterFrustumWidth;
			else
				scrollAmount = 0;

			// If we calculated the scroll amount to be the largest
			// scrollAmount we'll need
			if(scrollAmount > quadWidth-masterFrustumWidth)
			{
				// dwell at the end of the image even if autoadvance is on.
				double now = glfwGetTime();
				// make sure we still have a few seconds before advancing
				if(SLIDESHOW_WAIT-(now-lastAdvance) < 3)
				{
					// Go back and set lastAdvance time so we have
					// some time to dwell here.
					lastAdvance = now-SLIDESHOW_WAIT+3;
				}
			}
		}
	}

	/* If autoadvance is set and we are not scrolling (or done
	 * scrolling) figure out if it is now time to advance to the next
	 * image. */
	if(autoAdvance == 1 && scrollStatus != 1)
	{
//		printf("time since last advance %d\n", glutGet(GLUT_ELAPSED_TIME)-lastAdvance);
		if(glfwGetTime()-lastAdvance > SLIDESHOW_WAIT) // time to show new image:
		{
			msg(MSG_INFO, "Automatically advancing to next image, please wait.\n");
			currentTexture = getNextTexture();
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1,1,1); // color of quad

	// Draw the top and bottom quad for each of the tiles going across the screen horizontally. */
	for(GLuint i=0; i<numTiles*2; i=i+2)
	{
		float tileLeft  = (i/2  )*tileWidth + masterFrustum[0];
		float tileRight = (i/2+1)*tileWidth + masterFrustum[0];
		
		// Draw bottom tile
		glBindTexture(GL_TEXTURE_2D, texNames[i]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2d(tileLeft -scrollAmount, masterFrustum[2]); // lower left
		glTexCoord2f(1.0, 0.0); glVertex2d(tileRight-scrollAmount, masterFrustum[2]); // lower right
		glTexCoord2f(1.0, 1.0); glVertex2d(tileRight-scrollAmount, masterFrustumMid); // upper right
		glTexCoord2f(0.0, 1.0); glVertex2d(tileLeft -scrollAmount, masterFrustumMid); // upper left
		glEnd();

		// Draw top tile
		glBindTexture(GL_TEXTURE_2D, texNames[i+1]);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2d(tileLeft -scrollAmount, masterFrustumMid); // lower left
		glTexCoord2f(1.0, 0.0); glVertex2d(tileRight-scrollAmount, masterFrustumMid); // lower right
		glTexCoord2f(1.0, 1.0); glVertex2d(tileRight-scrollAmount, masterFrustum[3]); // upper right
		glTexCoord2f(0.0, 1.0); glVertex2d(tileLeft -scrollAmount, masterFrustum[3]); // upper left
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);
	
#if 0
	/* Draw filename label on top of a quad. */
	glColor4f(0,0,0,.3);
	glBegin(GL_QUADS);
	glVertex2d(-1,-1);
	glVertex2d(-.5,-1);
	glVertex2d(-.5,-.96);
	glVertex2d(-1,-.96);
	glEnd();

	glColor4f(1,1,1,.9);
	glRasterPos2f(-.98,-.98);
	void *font = GLUT_BITMAP_TIMES_ROMAN_24;
	char *str = globalargv[currentTexture];
	for(GLuint i=0; i<strlen(str); i++)
		glutBitmapCharacter(font, str[i]);
#endif

	viewmat_end_eye(0);
	viewmat_end_frame();
}




/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action != GLFW_PRESS)
		return;
	
	// Ignore keys if DGR is enabled and we are not master
//	if(dgr_is_enabled() && !dgr_is_master())
//		return;
	switch(key)
	{
		case GLFW_KEY_Q:
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;

		case GLFW_KEY_N:
		case GLFW_KEY_PAGE_DOWN:
			msg(MSG_INFO, "Advancing to next image, please wait.\n");
		
			// if auto-advance, just force next picture by adjusting time the
			// picture was first displayed!
			if(autoAdvance)
				lastAdvance = 0;
			else
				currentTexture = getNextTexture();
			
			dgr_setget("currentTexture", &currentTexture, sizeof(int));
			break;

		case GLFW_KEY_B:
		case GLFW_KEY_P:
		case GLFW_KEY_PAGE_UP:
			msg(MSG_INFO, "Advancing to previous image...please wait...\n");

			// if auto-advance, just force next picture by adjusting time the
			// picture was first displayed!
			if(autoAdvance)
				lastAdvance = 0;
			else
				currentTexture = getPrevTexture();
			dgr_setget("currentTexture", &currentTexture, sizeof(int));
			break;

		case GLFW_KEY_S:
			if(autoAdvance == 1)
			{
				msg(MSG_INFO, "stopping auto-advance.\n");
				autoAdvance = 0;
			}
			else
			{
				msg(MSG_INFO, "starting auto-advance.\n");
				autoAdvance = 1;
			}
			break;
	}
}


int handle_directory(int argc, char** argv)
{
#ifdef __linux__
	char *dirLoc = kuhl_find_file(argv[1]);
	if(argc == 2 && dirLoc)
	{
		printf("Directory was passed as an argument.\n");

		// Scan the directory for files.
		struct dirent **dirList;
		int count = scandir(dirLoc, &dirList, NULL, alphasort);
		if(count == -1)
		{   // probably not a directory...
			printf("Maybe it wasn't a directory: %s\n", dirLoc);
			free(dirLoc);
			return 0;
		}

		// Create a list of files to use in place of argv. Note that
		// we put a dummy value in the first spot which would normally
		// correspond to the executable name.
		char **filenamePtrs = malloc(sizeof(char*)*count);
		totalTextures=0;
		for(int i=0; i<count; i++) // for each file
		{
			// concatenate directory name and the filename
			char dirPlusFile[1024];
			snprintf(dirPlusFile, 1024, "%s/%s", argv[1], dirList[i]->d_name);
			// Find the path to the file
			char *filename = kuhl_find_file(dirPlusFile);

			// Make sure it is a file.
			struct stat fileStat;
			if(stat(filename,&fileStat) < 0)
			{
				perror("stat");
				exit(EXIT_FAILURE);
			}

			if(S_ISREG(fileStat.st_mode) &&
			   (strcasecmp(filename+strlen(filename)-4, ".jpg") == 0 ||
			    strcasecmp(filename+strlen(filename)-4, ".png") == 0 ||
			    strcasecmp(filename+strlen(filename)-4, ".tif") == 0 ||
				strcasecmp(filename+strlen(filename)-5, ".tiff") == 0))
			{
				printf("Found image filename: %s\n", filename);
				*(filenamePtrs+totalTextures) = strdup(filename);
				totalTextures++;
			}
			free(filename);
		}
		
		// Free space
		for(int i=0; i<count; i++)
			free(dirList[i]);
		free(dirList);
		free(dirLoc);

		globalargv = filenamePtrs;
		// totalTextures is the actual number of textures, not counting the dummy entry in the first spot.
		totalTextures++;
		return 1;
	}
#endif
	return 0;
}

int main(int argc, char** argv)
{
	if(handle_directory(argc, argv) == 0)
	{
		// If we were passed one or more filenames, not directories
		totalTextures = argc-1;
		if(totalTextures == 0)
		{
			msg(MSG_FATAL, "Provide textures to use.\n");
			exit(EXIT_FAILURE);
		}
		globalargv = &(argv[1]);
	}

	kuhl_ogl_init(&argc, argv, 1152, 432, 20, 4); // same aspect ratio as IVS

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Initialize DGR
	dgr_init();
	
	float initCamPos[3]  = {0,0,10}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);

	loadTexture(currentTexture);

	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
