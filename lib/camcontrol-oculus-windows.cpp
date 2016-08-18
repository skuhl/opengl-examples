#include "windows-compat.h"
#include <stdlib.h>

#include "camcontrol-oculus-windows.h"
#include "sensorfuse.h"
#include "kuhl-util.h"
#include "vecmat.h"
#include "vrpn-help.h"

camcontrolOculusWindows::camcontrolOculusWindows(dispmode *currentDisplayMode, const float initialPos[3])
	:camcontrol(currentDisplayMode)
{
	vec3f_copy(oculusPosition, initialPos);
	oculus = dynamic_cast<dispmodeOculusWindows*>(currentDisplayMode);
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

	// Get eye offsets
	ovrEyeRenderDesc eyeRenderDesc[2];
	eyeRenderDesc[0]  = ovr_GetRenderDesc(oculus->session, ovrEye_Left, oculus->hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(oculus->session, ovrEye_Right, oculus->hmdDesc.DefaultEyeFov[1]);
	oculus->HmdToEyeOffset[0] = eyeRenderDesc[0].HmdToEyeOffset; // -X for left eye
	oculus->HmdToEyeOffset[1] = eyeRenderDesc[1].HmdToEyeOffset; // +X for right eye

	 // Given eye offsets, get position and orientation
	ovr_GetEyePoses(oculus->session, oculus->frameIndex, ovrTrue, oculus->HmdToEyeOffset, oculus->EyeRenderPose, &(oculus->sensorSampleTime));
	const char *vrpnObject = kuhl_config_get("viewmat.vrpn.object");
	if (vrpnObject != NULL)
	{
		float vrpnPos[3], vrpnOrient[16];
		vrpn_get(vrpnObject, NULL, vrpnPos, vrpnOrient);

		if (strcmp(vrpnObject, "DK2") == 0)
		{
			float offsetVicon[16];
			mat4f_identity(offsetVicon);
			mat4f_rotateAxis_new(offsetVicon, 90, 1, 0, 0);
			mat4f_mult_mat4f_new(vrpnOrient, vrpnOrient, offsetVicon);
		}

		// Only use the orientation from the Oculus sensor.
		// (Orientation should be same for both eyes)
		float origOrient[16];
		mat4f_rotateQuat_new(origOrient,
			oculus->EyeRenderPose[eye].Orientation.x,
			oculus->EyeRenderPose[eye].Orientation.y,
			oculus->EyeRenderPose[eye].Orientation.z,
			oculus->EyeRenderPose[eye].Orientation.w);

		sensorfuse(rot, origOrient, vrpnOrient);

		// Copy the VRPN position
		vec3f_copy(pos, vrpnPos);

		// Eye offset will be retrieved from a subsequent call to dispmodeOculusWindows::get_eyeoffset()
		return VIEWMAT_EYE_MIDDLE;
	}
	else
	{
		float orientationQuat[4];
		orientationQuat[0] = oculus->EyeRenderPose[eye].Orientation.x;
		orientationQuat[1] = oculus->EyeRenderPose[eye].Orientation.y;
		orientationQuat[2] = oculus->EyeRenderPose[eye].Orientation.z;
		orientationQuat[3] = oculus->EyeRenderPose[eye].Orientation.w;
		mat4f_rotateQuatVec_new(rot, orientationQuat);

		// Note: Eyes rotate around some location if sensor is NOT used.
		pos[0] = oculus->EyeRenderPose[eye].Position.x;
		pos[1] = oculus->EyeRenderPose[eye].Position.y;
		pos[2] = oculus->EyeRenderPose[eye].Position.z;
	}

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

