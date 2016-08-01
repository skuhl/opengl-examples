/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file

    viewmat provides an easy-to-use interface for generating
    appropriate view matrices for a variety of systems. Simply call
    viewmat_init() when your program starts and th en call
    viewmat_get() whenever you want to get a view matrix.

    viewmat handles the details of talking with a VRPN server for IVS
    and HMD systems. If you are running in a DGR environment, it also
    makes sure that the view matrices are synchronized across all DGR
    processes.

    VIEWMAT_MODE="mouse" - Can be set to ivs, hmd, none, mouse. If not
    set, "mouse" is assumed.

    VIEWMAT_VRPN_OBJECT="Head" - The name of the tracked object that
    will be placed on the user's head. Currently only used in "ivs"
    mode.

    @author Scott Kuhl
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef enum { VIEWMAT_EYE_LEFT,    /*< Right eye viewport */
               VIEWMAT_EYE_RIGHT,   /*< Left eye viewport */
               VIEWMAT_EYE_MIDDLE,  /*< Single viewport */
               VIEWMAT_EYE_UNKNOWN } viewmat_eye;

void viewmat_window_size(int *width, int *height);
float viewmat_window_aspect_ratio(void);
	
void viewmat_begin_frame(void);
void viewmat_begin_eye(int viewportID);
int viewmat_get_blitted_framebuffer(int viewportID);
void viewmat_end_frame(void);
	
void viewmat_init(const float pos[3], const float look[3], const float up[3]);
viewmat_eye viewmat_get(float viewmatrix[16], float projmatrix[16], int viewportNum);
int viewmat_num_viewports(void);
void viewmat_get_viewport(int viewportValue[4], int viewportNum);

	
#ifdef __cplusplus
} // end extern "C"


#include "vecmat.h"
class camcontrol
{
private:
	float pos[3],look[3],up[3];
	
public:
	camcontrol()
	{
		float inPos[3] = { 0,0,0 };
		float inLook[3] = {0,0,-1};
		float inUp[3] = {0,1,0};
		vec3f_copy(pos, inPos);
		vec3f_copy(look, inLook);
		vec3f_copy(up, inUp);
	};
	
	camcontrol(const float inPos[3], const float inLook[3], const float inUp[3])
	{
		vec3f_copy(pos, inPos);
		vec3f_copy(look, inLook);
		vec3f_copy(up, inUp);
	};

	/** Gets camera position and a rotation matrix for the camera.
	    
	    @param outPos The position of the camera.
	    
	    @param outRot A rotation matrix for the camera.

	    @return The eye that the matrix is actually for. In some
	    cases, the requestedEye may not match the actual eye. For
	    example, the mouse movement manipulator might always return
	    VIEWMAT_EYE_MIDDLE regardless of which eye was requested (and
	    the caller must then update it appropriately for a specific
	    eye). Other systems such as that for the Oculus may return
	    different matrices for different eyes.
	*/
	virtual viewmat_eye get_separate(float outPos[3], float outRot[16], viewmat_eye requestedEye)
	{
		mat4f_lookatVec_new(outRot, pos, look, up);
		float zero[4] = { 0,0,0,1 };
		mat4f_setColumn(outRot, zero, 3);

		vec3f_copy(outPos, pos);
		return VIEWMAT_EYE_MIDDLE;
	};

	/** Gets a view matrix.
	    
	    @param matrix The requested view matrix.
	    
	    @param requestedEye The eye that we are requesting.

	    @return The eye that the matrix is actually for. In some
	    cases, the requestedEye may not match the actual eye. For
	    example, the mouse movement manipulator might always return
	    VIEWMAT_EYE_MIDDLE regardless of which eye was requested (and
	    the caller must then update it appropriately for a specific
	    eye). Other systems such as that for the Oculus may return
	    different matrices for different eyes.
	*/
	virtual viewmat_eye get(float matrix[16], viewmat_eye requestedEye) {
		float pos[3], rot[16], trans[16];
		viewmat_eye actualEye = this->get_separate(pos, rot, requestedEye);
		mat4f_translate_new(trans, -pos[0],-pos[1],-pos[2]); // negate translation because we are translating camera, not an object
		mat4f_transpose(rot); // transpose because we are rotating camera, not an object
		mat4f_mult_mat4f_new(matrix, rot, trans);

		if(actualEye == VIEWMAT_EYE_MIDDLE &&
		   (requestedEye == VIEWMAT_EYE_LEFT ||
		    requestedEye == VIEWMAT_EYE_RIGHT) )
		{
			/* Update the view matrix based on which eye we are rendering */
			float eyeDist = 0.055f;  // TODO: Make this configurable.
			float eyeShift = eyeDist/2.0f;
			if(requestedEye == VIEWMAT_EYE_LEFT)
				eyeShift = eyeShift * -1;
			
			// Negate eyeShift because the matrix would shift the world, not
			// the eye by default.
			float shiftMatrix[16];
			mat4f_translate_new(shiftMatrix, -eyeShift, 0, 0);
			
			/* Adjust the view matrix by the eye offset */
			mat4f_mult_mat4f_new(matrix, shiftMatrix, matrix);
			
			actualEye = requestedEye;
		}
		
		return actualEye;
	};
};


#include <stdlib.h>
#include "msg.h"
#include "bufferswap.h"
class dispmode
{
public:
	dispmode()
	{
		msg(MSG_FATAL, "You shouldn't use the dispmode class directly---use a class that inherits from it.");
		exit(1);
	};
	virtual viewmat_eye eye_type(int viewportID) { return VIEWMAT_EYE_UNKNOWN;};
	virtual int num_viewports(void) { return 1; };
	virtual void get_viewport(int viewportValue[4], int viewportId) { };
	virtual void get_frustum(float result[6], int viewportID) { };

	/* Ideally, dispmode returns a view frustum. This allows us to
	 * further adjust the frustum for systems such as those that
	 * provide a dynamic frustum (e.g., IVS display wall). However,
	 * other systems might not provide a way to access the view
	 * frustum and just provides us with a projection matrix (Oculus).
	 */
	virtual int provides_projmat_only() { return 0; };
	virtual void get_projmatrix(float projmatrix[16], int viewportID)
	{
		float f[6];  // left, right, bottom, top, near>0, far>0
		this->get_frustum(f, viewportID);
		mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
	};

	virtual void begin_frame() { };
	virtual void end_frame() { bufferswap(); };
	virtual void begin_eye(int viewportID) { };
};

#endif
