#include <stdlib.h>
#include "kuhl-util.h"
#include "camcontrol-vrpn.h"
#include "vecmat.h"
#include "vrpn-help.h"

camcontrolVrpn::camcontrolVrpn(dispmode *currentDisplayMode, const char *inObject, const char *inHostname)
	:camcontrol(currentDisplayMode)
{
	if(inObject == NULL)
		object = NULL;
	else
		object = strdup(inObject);

	if(inHostname == NULL)
		hostname = NULL;
	else
		hostname = strdup(inHostname);
}

camcontrolVrpn::~camcontrolVrpn()
{
	if(object != NULL)
		free(object);

	if(hostname != NULL)
		free(hostname);
}

viewmat_eye camcontrolVrpn::get_separate(float pos[3], float rot[16], viewmat_eye requestedEye)
{
	viewmat_eye returnVal = VIEWMAT_EYE_MIDDLE;
	vrpn_get(object, hostname, pos, rot);

	/* In many cases, the code above is all we need to do. Some
	 * objects, need to be adjusted or rotated, however. */

	const char *hostname = vrpn_default_host();
	if(hostname == NULL || object == NULL)
		return returnVal;
	
	/* Some objects in the IVS lab need to be rotated to match the
	 * orientation that we expect. Apply the fix here. */
	if(vrpn_is_vicon(hostname)) // MTU vicon tracker
	{
		/* Note, orient has not been transposed/inverted yet. Doing
		 * orient*offset will effectively effectively be rotating the
		 * camera---not the world. */ 
		if(strcmp(object, "DK2") == 0)
		{
			float offsetVicon[16];
			mat4f_identity(offsetVicon);
			mat4f_rotateAxis_new(offsetVicon, 90, 1,0,0);
			mat4f_mult_mat4f_new(rot, rot, offsetVicon);
		}

		if(strcmp(object, "DSight") == 0)
		{
			float offsetVicon1[16];
			mat4f_identity(offsetVicon1);
			mat4f_rotateAxis_new(offsetVicon1, 90, 1,0,0);
			float offsetVicon2[16];
			mat4f_identity(offsetVicon2);
			mat4f_rotateAxis_new(offsetVicon2, 180, 0,1,0);
			
			// orient = orient * offsetVicon1 * offsetVicon2
			mat4f_mult_mat4f_many(rot, rot, offsetVicon1, offsetVicon2, NULL);
		}
	}

	
	return VIEWMAT_EYE_MIDDLE;
}
