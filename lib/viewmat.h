/* Copyright (c) 2014-2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file

    viewmat provides an easy-to-use interface for generating a
    projection matrix (view frustum), view matrices (camera
    position/orientation) while also rendering graphics correctly for
    different display systems (desktop, anaglyph, Oculus, etc).

    To use this function, call:
    viewmat_init()
    viewmat_begin_frame() prior to drawing a frame
    viewmat_num_viewports() to get how many viewports/eyes we are rendering to
    viewmat_get_viewport() to get the viewport position and dimensions
    viewmat_begin_eye() prior to drawing each eye
    viewmat_get() to get projection and view matrices for the eye.
    viewmat_end_eye() when finished drawing graphics for an eye.
    viewmat_end_frame() when finished drawing a frame.

    If you are running in a DGR environment, it also ensures that the
    information is sent (dgr_update()) and that the view matrices are
    synchronized across all DGR processes.

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
void viewmat_end_frame(void);
void viewmat_begin_eye(int viewportID);
void viewmat_end_eye(int viewportID);

int viewmat_get_framebuffer(int viewportID);

void viewmat_init(const float pos[3], const float look[3], const float up[3]);
viewmat_eye viewmat_get(float viewmatrix[16], float projmatrix[16], int viewportNum);

int viewmat_num_viewports(void);
void viewmat_get_viewport(int viewportValue[4], int viewportNum);

void viewmat_get_frustum(float frustum[6], int viewportID);
void viewmat_get_master_frustum(float frustum[6]);

#ifdef __cplusplus
} // end extern "C"
#endif
