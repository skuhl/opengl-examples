/* NOTE: Some of the Oculus OVR code is based on an SDL example that
   is released into the public domain. See below for direct links to
   the code:
   
   https://codelab.wordpress.com/2014/09/07/oculusvr-sdk-and-simple-oculus-rift-dk2-opengl-test-program/
   http://nuclear.mutantstargoat.com/hg/oculus2/file/tip
*/


#include <GL/glew.h>
#include <GL/glx.h>

#include <stdlib.h>
#include "kuhl-util.h"
#include "msg.h"
#include "viewmat.h"
#include "vecmat.h"
#include "dispmode-oculus-linux.h"

dispmodeOculusLinux::dispmodeOculusLinux()
{
	ovr_Initialize(NULL);

	int useDebugMode = 0;
	hmd = ovrHmd_Create(0);
	if(!hmd)
	{
		msg(MSG_ERROR, "Failed to open Oculus HMD, trying to open debug window instead. Is ovrd running? Is libOVRRT*.so.* in /usr/lib, /usr/local/lib, or the current directory?\n");

		hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
		useDebugMode = 1;
		if(!hmd)
		{
			msg(MSG_FATAL, "Oculus: Failed to create virtual debugging HMD\n");
			exit(EXIT_FAILURE);
		}
	}
	
	msg(MSG_INFO, "Initialized HMD: %s - %s\n", hmd->Manufacturer, hmd->ProductName);

#if 0
	printf("default fov tangents left eye:\n");
	printf("up=%f\n", hmd->DefaultEyeFov[ovrEye_Left].UpTan);
	printf("up=%f\n", hmd->DefaultEyeFov[ovrEye_Left].DownTan);
	printf("up=%f\n", hmd->DefaultEyeFov[ovrEye_Left].LeftTan);
	printf("up=%f\n", hmd->DefaultEyeFov[ovrEye_Left].RightTan);
#endif
	

	/* pixelDensity can range between 0 to 1 (where 1 has the highest
	 * resolution). Using smaller values will result in smaller
	 * textures that each eye is rendered into. */
	float pixelDensity = 1;
	/* Number of multisample antialiasing while rendering the scene
	 * for each eye. */
	GLint msaa_samples = 2;
	recommendTexSizeL = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left,  hmd->DefaultEyeFov[ovrEye_Left],  pixelDensity);
	recommendTexSizeR = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[ovrEye_Right], pixelDensity);
	
	GLuint leftTextureAA,rightTextureAA;
	leftFramebufferAA  = kuhl_gen_framebuffer_msaa(recommendTexSizeL.w, recommendTexSizeL.h, &leftTextureAA, NULL, msaa_samples);
	rightFramebufferAA = kuhl_gen_framebuffer_msaa(recommendTexSizeR.w, recommendTexSizeR.h, &rightTextureAA, NULL, msaa_samples);
	GLuint leftTexture,rightTexture;
	leftFramebuffer  = kuhl_gen_framebuffer(recommendTexSizeL.w, recommendTexSizeL.h, &leftTexture,  NULL);
	rightFramebuffer = kuhl_gen_framebuffer(recommendTexSizeR.w, recommendTexSizeR.h, &rightTexture, NULL);
	//printf("Left recommended texture size: %d %d\n", recommendTexSizeL.w, recommendTexSizeL.h);
	//printf("Right recommended texture size: %d %d\n", recommendTexSizeR.w, recommendTexSizeR.h);

	EyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
	EyeTexture[0].OGL.Header.TextureSize.w = recommendTexSizeL.w;
	EyeTexture[0].OGL.Header.TextureSize.h = recommendTexSizeL.h;
	EyeTexture[0].OGL.Header.RenderViewport.Pos.x = 0;
	EyeTexture[0].OGL.Header.RenderViewport.Pos.y = 0;
	EyeTexture[0].OGL.Header.RenderViewport.Size.w = recommendTexSizeL.w;
	EyeTexture[0].OGL.Header.RenderViewport.Size.h = recommendTexSizeL.h;

	EyeTexture[1].OGL.Header.API = ovrRenderAPI_OpenGL;
	EyeTexture[1].OGL.Header.TextureSize.w = recommendTexSizeR.w;
	EyeTexture[1].OGL.Header.TextureSize.h = recommendTexSizeR.h;
	EyeTexture[1].OGL.Header.RenderViewport.Pos.x = 0;
	EyeTexture[1].OGL.Header.RenderViewport.Pos.y = 0;
	EyeTexture[1].OGL.Header.RenderViewport.Size.w = recommendTexSizeR.w;
	EyeTexture[1].OGL.Header.RenderViewport.Size.h = recommendTexSizeR.h;

	EyeTexture[0].OGL.TexId = leftTexture;
	EyeTexture[1].OGL.TexId = rightTexture;

	union ovrGLConfig glcfg;
	memset(&glcfg, 0, sizeof(glcfg));
	glcfg.OGL.Header.API=ovrRenderAPI_OpenGL;
	glcfg.OGL.Header.Multisample = 0;
	glcfg.OGL.Disp = glXGetCurrentDisplay();
	
	if(hmd->Type == ovrHmd_DK2 && useDebugMode == 0)
	{
		/* Since the DK2 monitor is rotated, we need to swap the width
		 * and height here so that the final image correctly fills the
		 * entire screen. */
		glcfg.OGL.Header.BackBufferSize.h=hmd->Resolution.w;
		glcfg.OGL.Header.BackBufferSize.w=hmd->Resolution.h;
	} else
	{
		glcfg.OGL.Header.BackBufferSize.h=hmd->Resolution.h;
		glcfg.OGL.Header.BackBufferSize.w=hmd->Resolution.w;
	}
// interferes with PROJAT_FULLSCREEN
//	glutReshapeWindow(glcfg.OGL.Header.BackBufferSize.w,
//	                  glcfg.OGL.Header.BackBufferSize.h);

	unsigned int trackingcap = 0;
	trackingcap |= ovrTrackingCap_Orientation; // orientation tracking
	trackingcap |= ovrTrackingCap_Position;    // position tracking
	trackingcap |= ovrTrackingCap_MagYawCorrection; // use magnetic compass
	ovrHmd_ConfigureTracking(hmd, trackingcap, 0);

	
	unsigned int hmd_caps = 0;
	hmd_caps |= ovrHmdCap_DynamicPrediction; // enable internal latency feedback
	
	/* disable vsync; allow frame rate higher than display refresh
	   rate, can cause tearing. On some windowing systems, you using
	   this setting reduces issues with overrunning the time budget
	   and tearing still does not occur. */
	hmd_caps |= ovrHmdCap_NoVSync;
	hmd_caps |= ovrHmdCap_LowPersistence; // Less blur during rotation; dimmer screen
	
	ovrHmd_SetEnabledCaps(hmd, hmd_caps);

	/* Distortion options
	 * See OVR_CAPI.h for additional options
	 */
	unsigned int distort_caps = 0;
	distort_caps |= ovrDistortionCap_LinuxDevFullscreen; // Screen rotation for DK2
	// distort_caps |= ovrDistortionCap_Chromatic; // Chromatic aberration correction - Necessary for 0.4.4, turned on permanently in 0.5.0.1
	distort_caps |= ovrDistortionCap_Vignette; // Apply gradient to edge of image
	// distort_caps |= ovrDistortionCap_OverDrive; // Overdrive brightness transitions to compensate for DK2 artifacts

	/* Shift image based on time difference between
	 * ovrHmd_GetEyePose() and ovrHmd_EndFrame(). This option seems to
	 * reduce FPS on at least one machine. */
	//distort_caps |= ovrDistortionCap_TimeWarp; 
	
	if(!ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc)) {
		msg(MSG_FATAL, "Failed to configure distortion renderer.\n");
		exit(EXIT_FAILURE);
	}

	/* disable health and safety warning */
	ovrHmd_DismissHSWDisplay(hmd);

	/* Warn the user that we are using the IPD specified by Oculus. */
	if(kuhl_config_isset("ipd"))
	{
		msg(MSG_WARNING, "You specified 'ipd=%s' in the config file. We are IGNORING this value because the Oculus API calculates the IPD for us.", kuhl_config_get("ipd"));

		float offsetLeft[3];
		float offsetRight[3];
		get_eyeoffset(offsetLeft, VIEWMAT_EYE_LEFT);
		get_eyeoffset(offsetRight, VIEWMAT_EYE_RIGHT);
		float offsetDiff[3];
		vec3f_sub_new(offsetDiff, offsetRight, offsetLeft);
		msg(MSG_WARNING, "The Oculus API is telling us to use %0.3f cm for the IPD.", offsetDiff[0]*100);
	}
}

dispmodeOculusLinux::~dispmodeOculusLinux()
{
	// We are supposed to do these things when we are done:
	ovrHmd_Destroy(hmd);
	ovr_Shutdown();
}

void dispmodeOculusLinux::begin_frame()
{
	if(hmd)
		timing = ovrHmd_BeginFrame(hmd, 0);
}

void dispmodeOculusLinux::end_frame()
{
	/* Copy the prerendered image from a multisample antialiasing
	   texture into a normal OpenGL texture. This section of code
	   is not necessary if we are rendering directly into the
	   normal (non-antialiased) OpenGL texture. */
	static const GLuint buffersToBlit[3] = { GL_COLOR_BUFFER_BIT, GL_STENCIL_BUFFER_BIT, GL_DEPTH_BUFFER_BIT };

	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftFramebufferAA);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftFramebuffer);
	for(unsigned int i=0; i<sizeof(buffersToBlit)/sizeof(buffersToBlit[0]); i++)
		glBlitFramebuffer(0, 0, recommendTexSizeL.w,
		                  recommendTexSizeL.h, 0, 0, recommendTexSizeL.w,
		                  recommendTexSizeL.h, buffersToBlit[i], GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightFramebufferAA);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightFramebuffer);
	for(unsigned int i=0; i<sizeof(buffersToBlit)/sizeof(buffersToBlit[0]); i++)
		glBlitFramebuffer(0, 0, recommendTexSizeR.w,
		                  recommendTexSizeR.h, 0, 0, recommendTexSizeR.w,
		                  recommendTexSizeR.h, buffersToBlit[i], GL_NEAREST);
	kuhl_errorcheck();
		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(hmd)
		ovrHmd_EndFrame(hmd, pose, &EyeTexture[0].Texture);

	dispmode::end_frame(); // call our parent's implementation
}

void dispmodeOculusLinux::begin_eye(int viewportID)
{
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
}

viewmat_eye dispmodeOculusLinux::eye_type(int viewportID)
{
	if(viewportID != 0 && viewportID != 1)
	{
		msg(MSG_FATAL, "Invalid viewport ID: %", viewportID);
		exit(EXIT_FAILURE);
	}

	ovrEyeType eye = hmd->EyeRenderOrder[viewportID];
	if(eye == ovrEye_Left)
		return VIEWMAT_EYE_LEFT;
	else if(eye == ovrEye_Right)
		return VIEWMAT_EYE_RIGHT;
	else
		return VIEWMAT_EYE_UNKNOWN;
}

int dispmodeOculusLinux::num_viewports(void)
{
	return 2;
}


void dispmodeOculusLinux::get_eyeoffset(float offset[3], viewmat_eye eye)
{
	/* IMPORTANT: If we are using the Oculus camcontrol, then this
	 * function shouldn't get called because get_separate() will
	 * already return the adjusted value.
	 */

	ovrEyeType oeye;
	if(eye == VIEWMAT_EYE_LEFT)
		oeye = ovrEye_Left;
	else if(eye == VIEWMAT_EYE_RIGHT)
		oeye = ovrEye_Right;
	else
	{
		msg(MSG_FATAL, "Requested eye offset of something that wasn't the left or right eye");
		exit(EXIT_FAILURE);
	}

	/* We negate the values because the documentation for
	 * HmdToEyeViewOffset says that the values represent how much to
	 * translate the viewmatrix (not how much to translate the
	 * eye). */
	vec3f_set(offset, 
	          -eye_rdesc[oeye].HmdToEyeViewOffset.x, // left & right IPD offset
	          -eye_rdesc[oeye].HmdToEyeViewOffset.y, // vertical offset
	          -eye_rdesc[oeye].HmdToEyeViewOffset.z); // forward/back offset

	//vec3f_print(offset);
}


int dispmodeOculusLinux::get_framebuffer(int viewportID)
{
	ovrEyeType eye = hmd->EyeRenderOrder[viewportID];
	if(eye == ovrEye_Left)
		return leftFramebuffer;
	else if(eye == ovrEye_Right)
		return rightFramebuffer;
	else
		return 0;
}



void dispmodeOculusLinux::get_viewport(int viewportValue[4], int viewportID)
{
	/* Oculus uses one framebuffer per eye */
	int windowWidth  = EyeTexture[0].OGL.Header.RenderViewport.Size.w;
	int windowHeight = EyeTexture[0].OGL.Header.RenderViewport.Size.h;

	/* The two Oculus viewports are the same (they each fill the
	 * entire 'screen') because we are rendering the left and right eyes
	 * separately straight to a texture. */
	viewportValue[0] = 0;
	viewportValue[1] = 0;
	viewportValue[2] = windowWidth;
	viewportValue[3] = windowHeight;

	if(viewportID > 2 || viewportID < 0)
	{
		msg(MSG_WARNING, "Invalid viewportID=%d requested in mode", viewportID);
	}
}

void dispmodeOculusLinux::get_frustum(float result[6], int viewportID)
{
	// We don't get a view frustum from Oculus---only a projection matrix.
	msg(MSG_FATAL, "You tried to call get_frustum() on the Oculus dispmode object. Use get_projmatrix() instead.");
	exit(EXIT_FAILURE);
}

void dispmodeOculusLinux::get_projmatrix(float projmatrix[16], int viewportID)
{
	/* Oculus recommends the order that we should render eyes. We
	 * assume that smaller viewportIDs are rendered first. So, we need
	 * to map the viewportIDs to the specific Oculus HMD eye. The
	 * "eye" variable will be set to either ovrEye_Left (if we are
	 * rendering the left eye) or ovrEye_Right (if we are rendering
	 * the right eye). */
	ovrEyeType eye = hmd->EyeRenderOrder[viewportID];

	/* Oculus doesn't provide us with easy access to the view
	 * frustum information. We get the projection matrix directly
	 * from libovr. */
	ovrMatrix4f ovrpersp = ovrMatrix4f_Projection(hmd->DefaultEyeFov[eye],
	                                              kuhl_config_float("nearplane", 0.1f, 0.1f),
	                                              kuhl_config_float("farplane", 200.0f, 200.0f),
	                                              1);
	mat4f_setRow(projmatrix, &(ovrpersp.M[0][0]), 0);
	mat4f_setRow(projmatrix, &(ovrpersp.M[1][0]), 1);
	mat4f_setRow(projmatrix, &(ovrpersp.M[2][0]), 2);
	mat4f_setRow(projmatrix, &(ovrpersp.M[3][0]), 3);
}
