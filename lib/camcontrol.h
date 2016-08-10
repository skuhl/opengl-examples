/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

#pragma once
#include "viewmat.h"
#include "dispmode.h"

class camcontrol
{
private:
	float pos[3],look[3],up[3];
protected:
	dispmode *displaymode;
public:
	camcontrol(dispmode *currentDisplayMode);
	camcontrol(dispmode *currentDisplayMode, const float inPos[3], const float inLook[3], const float inUp[3]);
	virtual viewmat_eye get_separate(float outPos[3], float outRot[16], viewmat_eye requestedEye);
	virtual viewmat_eye get(float matrix[16], viewmat_eye requestedEye);
};
