/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

#pragma once

#if !defined(MISSING_OVR) && defined(_WIN32)
#include "camcontrol.h"
#include "dispmode-oculus-windows.h"


class camcontrolOculusWindows : public camcontrol
{
private:
	dispmodeOculusWindows *oculus;
	float oculusPosition[3];
public:
	camcontrolOculusWindows(dispmode *currentDisplayMode, const float initialPos[3]);
	~camcontrolOculusWindows();
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};


#endif
