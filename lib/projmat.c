/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#ifdef FREEGLUT
#include <GL/freeglut.h>
#else
#include <GLUT/glut.h>
#endif
#include "kuhl-util.h"
#include "viewmat.h"

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
		msg(INFO, "Setting window size %d %d\n", windowSize[0], windowSize[1]);
		glutReshapeWindow(windowSize[0], windowSize[1]);
	}

	/* Change position of window if we were asked to. */
	const char* windowPosString = getenv("PROJMAT_WINDOW_POS");
	int windowPos[2];
	if(windowPosString != NULL &&
	   sscanf(windowPosString, "%d %d", &(windowPos[0]), &(windowPos[1])) == 2)
	{
		msg(INFO, "Setting window position %d %d\n", windowPos[0], windowPos[1]);
		glutPositionWindow(windowPos[0], windowPos[1]);
	}

	/* Change to fullscreen mode if we were asked to. */
	const char* fullscreenString = getenv("PROJMAT_FULLSCREEN");
	if(fullscreenString && strlen(fullscreenString) > 0)
	{
		msg(INFO, "Requesting fullscreen\n");
		glutFullScreen();
	}
}


/** Initialize projmat. This will apply any adjustments to the GLUT
 * window and then find a view frustum to use from the environment
 * variables. */
void projmat_init()
{
	projmat_init_window();

	int foundFrustum = 0;
	int foundMasterFrustum = 0;
	int foundFov = 0;

	const char* frustumString = getenv("PROJMAT_FRUSTUM");
	const char* masterFrustumString = getenv("PROJMAT_MASTER_FRUSTUM");
	
	if(frustumString != NULL)
	{
		if(sscanf(frustumString, "%f %f %f %f %f %f",
		          &(projmat_frustum[0]), &(projmat_frustum[1]), &(projmat_frustum[2]),
		          &(projmat_frustum[3]), &(projmat_frustum[4]), &(projmat_frustum[5])) != 6)
			msg(ERROR, "Unable to parse PROJMAT_FRUSTUM environment variable.\n");
		else
			foundFrustum = 1;
	}

	if(masterFrustumString != NULL)
	{
		if(sscanf(masterFrustumString, "%f %f %f %f %f %f",
		          &(projmat_master_frustum[0]), &(projmat_master_frustum[1]), &(projmat_master_frustum[2]),
		          &(projmat_master_frustum[3]), &(projmat_master_frustum[4]), &(projmat_master_frustum[5])) != 6)
			msg(ERROR, "Unable to parse PROJMAT_MASTER_FRUSTUM environment variable.\n");
		else
			foundMasterFrustum = 1;
	}
	
	const char* vfovString = getenv("PROJMAT_VFOV");
	if(vfovString != NULL)
	{
		if(sscanf(vfovString, "%f", &projmat_vfov) != 1)
			msg(ERROR, "Unable to parse PROJMAT_VFOV environment variable.\n");
		else
			foundFov = 1;
	}

	if(foundFov == 1)
	{
		projmat_mode = 0;
		msg(INFO, "Using a simple perspective projection (vfov=%f degrees).\n", projmat_vfov);
	}
	else if(foundMasterFrustum == 1 && foundFrustum == 1)
	{
		projmat_mode = 1;
		msg(INFO, "Using custom view frustum.\n");
	}
	else if(foundMasterFrustum == 0 && foundFrustum == 1)
	{
		projmat_mode = 1;
		// If running on a multi-computer cluster, environment
		// variables must specify the overall frustum and the
		// individual screen frustum.
		msg(WARNING, "PROJMAT_FRUSTUM was defined but PROJMAT_MASTER_FRUSTUM was not.");
		msg(WARNING, "Assuming that the two frustums are the same (should work if running on a single machine).");
		for(int i=0; i<6; i++)
			projmat_master_frustum[i] = projmat_frustum[i];
	}
	else if(foundMasterFrustum == 1 && foundFrustum == 0)
	{
		projmat_mode = 1;
		// If running on a multi-computer cluster, environment
		// variables must specify the overall frustum and the
		// individual screen frustum.
		msg(WARNING, "PROJMAT_MASTER_FRUSTUM was defined but PROJMAT_FRUSTUM was not.");
		msg(WARNING, "Assuming that the two frustums are the same (should work if running on a single machine).");
		for(int i=0; i<6; i++)
			projmat_frustum[i] = projmat_master_frustum[i];
	}
	else
	{
		projmat_mode = -1;
		msg(INFO, "Using default perspective projection.\n");
	}

	// If the frustum was defined via environment variables, print it
	// out. If no frustum was specified, the actual projection frustum
	// depends on the size of the window---so we can't print out the
	// frustum values here.
	if(projmat_mode == 1)
	{
		msg(INFO, "View frustum: left=%f right=%f bot=%f top=%f near=%f far=%f\n",
		    projmat_frustum[0], projmat_frustum[1], projmat_frustum[2],
		    projmat_frustum[3], projmat_frustum[4], projmat_frustum[5]);
		if(projmat_frustum[4] < 0 || projmat_frustum[5] < 0)
			msg(WARNING, "The near and far values in the frustum should be positive (i.e., this matches the behavior of the old OpenGL glFrustum() function call.)");
	}

}


/** Calculates a view frustum based on the current projmat
 * settings.
 *
 * @param result The location for view frustum values to be stored.
 *
 * @param viewportWidth The width of the viewport this frustum is
 * for. If viewportWidth is -1, it is assumed that the frustum will
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
		int windowWidth, windowHeight;
		viewmat_window_size(&windowWidth, &windowHeight);
		if(viewportWidth < 0)
			viewportWidth  = windowWidth;
		if(viewportHeight < 0)
			viewportHeight  = windowHeight;
		float aspect = viewportWidth/(float)viewportHeight;
		float nearPlane = 0.1;
		float farPlane = 200;
		float vfov = 65;
		if(projmat_mode == 0)
			vfov = projmat_vfov;
		float fovyRad = vfov * M_PI/180.0f;
		float height = nearPlane * tanf(fovyRad/2.0f);
		float width = height * aspect;
		result[0] = -width;
		result[1] = width;
		result[2] = -height;
		result[3] = height;
		result[4] = nearPlane;
		result[5] = farPlane;

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
