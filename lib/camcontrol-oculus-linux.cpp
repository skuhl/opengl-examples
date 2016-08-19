#include <stdlib.h>
#include "kuhl-util.h"
#include "camcontrol-oculus-linux.h"
#include "vecmat.h"
#include "vrpn-help.h"
#include "sensorfuse.h"

camcontrolOculusLinux::camcontrolOculusLinux(dispmode *inDispmode, const float initialPos[3])
	:camcontrol(inDispmode)
{
	vec3f_copy(oculusPosition, initialPos);
	oculus = dynamic_cast<dispmodeOculusLinux*>(inDispmode);
}

camcontrolOculusLinux::~camcontrolOculusLinux()
{
}

viewmat_eye camcontrolOculusLinux::get_separate(float pos[3], float rot[16], viewmat_eye requestedEye)
{
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
			msg(MSG_FATAL, "You requested an eye that does not exist.");
			exit(EXIT_FAILURE);
	}

	oculus->pose[eye] = ovrHmd_GetHmdPosePerEye(oculus->hmd, eye);
	
	const char *vrpnObject = kuhl_config_get("viewmat.vrpn.object");
	if(vrpnObject != NULL)
	{
		float vrpnPos[3],vrpnOrient[16];
		vrpn_get(vrpnObject, NULL, vrpnPos, vrpnOrient);

		float origOrient[16];
		mat4f_rotateQuat_new(origOrient,
		                     oculus->pose[eye].Orientation.x,
		                     oculus->pose[eye].Orientation.y,
		                     oculus->pose[eye].Orientation.z,
		                     oculus->pose[eye].Orientation.w);

#if 0
		// This just contains the horizontal offsets for the IPD
		float offset[16];
		mat4f_translate_new(offset,
		                    oculus->eye_rdesc[eye].HmdToEyeViewOffset.x,
		                    oculus->eye_rdesc[eye].HmdToEyeViewOffset.y,
		                    oculus->eye_rdesc[eye].HmdToEyeViewOffset.z);
#endif

		if(strcmp(vrpnObject, "DK2") == 0)
		{
			float offsetVicon[16];
			mat4f_identity(offsetVicon);
			mat4f_rotateAxis_new(offsetVicon, 90, 1,0,0);
			mat4f_mult_mat4f_new(vrpnOrient, vrpnOrient, offsetVicon);
		}

		
		// fill in the rot and pos variables:
		//mat4f_copy(rot, origOrient);
		//mat4f_copy(rot, vrpnOrient);
		sensorfuse(rot, origOrient, vrpnOrient);
		vec3f_copy(pos, vrpnPos);

		// Eye offset will be retrieved from a subsequent call to dispmodeOculusWindows::get_eyeoffset()
		return VIEWMAT_EYE_MIDDLE;
	}
	else
	{
		vec3f_set(pos,  // position (includes IPD offset)
		          oculus->pose[eye].Position.x,
		          oculus->pose[eye].Position.y,
		          oculus->pose[eye].Position.z);
		
		mat4f_rotateQuat_new(rot,                          // rotation
		                     oculus->pose[eye].Orientation.x,
		                     oculus->pose[eye].Orientation.y,
		                     oculus->pose[eye].Orientation.z,
		                     oculus->pose[eye].Orientation.w);
		
		// Add translation based on the user-specified initial
		// position. You may choose to initialize the Oculus
		// position to y=1.5 meters to approximate a normal
		// standing eyeheight.
		vec3f_add_new(pos, oculusPosition, pos);
		
		return requestedEye;
		
	}
}

