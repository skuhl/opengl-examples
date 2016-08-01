/* NOTE: Some of the Oculus OVR code is based on an SDL example that
   is released into the public domain. See below for direct links to
   the code:
   
   https://codelab.wordpress.com/2014/09/07/oculusvr-sdk-and-simple-oculus-rift-dk2-opengl-test-program/
   http://nuclear.mutantstargoat.com/hg/oculus2/file/tip
*/


#if !defined(MISSING_OVR) && defined(__linux__)
#include <GL/glx.h>
#define OVR_OS_LINUX
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"

/* TODO: These should be organized into a ovr_state struct */
static ovrHmd hmd;
static GLint leftFramebuffer, rightFramebuffer, leftFramebufferAA, rightFramebufferAA;
static ovrSizei recommendTexSizeL,recommendTexSizeR;
static ovrGLTexture EyeTexture[2];
static ovrEyeRenderDesc eye_rdesc[2];
static ovrFrameTiming timing;
static ovrPosef pose[2];
static float oculus_initialPos[3];

#endif

#include <stdlib.h>
#include "msg.h"
#include "viewmat.h"
#include "vecmat.h"
#include "dispmode-oculus-linux.h"

dispmodeOculusLinux::dispmodeOculusLinux()
{
#if defined(MISSING_OVR) || !defined(__linux__)
	msg(MSG_FATAL, "Oculus-Linux support is missing: You have not compiled this code against the LibOVR library or you are not using Linux.\n");
	exit(EXIT_FAILURE);
#else
	ovr_Initialize(NULL);

	int useDebugMode = 0;
	hmd = ovrHmd_Create(0);
	if(!hmd)
	{
		msg(MSG_WARNING, "Failed to open Oculus HMD. Is ovrd running? Is libOVRRT*.so.* in /usr/lib, /usr/local/lib, or the current directory?\n");
		msg(MSG_WARNING, "Press any key to proceed with Oculus debugging window.\n");
		char c; 
		if(fscanf(stdin, "%c", &c) < 0)
		{
			msg(MSG_ERROR, "fscanf error.\n");
			exit(EXIT_FAILURE);
		}

		hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
		useDebugMode = 1;
		if(!hmd)
		{
			msg(MSG_ERROR, "Oculus: Failed to create virtual debugging HMD\n");
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

	vec3f_copy(oculus_initialPos, pos);

	// Try to connect to VRPN
	//viewmat_init_vrpn();

	
	// TODO: We are supposed to do these things when we are done:
	//ovrHmd_Destroy(hmd);
	//ovr_Shutdown();
#endif
}

void dispmodeOculusLinux::begin_frame()
{
#if !defined(MISSING_OVR) && defined(__linux__)
	if(hmd)
		timing = ovrHmd_BeginFrame(hmd, 0);
#endif
}

void dispmodeOculusLinux::end_frame()
{
#if !defined(MISSING_OVR) && defined(__linux__)
	/* Copy the prerendered image from a multisample antialiasing
	   texture into a normal OpenGL texture. This section of code
	   is not necessary if we are rendering directly into the
	   normal (non-antialiased) OpenGL texture. */
	GLuint buffersToBlit[3] = { GL_COLOR_BUFFER_BIT, GL_STENCIL_BUFFER_BIT, GL_DEPTH_BUFFER_BIT };

	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftFramebufferAA);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftFramebuffer);
	for(unsigned int i=0; i<sizeof(buffersToBlit)/sizeof(buffersToBlit[0]); i++)
		glBlitFramebuffer(0, 0, recommendTexSizeL.w,
		                  recommendTexSizeL.h, 0, 0, recommendTexSizeL.w,
		                  recommendTexSizeL.h, buffersToBlit[i], GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightFramebufferAA);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightFramebuffer);
	for(unsigned int i=0; i<sizeof(buffersToBlit)/sizeof(buffersToBlit[0]); i++)
		glBlitFramebuffer(0, 0, recommendTexSizeL.w,
		                  recommendTexSizeL.h, 0, 0, recommendTexSizeL.w,
		                  recommendTexSizeL.h, buffersToBlit[i], GL_NEAREST);
	kuhl_errorcheck();
		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if(hmd)
		ovrHmd_EndFrame(hmd, pose, &EyeTexture[0].Texture);
#endif

	dispmode::end_frame(); // call our parents implementation
}

void dispmodeOculusLinux::begin_eye(int viewportId)
{
#if !defined(MISSING_OVR) && defined(__linux__)
	if(viewmat_display_mode == VIEWMAT_OCULUS)
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
			msg(MSG_FATAL, "Unknown viewport ID: %d\n",viewportID);
			exit(EXIT_FAILURE);
		}
	}
#endif
}

viewmat_eye dispmodeOculusLinux::eye_type(int viewportID)
{
	if(viewportID != 0 && viewportID != 1)
	{
		msg(MSG_FATAL, "Invalid viewport id");
		exit(EXIT_FAILURE);
	}

#if !defined(MISSING_OVR) && defined(__linux__)
	
	ovrEyeType eye = hmd->EyeRenderOrder[viewportID];
	if(eye == ovrEye_Left)
		return VIEWMAT_EYE_LEFT;
	else if(eye == ovrEye_Right)
		return VIEWMAT_EYE_RIGHT;
	else
		return VIEWMAT_EYE_UNKNOWN;

#else

	return VIEWMAT_EYE_UNKNOWN;

#endif
}

int dispmodeOculusLinux::num_viewports(void)
{
	return 2;
}

void dispmodeOculusLinux::get_viewport(int viewportValue[4], int viewportID)
{
#if !defined(MISSING_OVR) && defined(__linux__)
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
#endif

	if(viewportID > 2 || viewportID < 0)
	{
		msg(MSG_WARNING, "Invalid viewportID=%d requested in mode", viewportID);
	}
}

void dispmodeOculusLinux::get_frustum(float result[6], int viewportID)
{
#if !defined(MISSING_OVR) && defined(__linux__)

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
	ovrMatrix4f ovrpersp = ovrMatrix4f_Projection(hmd->DefaultEyeFov[eye], 0.5, 500, 1);
	mat4f_setRow(projmatrix, &(ovrpersp.M[0][0]), 0);
	mat4f_setRow(projmatrix, &(ovrpersp.M[1][0]), 1);
	mat4f_setRow(projmatrix, &(ovrpersp.M[2][0]), 2);
	mat4f_setRow(projmatrix, &(ovrpersp.M[3][0]), 3);
	
	float offsetMat[16], rotMat[16], posMat[16], initPosMat[16];
	mat4f_identity(offsetMat);  // Viewpoint offset (IPD, etc);
	mat4f_identity(rotMat);     // tracking system rotation
	mat4f_identity(posMat);     // tracking system position
	mat4f_identity(initPosMat); // camera starting location
	
	/* Construct posMat and rotMat matrices which indicate the
	 * position and orientation of the HMD. */
	if(viewmat_vrpn_obj) // get position from VRPN
	{
		/* Get the offset for the left and right eyes from
		 * Oculus. If you are using a separate tracking system, you
		 * may also want to apply an offset here between the tracked
		 * point and the eye location. */
		mat4f_translate_new(offsetMat,
		                    eye_rdesc[eye].HmdToEyeViewOffset.x, // left & right IPD offset
		                    eye_rdesc[eye].HmdToEyeViewOffset.y, // vertical offset
		                    eye_rdesc[eye].HmdToEyeViewOffset.z); // forward/back offset

		float pos[3] = { 0,0,0 };
		vrpn_get(viewmat_vrpn_obj, NULL, pos, rotMat);
		mat4f_translate_new(posMat, -pos[0], -pos[1], -pos[2]); // position
		viewmat_fix_rotation(rotMat);
	}
	else // get position from Oculus tracker
	{
		pose[eye] = ovrHmd_GetHmdPosePerEye(hmd, eye);
		mat4f_translate_new(posMat,                           // position (includes IPD offset)
		                    -pose[eye].Position.x,
		                    -pose[eye].Position.y,
		                    -pose[eye].Position.z);
		mat4f_rotateQuat_new(rotMat,                          // rotation
		                     pose[eye].Orientation.x,
		                     pose[eye].Orientation.y,
		                     pose[eye].Orientation.z,
		                     pose[eye].Orientation.w);

		// Starting point:
		
		// Translate the world based on the initial camera position
		// specified in viewmat_init(). You may choose to initialize the
		// camera position with y=1.5 meters to approximate a normal
		// standing eyeheight.
		float initPosVec[3];
		vec3f_scalarMult_new(initPosVec, oculus_initialPos, -1.0f);
		mat4f_translateVec_new(initPosMat, initPosVec);
		// TODO: Could also get eyeheight via ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, 1.65)
	}
	mat4f_transpose(rotMat); /* orientation sensor rotates camera, not world */

	// viewmatrix = offsetMat * rotMat *  posMat * initposmat
	mat4f_mult_mat4f_new(viewmatrix, offsetMat, rotMat); // offset is identity if we are using Oculus tracker
	mat4f_mult_mat4f_new(viewmatrix, viewmatrix, posMat);
	mat4f_mult_mat4f_new(viewmatrix, viewmatrix, initPosMat);

	if(0)
	{
		printf("ViewportID=%d; eye=%s\n", viewportID, eye == ovrEye_Left ? "left" : "right");
		printf("Eye offset according to OVR (only used if VRPN is used): ");
		mat4f_print(offsetMat);
		printf("Rotation sensing (from OVR or VRPN): ");
		mat4f_print(rotMat);
		printf("Position tracking (from OVR or VRPN): ");
		mat4f_print(posMat);
		printf("Initial position (from set in viewmat_init()): ");
		mat4f_print(initPosMat);
		printf("Final view matrix: ");
		mat4f_print(viewmatrix);
	}
#endif
}
	
