/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "kuhl-util.h"

float projmat_frustum[6];
float projmat_master_frustum[6];
float projmat_vfov = -1;
int projmat_mode = -1; /**< -1=undefined, 0=vfov, 1=frustum */


/** 
 Updates the size and position of the GLUT window based on environment variables.

 PROJMAT_WINDOW_SIZE - width and height of window.

 PROJMAT_WINDOW_POS - position of window on the screen.

 PROJMAT_FULLSCREEN - Sets the window to fill the screen.
*/
static void projmat_init_window()
{
	/* Change size of window if we were asked to. */
	const char* windowSizeString = getenv("PROJMAT_WINDOW_SIZE");
	int windowSize[2];
	if(windowSizeString != NULL &&
	   sscanf(windowSizeString, "%d %d", &(windowSize[0]), &(windowSize[1])) == 2)
	{
		kuhl_msg("Setting window size %d %d\n", windowSize[0], windowSize[1]);
		glutReshapeWindow(windowSize[0], windowSize[1]);
	}

	/* Change position of window if we were asked to. */
	const char* windowPosString = getenv("PROJMAT_WINDOW_POS");
	int windowPos[2];
	if(windowPosString != NULL &&
	   sscanf(windowPosString, "%d %d", &(windowPos[0]), &(windowPos[1])) == 2)
	{
		kuhl_msg("Setting window position %d %d\n", windowPos[0], windowPos[1]);
		glutPositionWindow(windowPos[0], windowPos[1]);
	}

	/* Change to fullscreen mode if we were asked to. */
	const char* fullscreenString = getenv("PROJMAT_FULLSCREEN");
	if(fullscreenString && strlen(fullscreenString) > 0)
	{
		kuhl_msg("Requesting fullscreen\n");
		glutFullScreen();
	}
}


/** Initialize projmat. This will apply any adjustments to the GLUT
 * window and then find a view frustum to use from the environment
 * variables. */
void projmat_init()
{
	projmat_init_window();

	const char* frustumString = getenv("PROJMAT_FRUSTUM");
	const char* masterFrustumString = getenv("PROJMAT_MASTER_FRUSTUM");

	if(frustumString != NULL)
	{
		projmat_mode = 1;
		if(sscanf(frustumString, "%f %f %f %f %f %f",
		          &(projmat_frustum[0]), &(projmat_frustum[1]), &(projmat_frustum[2]),
		          &(projmat_frustum[3]), &(projmat_frustum[4]), &(projmat_frustum[5])) != 6)
		{
			kuhl_errmsg("Unable to parse PROJMAT_FRUSTUM environment variable.\n");
			projmat_mode = -1;
		}

		/* Copy this frustum over to the master view frustum in case a
		 * master view frustum isn't set */
		for(int i=0; i<6; i++)
			projmat_master_frustum[i] = projmat_frustum[i];
		
		if(masterFrustumString == NULL ||
		   sscanf(masterFrustumString, "%f %f %f %f %f %f",
		          &(projmat_master_frustum[0]), &(projmat_master_frustum[1]), &(projmat_master_frustum[2]),
		          &(projmat_master_frustum[3]), &(projmat_master_frustum[4]), &(projmat_master_frustum[5])) != 6)
		{
			/* Copy this frustum over to the master view frustum in case a
			 * master view frustum isn't set */
			for(int i=0; i<6; i++)
				projmat_master_frustum[i] = projmat_frustum[i];

			kuhl_errmsg("Error parsing master frustum.\n");
		}

	}

	const char* vfovString = getenv("PROJMAT_VFOV");
	if(vfovString != NULL)
	{
		projmat_mode = 0;
		if(sscanf(vfovString, "%f", &projmat_vfov) != 1)
		{
			kuhl_errmsg("Unable to parse PROJMAT_VFOV environment variable.\n");
			projmat_vfov = -1;
		}
	}

	if(projmat_mode == -1)
		kuhl_msg("Using default perspective projection.\n");
	else if(projmat_mode == 0)
		kuhl_msg("Using a simple perspective projection (vfov=%f degrees).\n", projmat_vfov);
	else if(projmat_mode == 1)
		kuhl_msg("Using a view frustum.\n");
	else
	{
		kuhl_errmsg("projmat is confused.\n");
		exit(EXIT_FAILURE);
	}

}


/** Calculates a view frustum based on the current projmat
 * settings.
 *
 * @param result The location for view frustum values to be stored.
 *
 * @param viewportWidth The width of the viewport this frustum is
 * for. If viewportWidth is -1, it is assumed that he frustum will
 * fill the entire window. This option is useful for HMD rendering
 * where there are two viewports for a single window. The viewport
 * dimensions are necessary to calculate an appropriate aspect ratio
 * for the frustum.
 *
 * @param viewportHeight The height of the viewport this frustum is
 * for. If viewportHeight is -1, it is assumed that he frustum will
 * fill the entire window. This option is useful for HMD rendering
 * where there are two viewports for a single window. The viewport
 * dimensions are necessary to calculate an appropriate aspect ratio
 * for the frustum.
 **/
void projmat_get_frustum(float result[6], int viewportWidth, int viewportHeight)
{
	if(projmat_mode == -1 || projmat_mode == 0)
	{
		if(viewportWidth < 0)
			viewportWidth  = glutGet(GLUT_WINDOW_WIDTH);
		if(viewportHeight < 0)
			viewportHeight  = glutGet(GLUT_WINDOW_HEIGHT);
		float aspect = viewportWidth/(float)viewportHeight;
		float near = .1;
		float far = 30;
		float vfov = 65;
		if(projmat_mode == 0)
			vfov = projmat_vfov;
		float fovyRad = vfov * M_PI/180.0f;
		float height = near * tanf(fovyRad/2.0f);
		float width = height * aspect;
		result[0] = -width;
		result[1] = width;
		result[2] = -height;
		result[3] = height;
		result[4] = near;
		result[5] = far;

		// Save the frustum in proj_master_frustum and proj_frustum to
		// reduce any confusion. Since there are no slaves (which is
		// implied by not giving a frustum or specifying a simple
		// vertical field of view), then the master frustum matches
		// this frustum.
		for(int i=0; i<6; i++)
		{
			projmat_master_frustum[i] = result[i];
			projmat_frustum[i] = result[i];
		}
		
		return;
	}

	if(projmat_mode == 1) // we were given a view frustum
	{
		for(int i=0; i<6; i++)
			result[i] = projmat_frustum[i];
		return;
	}
}

/** If you are using PROJMAT_FRUSTUM with the display wall, some slave
 * applications might want to have access to the master process view
 * frustum. If there is no master process, the master frustum will
 * match this process's view frustum.
 *
 * @param result An array to be filled in with frustum values.
 */
void projmat_get_master_frustum(float result[6])
{
	for(int i=0; i<6; i++)
		result[i] = projmat_master_frustum[i];
}
