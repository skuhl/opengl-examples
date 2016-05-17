/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file

    projmat provides an easy-to-use interface to handle GLUT windows
    and to generate projection matrices / view frustums for a multiple
    display devices. Simply call projmat_init() at the beginning of
    your program and then call projmat_get_frustum() to the values
    necessary to create a projection matrix.

    projmat does different things depending on environment variables
    that might be set:

    PROJMAT_WINDOW_SIZE="512 512" - Set the window size to 512x512 pixels<br>
    PROJMAT_WINDOW_POS="10 10" - Set window position to coordinate 10,10<br>
    PROJMAT_FULLSCREEN="1" - Make the window full screen.<br>
    PROJMAT_FRUSTUM="..." - The top bottom left right near far values for the current process view frustum.<br>
    PROJMAT_MASTER_FRUSTUM="..." - The top bottom left right near far values for master view frustum (if DGR is used).<br>
    PROJMAT_VFOV="65" - Sets the vertical field of view of the display to 65 degrees.

    Either the PROJMAT_FRUSTUM should be set or PROJMAT_VFOV should be set, but not both.

    If no environment variables are set, projmat generates a frustum
    equivalent to a basic perspective projection matrix.


    @author Scott Kuhl
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void projmat_init();
void projmat_get_frustum(float result[6], int viewportWidth, int viewportHeight, int viewportID);
void projmat_get_master_frustum(float result[6]);

#ifdef __cplusplus
} // end extern "C"
#endif
