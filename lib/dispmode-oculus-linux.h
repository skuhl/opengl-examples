#include "viewmat.h"

class dispmodeOculusLinux : public dispmode
{
public:
	dispmodeOculusLinux();
	virtual viewmat_eye eye_type(int viewportID);
	virtual int num_viewports(void);
	virtual void get_viewport(int viewportValue[4], int viewportId);
	virtual void get_frustum(float result[6], int viewportID);
	virtual void begin_frame();
	virtual void end_frame();
	virtual void begin_eye(int viewportId);
};
