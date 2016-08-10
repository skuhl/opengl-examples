/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

#pragma once
#include "camcontrol.h"

class camcontrolMouse : public camcontrol
{
public:
	camcontrolMouse(dispmode *currentDisplayMode, const float pos[3], const float look[3], const float up[3]);
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};
