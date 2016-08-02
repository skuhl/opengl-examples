#include <stdlib.h>
#include "kuhl-util.h"
#include "camcontrol-oculus-linux.h"
#include "vecmat.h"

camcontrolOculusLinux::camcontrolOculusLinux(const float initialPos[3], dispmode *inDispmode)
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
			msg(MSG_FATAL, "You a eye that does not exist.");
			exit(EXIT_FAILURE);
	}
	
#if 0
	/* Construct posMat and rotMat matrices which indicate the
	 * position and orientation of the HMD. */
	if(viewmat_vrpn_obj) // get position from VRPN
	{
		float offsetMat[16];
		mat4f_identity(offsetMat);  // Viewpoint offset (IPD, etc);


		/* Get the offset for the left and right eyes from
		 * Oculus. If you are using a separate tracking system, you
		 * may also want to apply an offset here between the tracked
		 * point and the eye location. */
		mat4f_translate_new(offsetMat,
		                    eye_rdesc[eye].HmdToEyeViewOffset.x, // left & right IPD offset
		                    eye_rdesc[eye].HmdToEyeViewOffset.y, // vertical offset
		                    eye_rdesc[eye].HmdToEyeViewOffset.z); // forward/back offset

		float pos[3] = { 0,0,0 };
		vrpn_get(viewmat_vrpn_obj, NULL, pos, rotMat);
		mat4f_translate_new(posMat, -pos[0], -pos[1], -pos[2]); // position
		viewmat_fix_rotation(rotMat);
	}
	else // get position from Oculus tracker
#endif
	{
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
	}
#if 0
	printf("eye=%s\n", eye == ovrEye_Left ? "left" : "right");
	printf("Eye offset according to OVR (only used if VRPN is used): ");
	mat4f_print(offsetMat);
	printf("Rotation sensing (from OVR or VRPN): ");
	mat4f_print(rotMat);
	printf("Position tracking (from OVR or VRPN): ");
	mat4f_print(posMat);
	printf("Initial position (from set in viewmat_init()): ");
	mat4f_print(initPosMat);
	printf("Final view matrix: ");
	mat4f_print(viewmatrix);
#endif
	return requestedEye;
}

