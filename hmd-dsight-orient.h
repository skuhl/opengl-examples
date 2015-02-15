/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
* @author Evan Hauck
*/

#ifndef __HMDDSIGHTORIENT_H__
#define __HMDDSIGHTORIENT_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int fd;
} HmdControlState;

HmdControlState initHmdControl(const char* deviceFile);
void updateHmdControl(HmdControlState *state, float quaternion[4]);

#ifdef __cplusplus
} // end extern "C"
#endif
#endif // __HMDDSIGHTORIENT_H__
