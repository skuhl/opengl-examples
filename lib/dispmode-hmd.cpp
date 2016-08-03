#include "msg.h"
#include "viewmat.h"
#include "vecmat.h"
#include "dispmode-hmd.h"

dispmodeHMD::dispmodeHMD()
{
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
	float nearPlane = 0.1f;
	float farPlane = 200.0f;
	float vfov = 65.0f;
	float fovyRad = vfov * M_PI/180.0f;
	float height = nearPlane * tanf(fovyRad/2.0f);
	float width = height * aspect;
	result[0] = -width;
	result[1] = width;
	result[2] = -height;
	result[3] = height;
	result[4] = nearPlane;
	result[5] = farPlane;
}
