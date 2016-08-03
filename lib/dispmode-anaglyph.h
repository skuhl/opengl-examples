#include "dispmode.h"

class dispmodeAnaglyph : public dispmode
{
public:
	dispmodeAnaglyph();
	virtual viewmat_eye eye_type(int viewportID);
	virtual int num_viewports(void);
	virtual void get_viewport(int viewportValue[4], int viewportId);
	virtual void get_frustum(float result[6], int viewportID);

	virtual void end_frame();
	virtual void begin_eye(int viewportID);
};
