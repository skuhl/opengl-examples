#pragma once
#include "viewmat.h"
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
	camcontrolOculusLinux(const float initialPos[3], dispmode *inDispMode);
	~camcontrolOculusLinux();
	viewmat_eye get_separate(float pos[3], float rot[16], viewmat_eye requestedEye);
};
