/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "kuhl-util.h"
#include "vecmat.h"
#include "mousemove.h"
#include "vrpn-help.h"
#include "hmd-dsight-orient.h"
#include "dgr.h"

#include "viewmat.h"
#include "projmat.h"


#ifndef MISSING_OVR
/* NOTE: Some of the Oculus OVR code is based on an SDL example that
   is released into the public domain. See below for direct links to
   the code:
   
   https://codelab.wordpress.com/2014/09/07/oculusvr-sdk-and-simple-oculus-rift-dk2-opengl-test-program/
   http://nuclear.mutantstargoat.com/hg/oculus2/file/tip
*/

#include <GL/glx.h>
#define OVR_OS_LINUX
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"

/* TODO: These should be organized into a ovr_state struct */
ovrHmd hmd;
GLint leftFramebuffer, rightFramebuffer, leftFramebufferAA, rightFramebufferAA;
ovrSizei recommendTexSizeL,recommendTexSizeR;
ovrGLTexture EyeTexture[2];
ovrEyeRenderDesc eye_rdesc[2];
ovrFrameTiming timing;
ovrPosef pose[2];
float oculus_initialPos[3];

#endif



/** The different modes that viewmat works with. */
typedef enum
{
	VIEWMAT_MOUSE,
	VIEWMAT_IVS,        /* Michigan Tech's Immersive Visualization Studio */
	VIEWMAT_HMD,        /* Side-by-side view */
	VIEWMAT_HMD_DSIGHT, /* Sensics dSight */
	VIEWMAT_HMD_OCULUS, /* HMDs supported by libovr (Oculus DK1, DK2, etc). */
	VIEWMAT_ANAGLYPH,   /* Red-Cyan anaglyph images */
	VIEWMAT_NONE        /* No matrix handler used */
} ViewmatModeType;

#define MAX_VIEWPORTS 32 /**< Hard-coded maximum number of viewports supported. */
static float viewports[MAX_VIEWPORTS][4]; /**< Contains one or more viewports. The values are the x coordinate, y coordinate, viewport width, and viewport height */
static int viewports_size = 0; /**< Number of viewports in viewports array */
static ViewmatModeType viewmat_mode = 0; /**< 0=mousemove, 1=IVS (using VRPN), 2=HMD (using VRPN), 3=none */
static const char *viewmat_vrpn_obj = NULL; /**< Name of the VRPN object that we are tracking */
static HmdControlState viewmat_hmd;


/** Sometimes calls to glutGet(GLUT_WINDOW_*) take several milliseconds
 * to complete. To maintain a 60fps frame rate, we have a budget of
 * about 16 milliseconds per frame. These functions might get called
 * multiple times in multiple places per frame. viewmat_window_size()
 * is an alternative way to get the window size but it may be up to 1
 * second out of date.
 *
 * This causes window resizing to look a little ugly, but it is
 * functional and results in a more consistent framerate.
 *
 * @param width To be filled in with the width of the GLUT window.
 *
 * @param height To be filled in with the height of the GLUT window.
 */
void viewmat_window_size(int *width, int *height)
{
	if(width == NULL || height == NULL)
	{
		msg(ERROR, "width and/or height pointers were null.");
		exit(EXIT_FAILURE);
	}
	
	// Initialize static variables upon startup
	static int savedWidth  = -1;
	static int savedHeight = -1;
	static long savedTime = -1;

	// If it is the first time we are called and the variables are not
	// initialized, initialize them!
	int needToUpdate = 0;
	if(savedWidth < 0 || savedHeight < 0 || savedTime < 0)
		needToUpdate = 1;
	else if(kuhl_milliseconds() - savedTime > 1000)
		needToUpdate = 1;

	if(needToUpdate)
	{
		savedWidth  = glutGet(GLUT_WINDOW_WIDTH);
		savedHeight = glutGet(GLUT_WINDOW_HEIGHT);
		savedTime = kuhl_milliseconds();
		// msg(INFO, "Updated window size\n");
	}

	*width = savedWidth;
	*height = savedHeight;
}


/** Should be called prior to rendering a frame. */
void viewmat_begin_frame(void)
{
#ifndef MISSING_OVR
	if(viewmat_mode == VIEWMAT_HMD_OCULUS)
	{
		if(hmd)
			timing = ovrHmd_BeginFrame(hmd, 0);
	}
#endif
}

/** Should be called when we have completed rendering a frame. For
 * HMDs, this should be called after both the left and right eyes have
 * been rendered. */
void viewmat_end_frame(void)
{
	if(viewmat_mode == VIEWMAT_HMD_OCULUS)
	{
#ifndef MISSING_OVR
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
	}
	else if(viewmat_mode == VIEWMAT_ANAGLYPH)
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	/* Need to swap front and back buffers here unless we are using
	 * Oculus. (Oculus draws to the screen directly). */
	if(viewmat_mode != VIEWMAT_HMD_OCULUS)
		glutSwapBuffers();
}


/** If we are rendering a scene for the Oculus, we will be rendering
 * into a multisampled FBO that we are not allowed to read from. We
 * can only read after the multisampled FBO is blitted into a normal
 * FBO inside of viewmat_end_frame(). This function will retrieve the
 * binding for the normal framebuffer. The framebuffer will be from
 * the previous frame unless it is called after
 * viewmat_end_frame().
 *
 * @param viewportID The viewport that we want a framebuffer for.
 * @return An OpenGL framebuffer that we can bind to.
 */
int viewmat_get_blitted_framebuffer(int viewportID)
{
#ifndef MISSING_OVR
	if(viewmat_mode == VIEWMAT_HMD_OCULUS)
	{
		ovrEyeType eye = hmd->EyeRenderOrder[viewportID];
		if(eye == ovrEye_Left)
			return leftFramebuffer;
		else if(eye == ovrEye_Right)
			return rightFramebuffer;
		else
			return 0;
	}
#endif

	GLint framebuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,  &framebuffer);
	return framebuffer;
}


/** Changes the framebuffer (as needed) that OpenGL is rendering
 * to. Some HMDs (such as the Oculus Rift) require us to prerender the
 * left and right eye scenes to a texture. Those textures are then
 * processed and displayed on the screen. This function should be
 * called before any drawing occurs for that particular eye. Once both
 * eyes are rendered, viewmat_end_frame() will reset the bound
 * framebuffer and render the scene.
 *
 * @param viewportID The viewport number that we are rendering to. If
 * running as a single window, non-stereo desktop application, use
 * 0. For HMDs, use 0 when rendering the left eye and 1 when rendering
 * the right eye.
 */
void viewmat_begin_eye(int viewportID)
{
#ifndef MISSING_OVR
	if(viewmat_mode == VIEWMAT_HMD_OCULUS)
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
			msg(ERROR, "Unknown viewport ID: %d\n",viewportID);
			exit(EXIT_FAILURE);
		}
	}
#endif

	if(viewmat_mode == VIEWMAT_ANAGLYPH)
	{
		if(viewportID == 0)
			glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
		else if(viewportID == 1)
			glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_FALSE);
		else
		{
			msg(ERROR, "Unknown viewport ID: %d\n",viewportID);
			exit(EXIT_FAILURE);
		}
	}
}

/** Sets up viewmat to only have one viewport. This can be called
 * every frame since resizing the window will change the size of the
 * viewport! */
static void viewmat_one_viewport()
{
	/* One viewport fills the entire screen */
	int windowWidth, windowHeight;
	viewmat_window_size(&windowWidth, &windowHeight);

	viewports_size = 1;
	viewports[0][0] = 0;
	viewports[0][1] = 0;
	viewports[0][2] = windowWidth;
	viewports[0][3] = windowHeight;
}

static void viewmat_anaglyph_viewports()
{
	/* One viewport fills the entire screen */
	int windowWidth, windowHeight;
	viewmat_window_size(&windowWidth, &windowHeight);
	viewports_size = 2;
	/* Our anaglyph rendering uses parallel cameras. Changing the
	 * offset will change the distance at which objects are perceived
	 * to be at the depth of your screen. For example, a star that is
	 * infinitely far away form the parallel cameras would project
	 * onto the same pixel in each camera. However, if we displayed
	 * those two images without an offset, your eyes would have to
	 * verge to the depth of the screen to fuse the star---causing
	 * convergence cues to indicate that the star is at the depth of
	 * the screen. Offsetting the image horizontally by the
	 * inter-pupil eye distances can resolve this problem. Offsetting
	 * too should be avoided because it causes divergence.
	 *
	 * Anaglyph images may not look good because some light will get
	 * to the incorrect eye due to imperfect filters. Also, if you
	 * move the camera very close to an object, may be difficult to
	 * fuse the object. (Objects close to your eyes in the real world
	 * are hard to fuse too!).
	 *
	 * The offset parameter below is in pixels. Depending on the size
	 * the pixels on your screen, you may need to adjust this value.
	 */
	int offset = 20;
	
	for(int i=0; i<2; i++)
	{

		if(i == 0)
			viewports[i][0] = -offset/2;
		else
			viewports[i][0] = offset/2;
		viewports[i][1] = 0;
		viewports[i][2] = windowWidth;
		viewports[i][3] = windowHeight;
	}
}

/** Sets up viewmat to split the screen vertically into two
 * viewports. This can be called every frame since resizing the window
 * will change the size of the viewport! */
static void viewmat_two_viewports()
{
	/* Two viewports, one for each eye */
	int windowWidth, windowHeight;
	viewmat_window_size(&windowWidth, &windowHeight);

	/* TODO: Figure out if it makes sense to make this configurable at runtime. */
	viewports_size = 2;
	viewports[0][0] = 0;
	viewports[0][1] = 0;
	viewports[0][2] = windowWidth/2;
	viewports[0][3] = windowHeight;

	viewports[1][0] = windowWidth/2;
	viewports[1][1] = 0;
	viewports[1][2] = windowWidth/2;
	viewports[1][3] = windowHeight;
}

/** The oculus uses one framebuffer per eye. */
static void viewmat_oculus_viewports()
{
#ifndef MISSING_OVR
	int windowWidth  = EyeTexture[0].OGL.Header.RenderViewport.Size.w;
	int windowHeight = EyeTexture[0].OGL.Header.RenderViewport.Size.h;

	/* The two Oculus viewports are the same (they each fill the
	 * entire scree) because we are rendering the left and right eyes
	 * separately straight to a texture. */
	viewports_size = 2;
	for(int i=0; i<2; i++)
	{
		viewports[i][0] = 0;
		viewports[i][1] = 0;
		viewports[i][2] = windowWidth;
		viewports[i][3] = windowHeight;
	}
#endif
}


/** This method should be called regularly to ensure that we adjust
 * our viewports after a window is resized. */
static void viewmat_refresh_viewports()
{
	switch(viewmat_mode)
	{
		case VIEWMAT_MOUSE:
		case VIEWMAT_NONE:
		case VIEWMAT_IVS:
			viewmat_one_viewport();
			break;
		case VIEWMAT_HMD:
		case VIEWMAT_HMD_DSIGHT:
			viewmat_two_viewports();
			break;
		case VIEWMAT_HMD_OCULUS:
			viewmat_oculus_viewports();
			break;
		case VIEWMAT_ANAGLYPH:
			viewmat_anaglyph_viewports();
			break;
		default:
			msg(ERROR, "Unknown viewmat mode: %d\n", viewmat_mode);
			exit(EXIT_FAILURE);
	}
}

/** Checks if VIEWMAT_VRPN_OBJECT environment variable is set. If it
    is, use VRPN to control the camera position and orientation.
    
    @return Returns 1 if VRPN is set up, 0 otherwise.
*/
static int viewmat_init_vrpn()
{
	viewmat_vrpn_obj = NULL;
	
	const char* vrpnObjString = getenv("VIEWMAT_VRPN_OBJECT");
	if(vrpnObjString != NULL && strlen(vrpnObjString) > 0)
	{
		viewmat_vrpn_obj = vrpnObjString;
		msg(INFO, "View is following tracker object: %s\n", viewmat_vrpn_obj);
		
		/* Try to connect to VRPN server */
		float vrpnPos[3];
		float vrpnOrient[16];
		vrpn_get(viewmat_vrpn_obj, NULL, vrpnPos, vrpnOrient);
		return 1;
	}
	return 0;
}

/** Initialize IVS view matrix calculations, connect to VRPN. */
void viewmat_init_ivs()
{
	/* Since the master process is the only one that talks to the VRPN
	 * server, slaves don't need to do anything to initialize. Also,
	 * if we somehow aren't using DGR, assume that we need to connect to
	 * VRPN. */
	if((dgr_is_enabled() && dgr_is_master()==0) || dgr_is_enabled()!=0)
		return;

	if(viewmat_init_vrpn() == 0)
	{
		msg(ERROR, "Failed to setup IVS mode because we could not connect to VRPN.\n");
		exit(EXIT_FAILURE);
	}
}


/** Initialize mouse movement. Or, if we have a VRPN object name we
 * are supposed to be tracking, get ready to use that instead. */
static void viewmat_init_mouse(float pos[3], float look[3], float up[3])
{
	// If using VRPN to control the camera, there is nothing we need
	// to do.
	if(viewmat_init_vrpn() == 1)
		return;
		
	glutMotionFunc(mousemove_glutMotionFunc);
	glutMouseFunc(mousemove_glutMouseFunc);
	mousemove_set(pos[0],pos[1],pos[2],
	              look[0],look[1],look[2],
	              up[0],up[1],up[2]);
	mousemove_speed(0.05, 0.5);
}


/** Initialize view matrix for "dSight" mode. */
static void viewmat_init_hmd_dsight()
{
	const char* hmdDeviceFile = getenv("VIEWMAT_DSIGHT_FILE");
	if (hmdDeviceFile == NULL)
	{
		msg(ERROR, "Failed to setup dSight HMD mode, VIEWMAT_DSIGHT_FILE not set\n");
		exit(EXIT_FAILURE);
	}

	/* TODO: Currently this mode only supports the orientation sensor
	 * in the dSight HMD */
	viewmat_hmd = initHmdControl(hmdDeviceFile);
}

/** Initialize the Oculus HMD.
 *
 * @param pos The position that we want the Oculus HMD to start at.
 */
static void viewmat_init_hmd_oculus(float pos[3])
{
#ifdef MISSING_OVR
	msg(ERROR, "Oculus support is missing: You have not compiled this code against the LibOVR library.\n");
	exit(EXIT_FAILURE);
#else
	ovr_Initialize(NULL);

	int useDebugMode = 0;
	hmd = ovrHmd_Create(0);
	if(!hmd)
	{
		msg(WARNING, "Failed to open Oculus HMD. Is ovrd running? Is libOVRRT*.so.* in /usr/lib, /usr/local/lib, or the current directory?\n");
		msg(WARNING, "Press any key to proceed with Oculus debugging window.\n");
		char c; 
		if(fscanf(stdin, "%c", &c) < 0)
		{
			msg(ERROR, "fscanf error.\n");
			exit(EXIT_FAILURE);
		}

		hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
		useDebugMode = 1;
		if(!hmd)
		{
			msg(ERROR, "Oculus: Failed to create virtual debugging HMD\n");
			exit(EXIT_FAILURE);
		}
	}
	
	msg(INFO, "Initialized HMD: %s - %s\n", hmd->Manufacturer, hmd->ProductName);

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
	float pixelDensity = .5;
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


#if 0
	/* We have only tested the Oculus in Extended Desktop mode on Linux. */
	if((hmd->HmdCaps & ovrHmdCap_ExtendDesktop)) {
		/* Note: debug mode seems to cause this if-statement to put us in direct-hmd mode. */
		printf("Oculus: Running in 'extended desktop' mode\n");
	} else {
#ifdef WIN32
		ovrHmd_AttachToWindow(hmd, glcfg.OGL.Window, 0, 0);
#elif defined(OVR_OS_LINUX)
		ovrHmd_AttachToWindow(hmd, (void*)glXGetCurrentDrawable(), 0, 0);
#endif
		printf("Oculus: running in 'direct-hmd' mode\n");
	}
#endif

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
		msg(ERROR, "Failed to configure distortion renderer.\n");
		exit(EXIT_FAILURE);
	}

	/* disable health and safety warning */
	ovrHmd_DismissHSWDisplay(hmd);

	vec3f_copy(oculus_initialPos, pos);

	// Try to connect to VRPN
	viewmat_init_vrpn();

	
	// TODO: We are supposed to do these things when we are done:
	//ovrHmd_Destroy(hmd);
	//ovr_Shutdown();
#endif
}

/** Initialize viewmat and use the specified pos/look/up values as a
 * starting location when mouse movement is used.
 *
 * @param pos The position of the camera (if mouse movement is used)
 * @param look A point the camera is looking at (if mouse movement is used)
 * @param up An up vector for the camera (if mouse movement is used).
 */
void viewmat_init(float pos[3], float look[3], float up[3])
{
	const char* modeString = getenv("VIEWMAT_MODE");
	if(modeString == NULL)
		modeString = "mouse";
	
	if(strcasecmp(modeString, "ivs") == 0)
	{
		viewmat_mode = VIEWMAT_IVS;
		viewmat_init_ivs();
		msg(INFO, "Using IVS head tracking mode, tracking object: %s\n", viewmat_vrpn_obj);
	}
	else if(strcasecmp(modeString, "dsight") == 0)
	{
		viewmat_mode = VIEWMAT_HMD_DSIGHT;
		viewmat_init_hmd_dsight();
		msg(INFO, "Using dSight HMD head tracking mode. Tracking object: %s\n", viewmat_vrpn_obj);
	}
	else if(strcasecmp(modeString, "oculus") == 0)
	{
		viewmat_mode = VIEWMAT_HMD_OCULUS;
		viewmat_init_hmd_oculus(pos);
		// TODO: Tracking options?
		msg(INFO, "Using Oculus HMD head tracking mode.\n");
	}
	else if(strcasecmp(modeString, "hmd") == 0)
	{
		viewmat_mode = VIEWMAT_HMD;
		viewmat_init_mouse(pos, look, up);
		msg(INFO, "Using HMD head tracking mode. Tracking object: %s\n", viewmat_vrpn_obj);
	}
	else if(strcasecmp(modeString, "none") == 0)
	{
		msg(INFO, "No view matrix handler is being used.\n");
		viewmat_mode = VIEWMAT_NONE;
		// Set our initial position, but don't handle mouse movement.
		mousemove_set(pos[0],pos[1],pos[2],
		              look[0],look[1],look[2],
		              up[0],up[1],up[2]);
	}
	else if(strcasecmp(modeString, "anaglyph") == 0)
	{
		msg(INFO, "Anaglyph image rendering. Use the red filter on the left eye and the cyan filter on the right eye.\n");
		viewmat_mode = VIEWMAT_ANAGLYPH;
		viewmat_init_mouse(pos, look, up);
	}
	else
	{
		if(strcasecmp(modeString, "mouse") == 0) // if no mode specified, default to mouse
			msg(INFO, "Using mouse movement.\n");
		else // if an unrecognized mode was specified.
			msg(ERROR, "Unrecognized VIEWMAT_MODE: %s; using mouse movement instead.\n", modeString);
		viewmat_mode = VIEWMAT_MOUSE;
		viewmat_init_mouse(pos, look, up);
	}

	viewmat_refresh_viewports();

	// If there are two "viewports" then it is likely that we are
	// doing stereoscopic rendering. Displaying the mouse cursor can
	// interfere with stereo images, so we disable the cursor here.
	if(viewports_size == 2)
		glutSetCursor(GLUT_CURSOR_NONE);
}

/** Some VRPN orientation sensors may be rotated differently than what we
 * expect them to be (for example, orientation is correct except that
 * the camera is pointing in the wrong direction). This function will
 * adjust the orientation matrix so that the camera is pointing in the
 * correct direction. */
static void viewmat_fix_rotation(float orient[16])
{
	if(viewmat_vrpn_obj == NULL || strlen(viewmat_vrpn_obj) == 0)
		return;

	char *hostname = vrpn_default_host();
	if(hostname == NULL)
		return;
	
	/* Currently, the "DK2" object over in the IVS lab is rotated by
	 * approx 90 degrees. Apply the fix here. */
	if(strcmp(viewmat_vrpn_obj, "DK2") == 0 &&
	   strlen(hostname) > 14 && strncmp(hostname, "tcp://141.219.", 14) == 0) // MTU vicon tracker
	{
		// The tracked object is oriented the wrong way in the IVS lab.
		float offsetVicon[16];
		mat4f_identity(offsetVicon);
		mat4f_rotateAxis_new(offsetVicon, 90, 1,0,0);
		// orient = orient * offsetVicon
		mat4f_mult_mat4f_new(orient, orient, offsetVicon);
	}
	if(hostname)
		free(hostname);
}


/** Get the view matrix from the mouse. Or, if a VRPN object is
 * specified, get the view matrix from VRPN.
 *
 * @param viewmatrix The view matrix to be filled in.
 *
 * @param viewportNum If mouse mode is used with a left an right
 * screen for an HMD, we need to return different view matrices for
 * the left (0) and right (1) eyes.
 **/
static void viewmat_get_mouse(float viewmatrix[16], int viewportNum)
{
	float eyeDist = 0.055;  // TODO: Make this configurable.
	
	if(viewmat_vrpn_obj == NULL) // if no VRPN object specified, use mouse movement
	{
		float pos[3],look[3],up[3];
		mousemove_get(pos, look, up);

		float lookVec[3], rightVec[3];
		vec3f_sub_new(lookVec, look, pos);
		vec3f_normalize(lookVec);
		vec3f_cross_new(rightVec, lookVec, up);
		vec3f_normalize(rightVec);

		if(viewports_size == 2) // if 2 viewports, use left/right IPD offset
		{
			if(viewportNum == 0)
				vec3f_scalarMult(rightVec, -eyeDist/2.0);
			else
				vec3f_scalarMult(rightVec, eyeDist/2.0);

			vec3f_add_new(look, look, rightVec);
			vec3f_add_new(pos, pos, rightVec);
		}

		mat4f_lookatVec_new(viewmatrix, pos, look, up);
	}
	else // if VRPN object is specified, use that instead
	{
		float pos[3], orient[16];
		vrpn_get(viewmat_vrpn_obj, NULL, pos, orient);

		float pos4[4] = {pos[0],pos[1],pos[2],1};
		viewmat_fix_rotation(orient);
		mat4f_copy(viewmatrix, orient);

		if(viewports_size == 2) // use left/right offset if 2 viewports are used
		{
			float rightVec[4];
			mat4f_getColumn(rightVec, orient, 0);
			if(viewportNum == 0)
				vec3f_scalarMult(rightVec, -eyeDist/2.0);
			else
				vec3f_scalarMult(rightVec, eyeDist/2.0);
			
			vec4f_add_new(pos4, rightVec, pos4);
			pos4[3] = 1;
		}
			
		mat4f_setColumn(viewmatrix, pos4, 3);
		mat4f_invert(viewmatrix);
	}

	if(viewportNum == 0)
		dgr_setget("!!viewMat0", viewmatrix, sizeof(float)*16);
	else
		dgr_setget("!!viewMat1", viewmatrix, sizeof(float)*16);
}

/** Get the view matrix for the dSight HMD. */
static void viewmat_get_hmd_dsight(float viewmatrix[16], int viewportNum)
{
	float quaternion[4];
	updateHmdControl(&viewmat_hmd, quaternion);

	float rotationMatrix[16];
	mat4f_rotateQuatVec_new(rotationMatrix, quaternion);

	float eyeDist = 0.055;  // TODO: Make this configurable.

	float xShift = (viewportNum == 0 ? -eyeDist : eyeDist) / 2.0f;

	float shiftMatrix[16];
	mat4f_translate_new(shiftMatrix, xShift, 0, 0);

	// apply the shift, then the rotation, then the position
	// as in (Pos * (Rotation * (Shift * (vector))) == (Pos * Rotation * Shift) * vector
	// except we have no way of acquiring the Pos matrix, so leave it out
	mat4f_mult_mat4f_new(viewmatrix, rotationMatrix, shiftMatrix);
	// Don't need to use DGR!
}

void viewmat_get_hmd_oculus(float viewmatrix[16], int viewportID)
{
#ifndef MISSING_OVR
	/* Oculus recommends the order that we should render eyes. We
	 * assume that smaller viewportIDs are rendered first. So, we need
	 * to map the viewportIDs to the specific Oculus HMD eye. The
	 * "eye" variable will be set to either ovrEye_Left (if we are
	 * rendering the left eye) or ovrEye_Right (if we are rendering
	 * the right eye). */
	ovrEyeType eye = hmd->EyeRenderOrder[viewportID];

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

/** Get a view matrix from VRPN and adjust the view frustum
 * appropriately.
 * @param viewmatrix The location where the viewmatrix should be stored.
 * @param frustum The location of the view frustum that should be adjusted.
 */
void viewmat_get_ivs(float viewmatrix[16], float frustum[6])
{
	float pos[3];
	if((dgr_is_enabled() && dgr_is_master()) || dgr_is_enabled()==0)
	{
		/* get information from vrpn */
		float orient[16];
		vrpn_get(viewmat_vrpn_obj, NULL, pos, orient);
	}
	/* Make sure all DGR hosts can get the position so that they
	 * can update the frustum appropriately */
	dgr_setget("!!viewMatPos", pos, sizeof(float)*3);

	/* Update view frustum if it was provided. */
	if(frustum != NULL)
	{
		frustum[0] -= pos[0];
		frustum[1] -= pos[0];
		frustum[2] -= pos[1];
		frustum[3] -= pos[1];
		frustum[4] += pos[2];
		frustum[5] += pos[2];
	}

	/* calculate a lookat point */
	float lookat[3];
	float forwardVec[3] = { 0, 0, -1 };
	for(int i=0; i<3; i++)
		lookat[i] = pos[i]+forwardVec[i];
		
	float up[3] = {0, 1, 0};
	mat4f_lookatVec_new(viewmatrix, pos, lookat, up);

}

/** Performs a sanity check on how long it took us to render a
 * frame. At 60fps, we have approximately 16 milliseconds or 16000
 * microseconds per frame. If the time between two subsequent
 * renderings of viewportID 0 is too large, a warning message is
 * printed.
 *
 * Even though the average FPS over a period of time may be above 60,
 * the rendering might not appear smooth if an occasional frame misses
 * the time budget.
 */
static void viewmat_validate_fps(int viewportID)
{
	/* Set the time budget. If vblank syncing is turned on, we'd
	 * expect to always get a FPS close to the monitor. Setting this
	 * to 55 (instead of 60) will prevent messages from getting
	 * printed out constantly on such machines. */
	static const int targetFps = 55;
	static const int timeBudget = 1000000.0f / targetFps;
	
	if(viewportID > 0)
		return;

	/* Initialize our warning message counter and the time that the
	 * last frame was rendered. */
	static int warnMsgCount = 0;
	static long lastTime = -1;
	if(lastTime < 0) // if our first time called, initialize time and return.
	{
		lastTime = kuhl_microseconds();
		return;
	}

	/* If it took too long to render the frame, print a message. */
	long delay = kuhl_microseconds() - lastTime;
	// msg(INFO, "Time to render frame %d\n", delay);
	if(delay > timeBudget)
	{
		warnMsgCount++;

		/* Don't print the message if the first few frames took too
		 * long to render. Also, eventually stop printing the
		 * message. */
		if(warnMsgCount > 5 && warnMsgCount <= 100)
			msg(WARNING, "It took %ld microseconds to render a frame. Time budget for %d fps is %d microseconds.\n", delay, targetFps, timeBudget);
		if(warnMsgCount == 100)
			msg(WARNING, "That was your last warning about the time budget per frame.\n");
	}

	lastTime = kuhl_microseconds();
}


/** Performs a sanity check on the IPD to ensure that it is not too small, big, or reversed.

 @param viewmatrix View matrix for the viewportID
 @param viewportID The viewportID for this particular view matrix.
*/
static void viewmat_validate_ipd(float viewmatrix[16], int viewportID)
{
	// First, if viewportID=0, save the matrix so we can do the check when we are called with viewportID=1.
	static float viewmatrix0[16];
	static long viewmatrix0time;
	if(viewportID == 0)
	{
		mat4f_copy(viewmatrix0, viewmatrix);
		viewmatrix0time = kuhl_microseconds();
		return;
	}

	// If rendering viewportID == 1, and if there are two viewports,
	// assume that we are running in a stereoscopic configuration and
	// validate the IPD value.
	if(viewportID == 1 && viewports_size == 2)
	{
		/* In most cases, viewportID=0 is the left eye. However,
		 * Oculus may cause this to get swapped. */
		float flip = 1;
#ifndef MISSING_OVR
		if(viewmat_mode == VIEWMAT_HMD_OCULUS &&
		   hmd->EyeRenderOrder[0] == ovrEye_Right)
			flip = -1;
#endif

		// Get the position matrix information
		float pos1[4], pos2[4];
		mat4f_getColumn(pos1, viewmatrix0, 3); // get last column
		mat4f_getColumn(pos2, viewmatrix,  3); // get last column

		// Get a vector between the eyes
		float diff[4];
		vec4f_sub_new(diff, pos1, pos2);
		vec4f_scalarMult_new(diff, diff, flip); // flip vector if necessary

		/* This message may be triggered if a person is moving quickly
		 * and/or when the FPS is low. This happens because the
		 * position/orientation of the head changed between the
		 * rendering of the left and right eyes. */
		float ipd = diff[0];
		long delay = kuhl_microseconds() - viewmatrix0time;
		if(ipd > .07 || ipd < .05)
		{
			msg(WARNING, "IPD=%.4f meters, delay=%ld us (IPD validation failed; occasional messages are OK!)\n", ipd, delay);
		}
		// msg(INFO, "IPD=%.4f meters, delay=%ld us\n", ipd, delay);

	}
}


/** Get a 4x4 view matrix. Some types of systems also need to update
 * the frustum based on where the virtual camera is. For example, on
 * the IVS display wall, the frustum is adjusted dynamically based on
 * where a person is relative to the screens.
 *
 * @param viewmatrix A 4x4 view matrix for viewmat to fill in.
 *
 * @param projmatrix A 4x4 projection matrix for viewmat to fill in.
 *
 * @param viewportID If there is only one viewport, set this to 0. If
 * the system uses multiple viewports (i.e., HMD), then set this to 0
 * or 1 to get the view matrices for the left and right eyes.
 *
 */
void viewmat_get(float viewmatrix[16], float projmatrix[16], int viewportID)
{
	int viewport[4]; // x,y of lower left corner, width, height
	viewmat_get_viewport(viewport, viewportID);

	/* Get the view or camera matrix; update the frustum values if needed. */
	float f[6]; // left, right, bottom, top, near>0, far>0
	projmat_get_frustum(f, viewport[2], viewport[3]);

	switch(viewmat_mode)
	{
		case VIEWMAT_MOUSE:    // mouse movement
		case VIEWMAT_ANAGLYPH: // red/cyan anaglyph
		case VIEWMAT_HMD:      // side-by-side HMD view
			viewmat_get_mouse(viewmatrix, viewportID);
			break;
		case VIEWMAT_IVS: // IVS display wall
			// frustum is updated based on head tracking
			viewmat_get_ivs(viewmatrix, f);
			break;
		case VIEWMAT_HMD_DSIGHT: // dSight HMD
			viewmat_get_hmd_dsight(viewmatrix, viewportID);
			break;
		case VIEWMAT_HMD_OCULUS:
			viewmat_get_hmd_oculus(viewmatrix, viewportID);
			break;
		case VIEWMAT_NONE:
			// Get the view matrix from the mouse movement code...but
			// we haven't registered mouse movement callback
			// functions, so mouse movement won't work.
			viewmat_get_mouse(viewmatrix, viewportID);
			break;
		default:
			msg(ERROR, "Unknown viewmat mode: %d\n", viewmat_mode);
			exit(EXIT_FAILURE);
	}
		
	/* The following code calculates the projection matrix. */

	if(viewmat_mode == VIEWMAT_HMD_OCULUS)
	{
#ifndef MISSING_OVR
		/* Oculus doesn't provide us with easy access to the view
		 * frustum information. We get the projection matrix directly
		 * from libovr. */
		ovrMatrix4f ovrpersp = ovrMatrix4f_Projection(hmd->DefaultEyeFov[viewportID], 0.5, 500, 1);
		mat4f_setRow(projmatrix, &(ovrpersp.M[0][0]), 0);
		mat4f_setRow(projmatrix, &(ovrpersp.M[1][0]), 1);
		mat4f_setRow(projmatrix, &(ovrpersp.M[2][0]), 2);
		mat4f_setRow(projmatrix, &(ovrpersp.M[3][0]), 3);
#endif
	}
	else
	{
		mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
	}

	viewmat_validate_ipd(viewmatrix, viewportID);
	viewmat_validate_fps(viewportID);
}

/** Gets the viewport information for a particular viewport.

 @param viewportValue A location to be filled in with the viewport x
 coordinate, y coordinate, width and height.

 @param viewportNum Which viewport number is being requested. If you
 are using only one viewport, set this to 0.
*/
void viewmat_get_viewport(int viewportValue[4], int viewportNum)
{
	viewmat_refresh_viewports();
	
	if(viewportNum >= viewports_size)
	{
		msg(ERROR, "You requested a viewport that does not exist.\n");
		exit(EXIT_FAILURE);
	}

	/* Copy the viewport into the location the caller provided. */
	for(int i=0; i<4; i++)
		viewportValue[i] = viewports[viewportNum][i];

}

/** Returns the number of viewports that viewmat has.

    @return The number of viewports that viewmat has.
*/
int viewmat_num_viewports()
{
	viewmat_refresh_viewports();
	return viewports_size;
}
