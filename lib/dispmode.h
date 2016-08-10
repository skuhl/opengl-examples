#pragma once
#include "viewmat.h"

class dispmode
{
public:
	dispmode();
	virtual viewmat_eye eye_type(int viewportID);
	virtual void get_eyeoffset(float offset[3], viewmat_eye eye);
	void get_eyeoffset(float offset[3], int viewportID);

	virtual int num_viewports(void);
	virtual void get_viewport(int viewportValue[4], int viewportId);
	virtual void get_frustum(float result[6], int viewportID);
	virtual int get_framebuffer(int viewportID);
	virtual int provides_projmat_only();
	virtual void get_projmatrix(float projmatrix[16], int viewportID);
	virtual void begin_frame();
	virtual void end_frame();
	virtual void begin_eye(int viewportID);
	virtual void end_eye(int viewportID);

};
