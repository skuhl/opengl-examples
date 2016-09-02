#include "msg.h"
#include "viewmat.h"
#include "vecmat.h"
#include "dispmode-desktop.h"
#include "kuhl-config.h"

dispmodeDesktop::dispmodeDesktop()
{

}

viewmat_eye dispmodeDesktop::eye_type(int viewportID)
{
	if(viewportID == 0)
		return VIEWMAT_EYE_MIDDLE;
	else
		return VIEWMAT_EYE_UNKNOWN;
}

int dispmodeDesktop::num_viewports(void)
{
	return 1;
}

void dispmodeDesktop::get_viewport(int viewportValue[4], int viewportID)
{
	if(viewportID != 0)
		msg(MSG_WARNING, "Requested a non-zero viewportID when display mode is desktop.");
	
	/* One viewport fills the entire window */
	int windowWidth, windowHeight;
	viewmat_window_size(&windowWidth, &windowHeight);

	viewportValue[0] = 0;   // x,y coords
	viewportValue[1] = 0;
	viewportValue[2] = windowWidth; // width, height
	viewportValue[3] = windowHeight;
}

void dispmodeDesktop::get_frustum(float result[6], int viewportID)
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
