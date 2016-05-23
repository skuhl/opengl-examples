/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 *
 * This file provides a simple C interface to get information about
 * the position and orientation of a tracked point from a VRPN
 * server. The VRPN library itself uses C++.
 *
 * For more information about VRPN, see:
 * http://www.cs.unc.edu/Research/vrpn/
 *
 * VRPN uses the Boost Software License which allows code to link to
 * VRPN and be published under a different license.
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

int vrpn_get(const char *object, const char *hostname, float pos[3], float orient[16]);
const char* vrpn_default_host(void);
int vrpn_is_vicon(const char *hostname);
float* vrpn_get_raw(const char *name, const char *host, int count);
	
#ifdef __cplusplus
} // end extern "C"
#endif
