/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include "windows-compat.h"
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "dispmode-desktop.h"
#include "dispmode-hmd.h"
#include "dispmode-anaglyph.h"
#include "dispmode-frustum.h"
#include "dispmode-oculus-linux.h"
#include "dispmode-oculus-windows.h"

#include "camcontrol-mouse.h"
#include "camcontrol-vrpn.h"
#include "camcontrol-orientsensor.h"
#include "camcontrol-oculus-linux.h"
#include "camcontrol-oculus-windows.h"


#include "kuhl-util.h"
#include "vecmat.h"
#include "mousemove.h"
#include "vrpn-help.h"
#include "orient-sensor.h"
#include "dgr.h"
#include "bufferswap.h"

#include "viewmat.h"

dispmode *desktop;
camcontrol *controller;


/** The display mode specifies how images are drawn to the screen. */
typedef enum
{
	VIEWMAT_DESKTOP,    /* Single window */
	VIEWMAT_IVS,        /* Michigan Tech's Immersive Visualization Studio */
	VIEWMAT_HMD,        /* Side-by-side view */
	VIEWMAT_OCULUS,     /* HMDs supported by libovr (Oculus DK1, DK2, etc). */
	VIEWMAT_ANAGLYPH,   /* Red-Cyan anaglyph images */
} ViewmatDisplayMode;
static ViewmatDisplayMode viewmat_display_mode = VIEWMAT_DESKTOP; /**< Currently active display mode */

/** The control mode specifies how the viewpoint position/orientation is determined. */
typedef enum
{
	VIEWMAT_CONTROL_NONE,
	VIEWMAT_CONTROL_MOUSE,
	VIEWMAT_CONTROL_VRPN,
	VIEWMAT_CONTROL_ORIENT,
	VIEWMAT_CONTROL_OCULUS
} ViewmatControlMode;
static ViewmatControlMode viewmat_control_mode = VIEWMAT_CONTROL_MOUSE; /**< Currently active control mode */



/** TODO: Update for switch to GLFW:

 * Sometimes calls to glutGet(GLUT_WINDOW_*) take several milliseconds
 * to complete. To maintain a 60fps frame rate, we have a budget of
 * about 16 milliseconds per frame. These functions might get called
 * multiple times in multiple places per frame. viewmat_window_size()
 * is an alternative way to get the window size but it may be up to 1
 * second out of date.
 *
 * This causes window resizing to look a little ugly, but it is
 * functional and results in a more consistent framerate.
 *
 * @param width To be filled in with the width of the GLUT window.
 *
 * @param height To be filled in with the height of the GLUT window.
 */
void viewmat_window_size(int *width, int *height)
{
	if(width == NULL || height == NULL)
	{
		msg(MSG_ERROR, "width and/or height pointers were null.");
		exit(EXIT_FAILURE);
	}
	
	// Initialize static variables upon startup
	static int savedWidth  = -1;
	static int savedHeight = -1;
	static long savedTime = -1;

	// If it is the first time we are called and the variables are not
	// initialized, initialize them!
	int needToUpdate = 0;
	if(savedWidth < 0 || savedHeight < 0 || savedTime < 0)
		needToUpdate = 1;
	else if(kuhl_milliseconds() - savedTime > 1000)
		needToUpdate = 1;

	if(needToUpdate)
	{
		glfwGetFramebufferSize(kuhl_get_window(), &savedWidth, &savedHeight);
		//savedWidth  = glutGet(GLUT_WINDOW_WIDTH);
		//savedHeight = glutGet(GLUT_WINDOW_HEIGHT);
		savedTime = kuhl_milliseconds();
		// msg(MSG_INFO, "Updated window size\n");
	}

	*width = savedWidth;
	*height = savedHeight;
}

/** Returns the aspect ratio of the current GLFW window. */
float viewmat_window_aspect_ratio(void)
{
	int w,h;
	viewmat_window_size(&w, &h);
	return w/(float)h;
}



/** Should be called prior to rendering a frame. */
void viewmat_begin_frame(void)
{
	desktop->begin_frame();
}


/** Should be called when we have completed rendering a frame. For
 * HMDs, this should be called after both the left and right eyes have
 * been rendered. */
void viewmat_end_frame(void)
{
	desktop->end_frame();
}


/** Changes the framebuffer (as needed) that OpenGL is rendering
 * to. Some HMDs (such as the Oculus Rift) require us to prerender the
 * left and right eye scenes to a texture. Those textures are then
 * processed and displayed on the screen. This function should be
 * called before any drawing occurs for that particular eye. Once both
 * eyes are rendered, viewmat_end_frame() will reset the bound
 * framebuffer and render the scene.
 *
 * @param viewportID The viewport number that we are rendering to. If
 * running as a single window, non-stereo desktop application, use
 * 0.
 */
void viewmat_begin_eye(int viewportID)
{
	desktop->begin_eye(viewportID);
}

void viewmat_end_eye(int viewportID)
{
	desktop->end_eye(viewportID);
}






/** Initialize viewmat and use the specified pos/look/up values as a
 * starting location when mouse movement is used.
 *
 * @param pos The position of the camera (if mouse movement is used)
 * @param look A point the camera is looking at (if mouse movement is used)
 * @param up An up vector for the camera (if mouse movement is used).
 */
void viewmat_init(const float pos[3], const float look[3], const float up[3])
{
	const char* controlModeString = kuhl_config_get("viewmat.controlmode");
	if(controlModeString == NULL)
		controlModeString = "mouse";
	const char* displayModeString = kuhl_config_get("viewmat.displaymode");
	if(displayModeString == NULL)
		displayModeString = "none";

	/* Make an intelligent guess if unspecified */
	if(controlModeString == NULL) 
	{
		if(kuhl_config_get("orientsensor.tty") != NULL &&
		   kuhl_config_get("orientsensor.type") != NULL)
		{
			msg(MSG_INFO, "viewmat control Mode: Unspecified, but using orientation sensor.");
			controlModeString = "orient";
		}
		else if(kuhl_config_get("viewmat.vrpn.object") != NULL)
		{
			msg(MSG_INFO, "viewmat control Mode: Unspecified, but using VRPN because a tracked object (%s) was specified.", kuhl_config_get("viewmat.vrpn.object"));
			controlModeString = "vrpn";
		}
		else
			controlModeString = "mouse";
	}

	if(dgr_is_enabled() == 1 && dgr_is_master() == 0)
	{
		msg(MSG_INFO, "Using no control mode because we are a slave.");
		controlModeString = "none";
	}

	/* Set viewmat_control_mode variable appropriately. */
	static const char *controlStrings[] = { "none", "mouse", "vrpn", "orient", "oculus" };
	static const ViewmatControlMode controlTypes[]    = { VIEWMAT_CONTROL_NONE, VIEWMAT_CONTROL_MOUSE, VIEWMAT_CONTROL_VRPN, VIEWMAT_CONTROL_ORIENT, VIEWMAT_CONTROL_OCULUS };
	for(int i=0; i<5; i++)
		if(strcasecmp(controlModeString, controlStrings[i]) == 0)
			viewmat_control_mode = controlTypes[i];

	/* Set viewmat_display_mode variable appropraitely */
	static const char *displayStrings[] = { "none", "ivs", "oculus", "hmd", "anaglyph" };
	static const ViewmatDisplayMode displayTypes[] = { VIEWMAT_DESKTOP, VIEWMAT_IVS, VIEWMAT_OCULUS, VIEWMAT_HMD, VIEWMAT_ANAGLYPH };
	for(int i=0; i<5; i++)
		if(strcasecmp(displayModeString, displayStrings[i]) == 0)
			viewmat_display_mode = displayTypes[i];

	/* We can't use the Oculus orientation sensor if we haven't
	 * initialized the Oculus code. We do that initialization in the
	 * Oculus display mode code. */
	if(viewmat_control_mode == VIEWMAT_CONTROL_OCULUS &&
	   viewmat_display_mode != VIEWMAT_OCULUS)
	{
		msg(MSG_FATAL, "viewmat: Oculus can only be used as a control mode if it is also used as a display mode.");
		exit(EXIT_FAILURE);
	}


	switch(viewmat_display_mode)
	{
		case VIEWMAT_DESKTOP:
			msg(MSG_INFO, "viewmat display mode: Single window desktop mode.\n");
			desktop = new dispmodeDesktop();
			break;
		case VIEWMAT_IVS:
			msg(MSG_INFO, "viewmat display mode: IVS");
			desktop = new dispmodeFrustum();
			break;
		case VIEWMAT_OCULUS:
#if defined(__linux__) && !defined(MISSING_OVR)
			msg(MSG_INFO, "viewmat display mode: Oculus HMD (Linux).");
			desktop = new dispmodeOculusLinux();
#elif defined(_WIN32) && !defined(MISSING_OVR)
			msg(MSG_INFO, "viewmat display mode: Oculus HMD (Windows).");
			desktop = new dispmodeOculusWindows();
#else
			msg(MSG_FATAL, "Oculus not supported on this platform.");
			exit(EXIT_FAILURE);
#endif
			break;
		case VIEWMAT_HMD:
			desktop = new dispmodeHMD();
			msg(MSG_INFO, "viewmat display mode: Side-by-side left/right view.\n");
			break;
			
		case VIEWMAT_ANAGLYPH:
			desktop = new dispmodeAnaglyph();
			msg(MSG_INFO, "viewmat display mode: Anaglyph image rendering. Use the red filter on the left eye and the cyan filter on the right eye.\n");
			break;

		default:
			msg(MSG_FATAL, "viewmat display mode: unhandled mode '%s'.", displayModeString);
			exit(EXIT_FAILURE);
	}
	
	switch(viewmat_control_mode)
	{
		case VIEWMAT_CONTROL_NONE:
			msg(MSG_INFO, "viewmat control mode: None (fixed view)");
			controller = new camcontrol(desktop, pos, look, up);
			break;
		case VIEWMAT_CONTROL_MOUSE:
			msg(MSG_INFO, "viewmat control mode: Mouse movement");
			controller = new camcontrolMouse(desktop, pos, look, up);
			break;
		case VIEWMAT_CONTROL_VRPN:
			msg(MSG_INFO, "viewmat control mode: VRPN");
			controller = new camcontrolVrpn(desktop, kuhl_config_get("viewmat.vrpn.object"), NULL);
			break;
		case VIEWMAT_CONTROL_ORIENT:
			msg(MSG_INFO, "viewmat control mode: Orientation sensor");
			controller = new camcontrolOrientSensor(desktop, pos);
			break;
		case VIEWMAT_CONTROL_OCULUS:
#if defined(__linux__) && !defined(MISSING_OVR)
			msg(MSG_INFO, "viewmat control mode: Oculus (Linux)");
			controller = new camcontrolOculusLinux(desktop, pos);
#elif defined(_WIN32) && !defined(MISSING_OVR)
			msg(MSG_INFO, "viewmat control mode: Oculus (Windows)");
			controller = new camcontrolOculusWindows(desktop, pos);
#else
			msg(MSG_FATAL, "Oculus not supported on this platform.");
			exit(EXIT_FAILURE);
#endif
			break;
		default:
			msg(MSG_FATAL, "viewmat control mode: Unhandled mode '%s'.", controlModeString);
			exit(EXIT_FAILURE);
	}
}


/** Performs a sanity check on the IPD to ensure that it is not too small, big, or reversed.

 @param viewmatrix View matrix for the viewportID
 @param viewportID The viewportID for this particular view matrix.
*/
static void viewmat_validate_ipd(const float viewmatrix[16], int viewportID)
{
	if(desktop->num_viewports() != 2)
		return;
	
	// First, if viewportID=0, save the matrix so we can do the check when we are called with viewportID=1.
	static float viewmatrix0[16];
	static long viewmatrix0time;
	if(viewportID == 0)
	{
		mat4f_copy(viewmatrix0, viewmatrix);
		viewmatrix0time = kuhl_microseconds();
		return;
	}

	// If rendering viewportID == 1, and if there are two viewports,
	// assume that we are running in a stereoscopic configuration and
	// validate the IPD value.
	if(viewportID == 1)
	{
		float flip = 1;
		/* In most cases, viewportID=0 is the left eye. However,
		 * Oculus may cause this to get swapped. */
		if(desktop->eye_type(0) == VIEWMAT_EYE_RIGHT)
			flip = -1;

		// The difference between the last columns of the two matrices
		// will tell us the IPD. To understand why this is, you could
		// take a view matrix and apply an additional translation in
		// the X direction (in camera coordinates) by simply adding a
		// value to the first value in the last column.
		float pos1[4], pos2[4];
		mat4f_getColumn(pos1, viewmatrix0, 3); // get last column
		mat4f_getColumn(pos2, viewmatrix,  3); // get last column

		/* For future reference: If we really wanted to extract the
		   camera position in world coordinates from the view
		   matrices, we would need to invert the matrices before
		   getting the last column. However, then we'd have to
		   calculate the distance between the two points instead of
		   just looking at the difference in the 'x' value. */

		// Get a vector between the eyes
		float diff[4];
		vec4f_sub_new(diff, pos1, pos2);
		vec4f_scalarMult_new(diff, diff, flip); // flip vector if necessary

		/* This message may be triggered if a person is moving quickly
		 * and/or when the FPS is low. This happens because the
		 * position/orientation of the head changed between the
		 * rendering of the left and right eyes. */
		float ipd = diff[0];
		long delay = kuhl_microseconds() - viewmatrix0time;
		if(ipd > .07 || ipd < .05)
		{
			msg(MSG_WARNING, "IPD=%.4f meters, delay=%ld us (IPD validation failed; occasional messages are OK!)\n", ipd, delay);
		}

#if 0
		// Debugging
		msg(MSG_INFO, "IPD=%.4f meters, delay=%ld us\n", ipd, delay);
		msg(MSG_INFO, "pos1=%0.3f %0.3f %0.3f", pos1[0], pos1[1], pos1[2]);
		msg(MSG_INFO, "pos2=%0.3f %0.3f %0.3f", pos2[0], pos2[1], pos2[2]);
#endif
	}
}


/** Get a 4x4 view matrix. Some types of systems also need to update
 * the frustum based on where the virtual camera is. For example, on
 * the IVS display wall, the frustum is adjusted dynamically based on
 * where a person is relative to the screens.
 *
 * @param viewmatrix A 4x4 view matrix for viewmat to fill in.
 *
 * @param projmatrix A 4x4 projection matrix for viewmat to fill in.
 *
 * @param viewportID If there is only one viewport, set this to
 * 0. This value must be smaller than the value reported by
 * viewmat_num_viewports(). In an HMD, typically viewportID=0 is the
 * left eye and viewportID=1 is the right eye. However, some Oculus
 * HMDs will result in this being swapped. To definitively know which
 * eye this view matrix corresponds to, examine the return value of
 * this function. If viewportID == -1, then this function will return
 * a value appropriate for a "middle" eye regardless of rendering
 * mode. This is useful if you are rendering for an HMD but you
 * actually want to know the position of the point between the center
 * of the eyes.
 *
 * @return A viewmat_eye enum which indicates if this view matrix is
 * for the left, right, middle, or unknown eye.
 *
 */
viewmat_eye viewmat_get(float viewmatrix[16], float projmatrix[16], int viewportID)
{
	viewmat_eye eye;
	if(viewportID == -1)
		eye = VIEWMAT_EYE_MIDDLE;
	else
		eye = desktop->eye_type(viewportID);
	
	
	int viewport[4]; // x,y of lower left corner, width, height
	desktop->get_viewport(viewport, viewportID);

	/* Get the current projection matrix. */
	desktop->get_projmatrix(projmatrix, viewportID);

	/* Get the current view matrix.
	 * 
	 * NOTE: There is no reason to get the view matrix if DGR is
	 * enabled and we are a slave because the master process will
	 * control the viewmatrix. */
	controller->get(viewmatrix, desktop->eye_type(viewportID));
	
	/* If we are running in IVS mode and using the tracking systems,
	 * all computers need to update their frustum differently. The
	 * master process will be controlled by VRPN, and all slaves will
	 * have their controllers set to "none". Here, we detect for this
	 * situation and make sure all processes work correctly.
	 */
	if(viewmat_display_mode == VIEWMAT_IVS &&
	   viewmat_control_mode == VIEWMAT_CONTROL_VRPN)
	{
		if(desktop->provides_projmat_only())
		{
			msg(MSG_ERROR, "The current dispmode only provides a projection matrix---but we need a view frustum to implement a dynamic frustum.");
		}
		else
		{
			// we need the frustum, not the projection matrix we
			// already retreived above.
			float frustum[6];
			desktop->get_frustum(frustum, viewportID);
			// Will update view matrix and frustum information

			// Retrieve tracked position from view matrix.
			float pos[4]; 
			float viewInverted[16];
			mat4f_invert_new(viewInverted, viewmatrix);
			mat4f_getColumn(pos, viewInverted, 3);

			/* Make sure all DGR hosts can get the position so that they
			 * can update the frustum appropriately */
			dgr_setget("!!viewMatPos", pos, sizeof(float)*3);

			/* Update view frustum. */
			frustum[0] -= pos[0];
			frustum[1] -= pos[0];
			frustum[2] -= pos[1];
			frustum[3] -= pos[1];
			frustum[4] += pos[2];
			frustum[5] += pos[2];

			/* Create a projection matrix from our updated frustum values */
			mat4f_frustum_new(projmatrix,
			                  frustum[0], frustum[1], frustum[2],
			                  frustum[3], frustum[4], frustum[5]);
			
			/* calculate a new view matrix */
			float lookat[3];
			float forwardVec[3] = { 0, 0, -1 };
			for(int i=0; i<3; i++)
				lookat[i] = pos[i]+forwardVec[i];
			float up[3] = {0, 1, 0};
			mat4f_lookatVec_new(viewmatrix, pos, lookat, up);
		}
	}

	/* Send the view matrix to DGR. At some point in the future, we
	 * may have multiple computers using different view matrices. For
	 * now, even in IVS mode, all processes will use the same view
	 * matrix (IVS uses different view frustums per process). */
	char dgrkey[128];
	snprintf(dgrkey, 128, "!!viewmat%d", viewportID);
	dgr_setget(dgrkey, viewmatrix, sizeof(float)*16);

	/* Sanity checks */
	viewmat_validate_ipd(viewmatrix, viewportID);
	return eye;
}

/** Gets the viewpgort information for a particular viewport.

 @param viewportValue A location to be filled in with the viewport x
 coordinate, y coordinate, width and height.

 @param viewportNum Which viewport number is being requested. If you
 are using only one viewport, set this to 0.
*/
void viewmat_get_viewport(int viewportValue[4], int viewportNum)
{
	desktop->get_viewport(viewportValue, viewportNum);
}


/** Returns the number of viewports that viewmat has.

    @return The number of viewports that viewmat has.
*/
int viewmat_num_viewports()
{
	return desktop->num_viewports();
}

/** Returns the framebuffer that this viewport is on. In many cases,
 * this will be the framebuffer for your window. However, some
 * rendering systems (such as the Oculus) render to an off-screen
 * texture. */
int viewmat_get_framebuffer(int viewportID)
{
	return desktop->get_framebuffer(viewportID);
}

/** Returns the view frustum for this viewport. */
void viewmat_get_frustum(float frustum[6], int viewportID)
{
	if(desktop->provides_projmat_only())
	{
		msg(MSG_FATAL, "The current display mode does not give us access to a view frustum.");
		exit(EXIT_FAILURE);
	}
		
	desktop->get_frustum(frustum, viewportID);
}

/** Returns the view frustum for the overall screen. For normal
 * desktop applications, this will match the normal frustum. However,
 * if this process is responsible for rendering a portion of a larger
 * view frustum, this function will provide access to the larger
 * overall frustum. */
void viewmat_get_master_frustum(float frustum[6])
{
	const char* frustumString = kuhl_config_get("frustum.master");
	
	if(frustumString != NULL)
	{
		if(sscanf(frustumString, "%f %f %f %f %f %f",
		          &(frustum[0]), &(frustum[1]), &(frustum[2]),
		          &(frustum[3]), &(frustum[4]), &(frustum[5])) != 6)
		{
			msg(MSG_FATAL, "Unable to parse 'frustum.master' configuration variable. It contained: %s", frustumString);
			exit(EXIT_FAILURE);
		}
		else
		{
			return; // success
		}
	}

	/* If frustum.master wasn't set, we should notify the user. If
	 * they are running a program in non-DGR mode, then there is
	 * likely no problem with assuming that the master frustum matches
	 * the viewport frustum. However, if DGR is set, then there is a
	 * good chance that the user intends that the master frustum
	 * differs from the viewport frustum. */
	static int warnTheUser = 1;
	if(warnTheUser)
	{
		msg_type warnValue = MSG_INFO;
		if(dgr_is_enabled())
			warnValue = MSG_WARNING;
		msg(warnValue, "The 'frustum.master' configuration variable is missing or empty; we are assuming that the master view frustum matches the view frustum of viewport 0.");
		warnTheUser = 0;
	}
	viewmat_get_frustum(frustum, 0);
}
