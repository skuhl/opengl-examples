#include "msg.h"
#include "viewmat.h"
#include "vecmat.h"
#include "dispmode-hmd.h"
#include "kuhl-config.h"

dispmodeHMD::dispmodeHMD()
{
	ipd = 6.0;
	if(kuhl_config_get("ipd") == NULL)
	{
		msg(MSG_WARNING, "The 'ipd' setting was unset or empty. Defaulting to %0.2f cm.", ipd);
	}
	else
	{
		float ipd = kuhl_config_float("ipd", -1234.0f, -1234.0f);
		if(ipd == -1234.0f)
		{
			msg(MSG_WARNING, "The 'ipd' setting was set an invalid value: '%s'", kuhl_config_get("ipd"));
			msg(MSG_WARNING, "Defaulting IPD to %0.2f cm.");
		}
	}

}

viewmat_eye dispmodeHMD::eye_type(int viewportID)
{
	if(viewportID == 0)
		return VIEWMAT_EYE_LEFT;
	else if(viewportID == 1)
		return VIEWMAT_EYE_RIGHT;
	else
		return VIEWMAT_EYE_UNKNOWN;
}

void dispmodeHMD::get_eyeoffset(float offset[3], viewmat_eye eye)
{
	if(eye == VIEWMAT_EYE_LEFT)
		vec3f_set(offset, -ipd/2.0f / 100.0f, 0, 0);
	else if(eye == VIEWMAT_EYE_RIGHT)
		vec3f_set(offset, ipd/2.0f / 100.0f, 0, 0);
	else
		vec3f_set(offset,0,0,0);
}

int dispmodeHMD::num_viewports(void)
{
	return 2;
}

void dispmodeHMD::get_viewport(int viewportValue[4], int viewportID)
{
	/* Each viewport has half of the screen */
	int windowWidth, windowHeight;
	viewmat_window_size(&windowWidth, &windowHeight);

	if(viewportID == 0)
	{
		viewportValue[0] = 0;   // x,y coords
		viewportValue[1] = 0;
		viewportValue[2] = windowWidth/2; // width, height
		viewportValue[3] = windowHeight;
	}
	else if(viewportID == 1)
	{
		viewportValue[0] = windowWidth/2;   // x,y coords
		viewportValue[1] = 0;
		viewportValue[2] = windowWidth/2; // width, height
		viewportValue[3] = windowHeight;
	}
	else
	{
		msg(MSG_WARNING, "Invalid viewportID=%d requested in mode", viewportID);
	}
}

void dispmodeHMD::get_frustum(float result[6], int viewportID)
{
	int viewport[4];
	this->get_viewport(viewport, viewportID);
	int viewportW = viewport[2];
	int viewportH = viewport[3];

	float aspect = viewportW/(float)viewportH;
	float nearPlane = kuhl_config_float("nearplane", 0.1f, 0.1f);
	float farPlane = kuhl_config_float("farplane", 200.0f, 200.0f);
	float vfov = kuhl_config_float("vfov", 65.0f, 65.0f);
	float fovyRad = (float) (vfov * M_PI/180.0f);
	float height = nearPlane * tanf(fovyRad/2.0f);
	float width = height * aspect;
	result[0] = -width;
	result[1] = width;
	result[2] = -height;
	result[3] = height;
	result[4] = nearPlane;
	result[5] = farPlane;
}
