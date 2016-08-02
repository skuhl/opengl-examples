#pragma once
#include "dispmode.h"

class dispmodeDesktop: public dispmode
{
public:
	dispmodeDesktop();
	virtual viewmat_eye eye_type(int viewportID);
	virtual int num_viewports(void);
	virtual void get_viewport(int viewportValue[4], int viewportId);
	virtual void get_frustum(float result[6], int viewportID);
};
