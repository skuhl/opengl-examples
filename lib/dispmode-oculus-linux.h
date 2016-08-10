#pragma once

#if !defined(MISSING_OVR) && defined(__linux__)

#include "dispmode.h"

#define OVR_OS_LINUX
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"


class dispmodeOculusLinux : public dispmode
{
public:
	dispmodeOculusLinux();
	~dispmodeOculusLinux();
	virtual viewmat_eye eye_type(int viewportID);
	virtual int num_viewports(void);
	virtual void get_eyeoffset(float offset[3], viewmat_eye eye);
	virtual void get_viewport(int viewportValue[4], int viewportId);
	virtual void get_frustum(float result[6], int viewportID);
	virtual void get_projmatrix(float result[6], int viewportID);
	virtual void begin_frame();
	virtual void end_frame();
	virtual void begin_eye(int viewportId);
	virtual int get_framebuffer(int viewportID);


	ovrHmd hmd;
	GLint leftFramebuffer, rightFramebuffer, leftFramebufferAA, rightFramebufferAA;
	ovrSizei recommendTexSizeL,recommendTexSizeR;
	ovrGLTexture EyeTexture[2];
	ovrEyeRenderDesc eye_rdesc[2];
	ovrFrameTiming timing;
	ovrPosef pose[2];

};


#endif
