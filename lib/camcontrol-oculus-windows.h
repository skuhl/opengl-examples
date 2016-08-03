#pragma once

#if !defined(MISSING_OVR) && defined(_WIN32)

#include "viewmat.h"
#include "dispmode-oculus-windows.h"


class camcontrolOculusWindows : public camcontrol
{
private:
	dispmodeOculusWindows *oculus;
	float oculusPosition[3];
public:
	camcontrolOculusWindows(const float initialPos[3], dispmode *inDispMode);
	~camcontrolOculusWindows();
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};


#endif
