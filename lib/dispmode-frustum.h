#pragma once
#include "dispmode-desktop.h"

class dispmodeFrustum: public dispmodeDesktop
{
private:
	float frustum[6];
public:
	dispmodeFrustum();
	dispmodeFrustum(const float inFrustum[6]);
	dispmodeFrustum(float left, float right, float bottom, float top, float near, float far);
	void print_frustum();
	void set_frustum(const float inFrustum[6]);
			
	virtual void get_frustum(float result[6], int viewportID);
};
