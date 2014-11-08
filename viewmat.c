/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include <stdlib.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "kuhl-util.h"
#include "mousemove.h"
#include "vrpn-help.h"
#include "dgr.h"


/** The different modes that viewmat works with. */
typedef enum
{
	VIEWMAT_MOUSE,
	VIEWMAT_IVS,
	VIEWMAT_HMD,
	VIEWMAT_NONE
} ViewmatModeType;

#define MAX_VIEWPORTS 32 /**< Hard-coded maximum number of viewports supported. */
static float viewports[MAX_VIEWPORTS][4]; /**< Contains one or more viewports. The values are the x coordinate, y coordinate, viewport width, and viewport height */
static int viewports_size = 0; /**< Number of viewports in viewports array */
static ViewmatModeType viewmat_mode = 0; /**< 0=mousemove, 1=IVS (using VRPN), 2=HMD (using VRPN), 3=none */
static const char *viewmat_vrpn_obj; /**< Name of the VRPN object that we are tracking */


/** Sets up viewmat to only have one viewport. This can be called
 * every frame since resizing the window will change the size of the
 * viewport! */
static void viewmat_one_viewport()
{
	/* One viewport fills the entire screen */
	int windowWidth  = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
	viewports_size = 1;
	viewports[0][0] = 0;
	viewports[0][1] = 0;
	viewports[0][2] = windowWidth;
	viewports[0][3] = windowHeight;
}

/** Sets up viewmat to split the screen vertically into two
 * viewports. This can be called every frame since resizing the window
 * will change the size of the viewport! */
static void viewmat_two_viewports()
{
	/* Two viewports, one for each eye */
	int windowWidth  = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

	/* TODO: Figure out if it makes sense to make this configurable at runtime. */
	viewports_size = 2;
	viewports[0][0] = 0;
	viewports[0][1] = 0;
	viewports[0][2] = windowWidth/2;
	viewports[0][3] = windowHeight;

	viewports[1][0] = windowWidth/2;
	viewports[1][1] = 0;
	viewports[1][2] = windowWidth/2;
	viewports[1][3] = windowHeight;
}

/** This method should be called regularly to ensure that we adjust
 * our viewports after a window is resized. */
static void viewmat_refresh_viewports()
{
	if(viewmat_mode == VIEWMAT_MOUSE ||
	   viewmat_mode == VIEWMAT_NONE ||
	   viewmat_mode == VIEWMAT_IVS)
		viewmat_one_viewport();
	else if(viewmat_mode == VIEWMAT_HMD)
		viewmat_two_viewports();
	else
	{
		printf("viewmat: unknown mode: %d\n", viewmat_mode);
		exit(EXIT_FAILURE);
	}
}

/** Initialize IVS view matrix calculations, connect to VRPN. */
void viewmat_init_ivs()
{
	/* Since the master process is the only one that talks to the VRPN
	 * server, slaves don't need to do anything to initialize. */
	if(dgr_is_master() == 0)
		return;
	
	const char* vrpnObjString = getenv("VIEWMAT_VRPN_OBJECT");
	if(vrpnObjString != NULL && strlen(vrpnObjString) > 0)
	{
		viewmat_vrpn_obj = vrpnObjString;
		printf("viewmat: View is following tracker object: %s\n", viewmat_vrpn_obj);
		
		/* Try to connect to VRPN server */
		float vrpnPos[3];
		float vrpnOrient[16];
		vrpn_get(viewmat_vrpn_obj, NULL, vrpnPos, vrpnOrient);
	}
	else
	{
		fprintf(stderr, "viewmat: Failed to setup IVS mode\n");
		exit(EXIT_FAILURE);
	}
}


/** Initialize mouse movement. */
static void viewmat_init_mouse(float pos[3], float look[3], float up[3])
{
	glutMotionFunc(mousemove_glutMotionFunc);
	glutMouseFunc(mousemove_glutMouseFunc);
	mousemove_set(pos[0],pos[1],pos[2],
	              look[0],look[1],look[2],
	              up[0],up[1],up[2]);
	mousemove_speed(0.05, 0.5);
}


static void viewmat_init_hmd(float pos[3], float look[3], float up[3])
{
	/* TODO: For now, we are using mouse movement for the HMD mode */
	glutMotionFunc(mousemove_glutMotionFunc);
	glutMouseFunc(mousemove_glutMouseFunc);
	mousemove_set(pos[0],pos[1],pos[2],
	              look[0],look[1],look[2],
	              up[0],up[1],up[2]);
	mousemove_speed(0.05, 0.5);
}


/** Initialize viewmat and use the specified pos/look/up values as a
 * starting location when mouse movement is used.
 *
 * @param pos The position of the camera (if mouse movement is used)
 * @param look A point the camera is looking at (if mouse movement is used)
 * @param up An up vector for the camera (if mouse movement is used).
 */
void viewmat_init(float pos[3], float look[3], float up[3])
{
	const char* modeString = getenv("VIEWMAT_MODE");
	if(modeString == NULL)
		modeString = "mouse";
	
	if(strcasecmp(modeString, "ivs") == 0)
	{
		viewmat_mode = VIEWMAT_IVS;
		viewmat_init_ivs();
		printf("viewmat: Using IVS head tracking mode, tracking object: %s\n", viewmat_vrpn_obj);
	}
	else if(strcasecmp(modeString, "hmd") == 0)
	{
		viewmat_mode = VIEWMAT_HMD;
		viewmat_init_hmd(pos, look, up);
		printf("viewmat: Using HMD head tracking mode, NOT COMPLETED. Tracking object: %s\n", viewmat_vrpn_obj);
	}
	else if(strcasecmp(modeString, "none") == 0)
	{
		printf("viewmat: No view matrix handler is being used.\n");
		viewmat_mode = VIEWMAT_NONE;
	}
	else
	{
		if(strcasecmp(modeString, "mouse") == 0) // if no mode specified, default to mouse
			printf("viewmat: Using mouse movement.\n");
		else // if an unrecognized mode was specified.
			fprintf(stderr, "viewmat: Unrecognized VIEWMAT_MODE: %s; using mouse movement instead.\n", modeString);
		viewmat_mode = VIEWMAT_MOUSE;
		viewmat_init_mouse(pos, look, up);
	}

	viewmat_refresh_viewports();
}

void viewmat_get_hmd(float viewmatrix[16], int viewportNum)
{
	float pos[3],look[3],up[3];
	mousemove_get(pos, look, up);

	float eyeDist = 0.055;  // TODO: Make this configurable.

	float lookVec[3], rightVec[3];
	vec3f_sub_new(lookVec, look, pos);
	vec3f_normalize(lookVec);
	vec3f_cross_new(rightVec, lookVec, up);
	vec3f_normalize(rightVec);
	if(viewportNum == 0)
		vec3f_scalarMult(rightVec, -eyeDist/2.0);
	else
		vec3f_scalarMult(rightVec, eyeDist/2.0);

	vec3f_add_new(look, look, rightVec);
	vec3f_add_new(pos, pos, rightVec);

	mat4f_lookatVec_new(viewmatrix, pos, look, up);
	// Don't need to use DGR!
}




/** Get a view matrix from mousemove.

 @param viewmatrix The location where the viewmatrix should be stored.
*/
void viewmat_get_mouse(float viewmatrix[16])
{
	float pos[3],look[3],up[3];
	mousemove_get(pos, look, up);
	mat4f_lookatVec_new(viewmatrix, pos, look, up);
	dgr_setget("!!viewMatMouse", viewmatrix, sizeof(float)*16);
}

/** Get a view matrix from VRPN and adjust the view frustum
 * appropriately.
 * @param viewmatrix The location where the viewmatrix should be stored.
 * @param frustum The location of the view frustum that should be adjusted.
 */
void viewmat_get_ivs(float viewmatrix[16], float frustum[6])
{
	float pos[3];
	if((dgr_is_enabled() && dgr_is_master()) || dgr_is_enabled()==0)
	{
		/* get information from vrpn */
		float orient[16];
		vrpn_get(viewmat_vrpn_obj, NULL, pos, orient);
	}
	/* Make sure all DGR hosts can get the position so that they
	 * can update the frustum appropriately */
	dgr_setget("!!viewMatPos", pos, sizeof(float)*3);

	/* Update view frustum if it was provided. */
	if(frustum != NULL)
	{
		frustum[0] -= pos[0];
		frustum[1] -= pos[0];
		frustum[2] -= pos[1];
		frustum[3] -= pos[1];
		frustum[4] += pos[2];
		frustum[5] += pos[2];
	}

	/* calculate a lookat point */
	float lookat[3];
	float forwardVec[3] = { 0, 0, -1 };
	for(int i=0; i<3; i++)
		lookat[i] = pos[i]+forwardVec[i];
		
	float up[3] = {0, 1, 0};
	mat4f_lookatVec_new(viewmatrix, pos, lookat, up);

}

/** Get a 4x4 view matrix. Some types of systems also need to update
 * the frustum based on where the virtual camera is. For example, on
 * the IVS display wall, the frustum is adjusted dynamically based on
 * where a person is relative to the screens.
 *
 * @param viewportNum If there is only one viewport, set this to 0. If
 * the system uses multiple viewports (i.e., HMD), then set this to 0
 * or 1 to get the view matrices for the left and right eyes.
 *
 * @param viewmatrix A 4x4 view matrix for viewmat to fill in.
 *
 * @param frustum View frustum values to be adjusted by viewmat if
 * needed. You can also set frustum to NULL if you don't want to use
 * dynamic frustums.
 */
void viewmat_get(float viewmatrix[16], float frustum[6], int viewportNum)
{
	if(viewmat_mode == VIEWMAT_MOUSE) // mouse movement
		viewmat_get_mouse(viewmatrix);

	if(viewmat_mode == VIEWMAT_IVS) // IVS
		viewmat_get_ivs(viewmatrix, frustum);

	if(viewmat_mode == VIEWMAT_HMD) // HMD
		viewmat_get_hmd(viewmatrix, viewportNum);
}

/** Gets the viewport information for a particular viewport.

 @param viewportValue A location to be filled in with the viewport x
 coordinate, y coordinate, width and height.

 @param viewportNum Which viewport number is being requested. If you
 are using only one viewport, set this to 0.
*/
void viewmat_get_viewport(int viewportValue[4], int viewportNum)
{
	viewmat_refresh_viewports();
	
	if(viewportNum >= viewports_size)
	{
		printf("viewmat: You requested a viewport that does not exist.\n");
		exit(EXIT_FAILURE);
	}

	/* Copy the viewport into the location the caller provided. */
	for(int i=0; i<4; i++)
		viewportValue[i] = viewports[viewportNum][i];

}

/** Returns the number of viewports that viewmat has.

    @return The number of viewports that viewmat has.
*/
int viewmat_num_viewports()
{
	viewmat_refresh_viewports();
	return viewports_size;
}
