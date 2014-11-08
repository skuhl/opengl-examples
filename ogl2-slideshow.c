#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include "kuhl-util.h"
#include "imageio.h"
#include "dgr.h"
#include "projmat.h"


#define SCROLL_SPEED 30  // number of seconds to scroll past one screen-width of image.
#define MAX_TILES 100
#define SLIDESHOW_WAIT 10 // time in seconds to wait when autoadvance is turned on.
#define MAX_TILES 100

int autoAdvance = 0; // Automatically advance from image to image after a time out.
int lastAdvance = 0; // Time (in ms from when our program started) that we advanced picture.
float scrollAmount = 0; // How far has the image scrolled?

/* The current image that we are displaying. */
GLuint numTiles=0;
GLuint texNames[MAX_TILES];
float aspectRatio;

int alreadyDisplayedTexture = 1; // display() needs to reload currentTexture.
int currentTexture = 1; // first texture is 1 (corresponds to argv[1]
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
	imageio_info iioinfo;
	iioinfo.filename = filename;
	iioinfo.type = CharPixel;
	iioinfo.map = "RGBA";
	iioinfo.colorspace = sRGBColorspace;
	char *image = (char*) imagein(&iioinfo);

	if(image == NULL)
	{
		fprintf(stderr, "\n%s: Unable to read image.\n", filename);
		return -1;
	}

	/* "image" is a 1D array of characters (unsigned bytes) with four
	 * bytes for each pixel (red, green, blue, alpha). The data in "image"
	 * is in row major order. The first 4 bytes are the color information
	 * for the lowest left pixel in the texture. */
	int width  = (int)iioinfo.width;
	int height = (int)iioinfo.height;
	float original_aspectRatio = (float)width/height;
	if(verbose)
		printf("%s: Finished reading, dimensions are %dx%d\n", filename, width, height);

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
		printf("Source image must <= 8192 pixels tall.");
		exit(1);
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
		printf("Too many tiles");
		exit(1);
	}

	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA8, subimgW, subimgH,
	             0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	int tmp;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tmp);

	if(tmp == 0)
	{
		fprintf(stderr, "%s: File is too large (%d x %d). I can't load it!\n", filename, subimgW, subimgH);
		free(image);
		exit(1);
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
	if(next > totalTextures)
		return 1;
	return next;
}

int getPrevTexture()
{
	int next = currentTexture-1;
	if(next < 1)
		return totalTextures;
	return next;
}

void display(void);

/* Deletes any tiles that are already loaded and then loads up the
 * current texture image. */
void loadTexture(int textureIndex)
{
	if(numTiles > 0)
		glDeleteTextures(numTiles*2, texNames);

	scrollAmount = 0;
	aspectRatio = readfile(globalargv[textureIndex], texNames, &numTiles);
	lastAdvance = glutGet(GLUT_ELAPSED_TIME);
}

void display(void)
{
	kuhl_limitfps(100);
	dgr_update();
	// Make sure slaves get updates ASAP
	dgr_setget("currentTex", &currentTexture, sizeof(int));


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
	projmat_get_master_frustum(masterFrustum);
	projmat_get_frustum(frustum, -1, -1); // frustum of this process (master or slave)

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

	int msSincePictureDisplayed = glutGet(GLUT_ELAPSED_TIME)-lastAdvance;
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
			if(msSincePictureDisplayed > 5000)
				scrollAmount = ((msSincePictureDisplayed-5000) / (SCROLL_SPEED*1000.0))*masterFrustumWidth;
			else
				scrollAmount = 0;

			// If we calculated the scroll amount to be the largest
			// scrollAmount we'll need
			if(scrollAmount > quadWidth-masterFrustumWidth)
			{
				// dwell at the end of the image even if autoadvance is on.
				int now = glutGet(GLUT_ELAPSED_TIME);
				// make sure we still have a few seconds before advancing
				if(SLIDESHOW_WAIT*1000-(now-lastAdvance) < 3000)
				{
					// Go back and set lastAdvance time so we have
					// some time to dwell here.
					lastAdvance = now-SLIDESHOW_WAIT*1000+3000;
				}
			}
		}
	}

	dgr_setget("scrollAmount", &scrollAmount, sizeof(float));

	/* If autoadvance is set and we are not scrolling (or done
	 * scrolling) figure out if it is now time to advance to the next
	 * image. */
	if(autoAdvance == 1 && scrollStatus != 1)
	{
//		printf("time since last advance %d\n", glutGet(GLUT_ELAPSED_TIME)-lastAdvance);
		if(glutGet(GLUT_ELAPSED_TIME)-lastAdvance > SLIDESHOW_WAIT*1000) // time to show new image:
		{
			currentTexture = getNextTexture();
			loadTexture(currentTexture);
			return;
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

	/* Flush and swap the OpenGL buffers. */
	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}




void keyboard(unsigned char key, int x, int y)
{
	if (key == 'n' || key == ' ')
	{
		printf("Advancing to next image...please wait...\n");
		
		// If we press spacebar, advance to next image even if scrolling
		// didn't finish---so we will artificially add a lot of scroll so
		// display() thinks scrolling is done.
		scrollAmount = 1000;
		
		// if auto-advance, just force next picture by adjusting time the
		// picture was first displayed!
		if(autoAdvance)
			lastAdvance = 0;
		else
			currentTexture = getNextTexture();

		dgr_setget("currentTexture", &currentTexture, sizeof(int));
		dgr_update();
	}

	if (key == 'b' || key == 'p' || key == 73) // back or previous - 73=pageup
	{
		printf("Advancing to previous image...please wait...\n");
		// If we press spacebar, advance to next image even if scrolling
		// didn't finish---so we will artificially add a lot of scroll so
		// display() thinks scrolling is done.
		scrollAmount = 1000;

		// if auto-advance, just force next picture by adjusting time the
		// picture was first displayed!
		if(autoAdvance)
			lastAdvance = 0;
		else
			currentTexture = getPrevTexture();
		dgr_setget("currentTexture", &currentTexture, sizeof(int));
		dgr_update();
	}


	if (key == 27 || key == 'q')  // escape key, exit program
		exit(0);
	if(key == 's')
	{
		if(autoAdvance == 1)
		{
			printf("stopping auto-advance.\n");
			autoAdvance = 0;
		}
		else
		{
			printf("starting auto-advance.\n");
			autoAdvance = 1;

		}
	}
	glutPostRedisplay();
}

void special_keyboard(int key, int x, int y)
{
	if(key == GLUT_KEY_PAGE_DOWN)
		keyboard('n', 0,0);
	if(key == GLUT_KEY_PAGE_UP)
		keyboard('p', 0,0);
}

int main(int argc, char** argv)
{
	totalTextures = argc-1;
	if(totalTextures == 0)
	{
		printf("ERROR: Provide textures to use.\n");
		exit(EXIT_FAILURE);
	}
	globalargv = argv;

	/* set up our GLUT window */
	glutInit(&argc, argv);
	glutInitWindowSize(1152, 432); // same aspect ratio as IVS
	/* Ask GLUT to for a double buffered, full color window that
	 * includes a depth buffer */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow(argv[0]); // set window title to executable name

	/* Initialize GLEW */
	GLenum glewError = glewInit();
	if(glewError != GLEW_OK)
	{
		fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(glewError));
		exit(EXIT_FAILURE);
	}
	kuhl_errorcheck();

	// setup callbacks
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special_keyboard); // pageup,pagedown, etc

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Initialize DGR
	dgr_init();
	projmat_init();

	loadTexture(1);

	/* Tell GLUT to start running the main loop and to call display(),
	 * keyboard(), etc callback methods as needed. */
	glutMainLoop();
	exit(EXIT_SUCCESS);
}
