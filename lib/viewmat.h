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

viewmat_eye viewmat_viewport_to_eye(int viewportNum);
	
void viewmat_window_size(int *width, int *height);

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
#endif
