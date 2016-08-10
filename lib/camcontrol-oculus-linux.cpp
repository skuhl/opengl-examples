#include <stdlib.h>
#include "kuhl-util.h"
#include "camcontrol-oculus-linux.h"
#include "vecmat.h"

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

#if 0
	printf("eye=%s\n", eye == ovrEye_Left ? "left" : "right");
	printf("Position: %f %f %f\n", pos[0], pos[1], pos[2]);
	printf("Orientation matrix:\n");
	mat4f_print(rot);
#endif
	return requestedEye;
}

