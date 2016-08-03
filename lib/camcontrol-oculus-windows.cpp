#include <stdlib.h>
#include "camcontrol-oculus-windows.h"

#include "kuhl-util.h"
#include "vecmat.h"

camcontrolOculusWindows::camcontrolOculusWindows(const float initialPos[3], dispmode *inDispmode)
{
	vec3f_copy(oculusPosition, initialPos);
	oculus = dynamic_cast<dispmodeOculusWindows*>(inDispmode);
}

camcontrolOculusWindows::~camcontrolOculusWindows()
{
}

viewmat_eye camcontrolOculusWindows::get_separate(float pos[3], float rot[16], viewmat_eye requestedEye)
{
	mat4f_identity(rot);
	vec3f_set(pos, 0, 0, 0);

	ovrEyeType eye;
	switch(requestedEye)
	{
		case VIEWMAT_EYE_LEFT:
			eye = ovrEye_Left;
			break;
		case VIEWMAT_EYE_RIGHT:
			eye = ovrEye_Right;
			break;
		default:
			msg(MSG_FATAL, "You requested an eye that does not exist: %d", requestedEye);
			exit(EXIT_FAILURE);
	}

	ovrEyeRenderDesc eyeRenderDesc[2];
	eyeRenderDesc[0]  = ovr_GetRenderDesc(oculus->session, ovrEye_Left, oculus->hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(oculus->session, ovrEye_Right, oculus->hmdDesc.DefaultEyeFov[1]);
	oculus->HmdToEyeOffset[0] = eyeRenderDesc[0].HmdToEyeOffset;
	oculus->HmdToEyeOffset[1] = eyeRenderDesc[1].HmdToEyeOffset;

	ovr_GetEyePoses(oculus->session, oculus->frameIndex, ovrTrue, oculus->HmdToEyeOffset, oculus->EyeRenderPose, &(oculus->sensorSampleTime));

	float orientationQuat[4];
	orientationQuat[0] = oculus->EyeRenderPose[eye].Orientation.x;
	orientationQuat[1] = oculus->EyeRenderPose[eye].Orientation.y;
	orientationQuat[2] = oculus->EyeRenderPose[eye].Orientation.z;
	orientationQuat[3] = oculus->EyeRenderPose[eye].Orientation.w;
	mat4f_rotateQuatVec_new(rot, orientationQuat);

	pos[0] = oculus->EyeRenderPose[eye].Position.x;
	pos[1] = oculus->EyeRenderPose[eye].Position.y;
	pos[2] = oculus->EyeRenderPose[eye].Position.z;

	// Add translation based on the user-specified initial
	// position. You may choose to initialize the Oculus
	// position to y=1.5 meters to approximate a normal
	// standing eyeheight.
	vec3f_add_new(pos, oculusPosition, pos);


	//printf("eye: %d\n", eye);
	//mat4f_print(rot);
	//vec3f_print(pos);
	return requestedEye;
}

