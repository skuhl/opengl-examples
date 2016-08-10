#include "dispmode.h"

class dispmodeHMD : public dispmode
{
private:
	float ipd;
	
public:
	dispmodeHMD();
	virtual viewmat_eye eye_type(int viewportID);
	virtual void get_eyeoffset(float offset[3], viewmat_eye eye);
	virtual int num_viewports(void);
	virtual void get_viewport(int viewportValue[4], int viewportId);
	virtual void get_frustum(float result[6], int viewportID);
};
