/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

#include "camcontrol.h"

class camcontrolVrpn : public camcontrol
{
private:
	char *object;
	char *hostname;
public:
	camcontrolVrpn(dispmode *currentDisplayMode, const char *object, const char *hostname);
	~camcontrolVrpn();
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};
