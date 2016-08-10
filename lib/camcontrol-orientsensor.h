/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */
#pragma once

#include "camcontrol.h"
#include "orient-sensor.h"

class camcontrolOrientSensor : public camcontrol
{
private:
	OrientSensorState orientsense;
	float position[3];

public:
	camcontrolOrientSensor(dispmode *currentDisplayMode, const float pos[3]);
	~camcontrolOrientSensor();
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};
