/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

#pragma once

#if !defined(MISSING_OVR) && defined(__linux__)

#include "camcontrol.h"
#include "dispmode-oculus-linux.h"

#define OVR_OS_LINUX
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"

class camcontrolOculusLinux : public camcontrol
{
private:
	dispmodeOculusLinux *oculus;
	float oculusPosition[3];
public:
	camcontrolOculusLinux(dispmode *inDispMode, const float initialPos[3]);
	~camcontrolOculusLinux();
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};


#endif
