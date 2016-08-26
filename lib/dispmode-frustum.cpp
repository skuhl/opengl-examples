#include "windows-compat.h"
#include "msg.h"
#include "viewmat.h"
#include "vecmat.h"
#include "dispmode-frustum.h"
#include "kuhl-config.h"

dispmodeFrustum::dispmodeFrustum(void)
{
	const char* frustumString = kuhl_config_get("frustum");
	float inFrustum[6];
	if(frustumString != NULL)
	{
		if(sscanf(frustumString, "%f %f %f %f %f %f",
		          &(inFrustum[0]), &(inFrustum[1]), &(inFrustum[2]),
		          &(inFrustum[3]), &(inFrustum[4]), &(inFrustum[5])) != 6)
		{
			msg(MSG_ERROR, "Unable to parse 'frustum' configuration variable. It contained: %s", frustumString);
		}
		else
		{
			msg(MSG_DEBUG, "Using view frustum: %s\n", frustumString);
			set_frustum(inFrustum);
			return;
		}
	}

	msg(MSG_WARNING, "Using default frustum values---this is probably not want you want.");
	float defaultFrustum[6] = { -1.0f ,1.0f,-1.0f,1.0f,.1f,50.0f };
	set_frustum(defaultFrustum);
}

dispmodeFrustum::dispmodeFrustum(const float inFrustum[6])
{
	set_frustum(inFrustum);
}

dispmodeFrustum::dispmodeFrustum(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
	float temp[6] = { left, right, bottom, top, nearPlane, farPlane };
	set_frustum(temp);
}

void dispmodeFrustum::set_frustum(const float inFrustum[6])
{
	for(int i=0; i<6; i++)
		frustum[i] = inFrustum[i];

	if(frustum[4] < 0 || frustum[5] < 0)
		msg(MSG_WARNING, "The near and far values in the frustum should be positive (i.e., this matches the behavior of the old OpenGL glFrustum() function call.)");

}

void dispmodeFrustum::print_frustum()
{
	msg(MSG_INFO, "View frustum: left=%f right=%f bot=%f top=%f near=%f far=%f\n",
	    frustum[0], frustum[1], frustum[2], frustum[3], frustum[4], frustum[5]);
}


void dispmodeFrustum::get_frustum(float result[6], int viewportID)
{
	for(int i=0; i<6; i++)
		result[i] = frustum[i];
}
