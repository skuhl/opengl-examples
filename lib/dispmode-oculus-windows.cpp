

#include <stdlib.h>
#include <GL/glew.h>
#include "dispmode-oculus-windows.h"

#include "kuhl-util.h"
#include "msg.h"
#include "viewmat.h"
#include "vecmat.h"

dispmodeOculusWindows::dispmodeOculusWindows()
{
	frameIndex = 0;

	ovrResult result = ovr_Initialize(NULL);
	if (!OVR_SUCCESS(result))
		msg(MSG_ERROR, "ovr_Initialize() error");

	ovrGraphicsLuid luid;
	result = ovr_Create(&session, &luid);
	if(!OVR_SUCCESS(result))
		msg(MSG_ERROR, "ovr_Create() error");
	
	hmdDesc = ovr_GetHmdDesc(session);
	msg(MSG_INFO, "Initialized HMD: %s - %s\n", hmdDesc.Manufacturer, hmdDesc.ProductName);

	for (int eye = 0; eye < 2; ++eye)
	{
		ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
		eyeRenderTexture[eye] = new TextureBuffer(session, true, true, idealTextureSize, 1, NULL, 1);
		eyeDepthBuffer[eye] = new DepthBuffer(eyeRenderTexture[eye]->GetSize(), 0);
	}
//	ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);

}

dispmodeOculusWindows::~dispmodeOculusWindows()
{
#if 0
	// We are supposed to do these things when we are done:
	ovrHmd_Destroy(hmd);
	ovr_Shutdown();
#endif

}

void dispmodeOculusWindows::begin_frame()
{
#if 0
	if(hmd)
		timing = ovrHmd_BeginFrame(hmd, 0);
#endif
}

ovrEyeType dispmodeOculusWindows::get_ovrEye(int viewportID)
{
	ovrEyeType eye;
	switch (viewportID)
	{
		case VIEWMAT_EYE_LEFT:
			eye = ovrEye_Left;
			break;
		case VIEWMAT_EYE_RIGHT:
			eye = ovrEye_Right;
			break;
		default:
			msg(MSG_FATAL, "Requested an invalid viewportID: %d", viewportID);
			exit(EXIT_FAILURE);
	}
	return eye;
}

void dispmodeOculusWindows::end_frame()
{
	ovrLayerEyeFov ld;
	ld.Header.Type = ovrLayerType_EyeFov;
	ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

	for (int eye = 0; eye < 2; ++eye)
	{
		ld.ColorTexture[eye] = eyeRenderTexture[eye]->TextureChain;
		ld.Viewport[eye].Pos.x = 0;
		ld.Viewport[eye].Pos.y = 0;
		ld.Viewport[eye].Size = eyeRenderTexture[eye]->GetSize();
		ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
		ld.RenderPose[eye] = EyeRenderPose[eye];
		ld.SensorSampleTime = sensorSampleTime;
	}

	ovrLayerHeader* layers = &ld.Header;
	ovrResult result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
	frameIndex++;

	dispmode::end_frame(); // call our parent's implementation
}

void dispmodeOculusWindows::begin_eye(int viewportID)
{
	ovrEyeType eye = dispmodeOculusWindows::get_ovrEye(viewportID);
	eyeRenderTexture[eye]->SetAndClearRenderSurface(eyeDepthBuffer[eye]);

#if 0
	/* The EyeRenderOrder array indicates which eye should be
	 * rendered first. This code assumes that lower viewport IDs
	 * are drawn before higher numbers. */
	ovrEyeType eye = hmd->EyeRenderOrder[viewportID];
	if(eye == ovrEye_Left)
		glBindFramebuffer(GL_FRAMEBUFFER, leftFramebufferAA);
	else if(eye == ovrEye_Right)
		glBindFramebuffer(GL_FRAMEBUFFER, rightFramebufferAA);
	else
	{
		msg(MSG_FATAL, "Unknown viewport ID: %d\n", viewportID);
		exit(EXIT_FAILURE);
	}
#endif

}

void dispmodeOculusWindows::end_eye(int viewportID)
{
	ovrEyeType eye = dispmodeOculusWindows::get_ovrEye(viewportID);

	// Avoids an error when calling SetAndClearRenderSurface during next iteration.
	// Without this, during the next while loop iteration SetAndClearRenderSurface
	// would bind a framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
	// associated with COLOR_ATTACHMENT0 had been unlocked by calling wglDXUnlockObjectsNV.
	eyeRenderTexture[eye]->UnsetRenderSurface();

	// Commit changes to the textures so they get picked up frame
	eyeRenderTexture[eye]->Commit();
}

viewmat_eye dispmodeOculusWindows::eye_type(int viewportID)
{
	if(viewportID != 0 && viewportID != 1)
	{
		msg(MSG_FATAL, "Invalid viewport ID: %", viewportID);
		exit(EXIT_FAILURE);
	}

	if (viewportID == ovrEye_Left)
		return VIEWMAT_EYE_LEFT;
	else if (viewportID == ovrEye_Right)
		return VIEWMAT_EYE_RIGHT;
	else
		return VIEWMAT_EYE_UNKNOWN;
}

int dispmodeOculusWindows::num_viewports(void)
{
	return 2;
}



int dispmodeOculusWindows::get_framebuffer(int viewportID)
{
#if 0
	ovrEyeType eye = hmd->EyeRenderOrder[viewportID];
	if(eye == ovrEye_Left)
		return leftFramebuffer;
	else if(eye == ovrEye_Right)
		return rightFramebuffer;
	else
		return 0;
#endif
	return 0;
}



void dispmodeOculusWindows::get_viewport(int viewportValue[4], int viewportID)
{
	/* Oculus uses one framebuffer per eye */
	ovrEyeType eye = dispmodeOculusWindows::get_ovrEye(viewportID);

	ovrSizei size = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
	viewportValue[0] = 0;
	viewportValue[1] = 0;
	viewportValue[2] = size.w;
	viewportValue[3] = size.h;
}

void dispmodeOculusWindows::get_frustum(float result[6], int viewportID)
{
	// We don't get a view frustum from Oculus---only a projection matrix.
	msg(MSG_FATAL, "You tried to call get_frustum() on the Oculus dispmode object. Use get_projmatrix() instead.");
	exit(EXIT_FAILURE);
}

void dispmodeOculusWindows::get_projmatrix(float projmatrix[16], int viewportID)
{
	ovrEyeType eye = dispmodeOculusWindows::get_ovrEye(viewportID);

	/* Oculus doesn't provide us with easy access to the view
	* frustum information. We get the projection matrix directly
	* from libovr. */
	ovrMatrix4f ovrpersp = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[eye], 0.2f, 1000.0f, ovrProjection_None);
	mat4f_setRow(projmatrix, &(ovrpersp.M[0][0]), 0);
	mat4f_setRow(projmatrix, &(ovrpersp.M[1][0]), 1);
	mat4f_setRow(projmatrix, &(ovrpersp.M[2][0]), 2);
	mat4f_setRow(projmatrix, &(ovrpersp.M[3][0]), 3);

	//msg(MSG_BLUE, "Projmatrix:");
	//mat4f_print(projmatrix);
}
