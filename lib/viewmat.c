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
/* LibOVR defines a static_assert macro that produces a compiler
 * error. Since it isn't necessary, we redefine it as a function that
 * doesn't do anything. */
#define static_assert(a,b)
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"
/* We call this function to disable the safety warning. Since it isn't
 * declared in the headers, we need to declare it here to avoid
 * compiler warnings about implicit declarations. */
OVR_EXPORT void ovrhmd_EnableHSWDisplaySDKRender(ovrHmd hmd, ovrBool enable);

ovrHmd hmd;
GLint leftFramebuffer, rightFramebuffer, leftFramebufferAA, rightFramebufferAA;
ovrSizei recommendTexSizeL,recommendTexSizeR;
ovrGLTexture EyeTexture[2];
ovrEyeRenderDesc eye_rdesc[2];
ovrFrameTiming timing;
ovrPosef pose[2];
float oculus_initialPos[3];
union ovrGLConfig glcfg;
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
static const char *viewmat_vrpn_obj; /**< Name of the VRPN object that we are tracking */
static HmdControlState viewmat_hmd;


/** Should be called prior to rendering a frame. */
void viewmat_begin_frame(void)
{
#ifndef MISSING_OVR
	if(viewmat_mode == VIEWMAT_HMD_OCULUS)
		timing = ovrHmd_BeginFrame(hmd, 0);
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
		glBindFramebuffer(GL_READ_FRAMEBUFFER, leftFramebufferAA);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftFramebuffer);
		glBlitFramebuffer(0, 0, recommendTexSizeL.w,
		                  recommendTexSizeL.h, 0, 0, recommendTexSizeL.w,
		                  recommendTexSizeL.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, rightFramebufferAA);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightFramebuffer);
		glBlitFramebuffer(0, 0, recommendTexSizeR.w,
		                  recommendTexSizeR.h, 0, 0, recommendTexSizeR.w,
		                  recommendTexSizeR.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		kuhl_errorcheck();
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
			kuhl_errmsg("Unknown viewport ID: %d\n",viewportID);
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
			kuhl_errmsg("Unknown viewport ID: %d\n",viewportID);
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
	int windowWidth  = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
	viewports_size = 1;
	viewports[0][0] = 0;
	viewports[0][1] = 0;
	viewports[0][2] = windowWidth;
	viewports[0][3] = windowHeight;
}

static void viewmat_anaglyph_viewports()
{
	/* One viewport fills the entire screen */
	int windowWidth  = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
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
	int windowWidth  = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

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
			kuhl_errmsg("Unknown viewmat mode: %d\n", viewmat_mode);
			exit(EXIT_FAILURE);
	}
}

/** Initialize IVS view matrix calculations, connect to VRPN. */
void viewmat_init_ivs()
{
	/* Since the master process is the only one that talks to the VRPN
	 * server, slaves don't need to do anything to initialize. */
	if(dgr_is_master() == 0)
		return;
	
	const char* vrpnObjString = getenv("VIEWMAT_VRPN_OBJECT");
	if(vrpnObjString != NULL && strlen(vrpnObjString) > 0)
	{
		viewmat_vrpn_obj = vrpnObjString;
		kuhl_msg("View is following tracker object: %s\n", viewmat_vrpn_obj);
		
		/* Try to connect to VRPN server */
		float vrpnPos[3];
		float vrpnOrient[16];
		vrpn_get(viewmat_vrpn_obj, NULL, vrpnPos, vrpnOrient);
	}
	else
	{
		kuhl_errmsg("Failed to setup IVS mode\n");
		exit(EXIT_FAILURE);
	}
}


/** Initialize mouse movement. */
static void viewmat_init_mouse(float pos[3], float look[3], float up[3])
{
	glutMotionFunc(mousemove_glutMotionFunc);
	glutMouseFunc(mousemove_glutMouseFunc);
	mousemove_set(pos[0],pos[1],pos[2],
	              look[0],look[1],look[2],
	              up[0],up[1],up[2]);
	mousemove_speed(0.05, 0.5);
}

/** Initialize view matrix for "hmd" mode. */
static void viewmat_init_hmd(float pos[3], float look[3], float up[3])
{
	/* TODO: For now, we are using mouse movement for the HMD mode */
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
		kuhl_errmsg("Failed to setup dSight HMD mode, VIEWMAT_DSIGHT_FILE not set\n");
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
	kuhl_errmsg("Oculus support is missing: You have not compiled this code against the LibOVR library.\n");
	exit(EXIT_FAILURE);
#else
	ovr_Initialize();

	int useDebugMode = 0;
	hmd = ovrHmd_Create(0);
	if(!hmd)
	{
		kuhl_warnmsg("Failed to open Oculus HMD. Is oculusd running?\n");
		kuhl_warnmsg("Press any key to proceed with Oculus debugging window.\n");
		char c; 
		if(fscanf(stdin, "%c", &c) < 0)
		{
			kuhl_errmsg("fscanf error.\n");
			exit(EXIT_FAILURE);
		}


		hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
		useDebugMode = 1;
		if(!hmd)
		{
			kuhl_errmsg("Oculus: Failed to create virtual debugging HMD\n");
			exit(EXIT_FAILURE);
		}
	}
	
	kuhl_msg("Initialized HMD: %s - %s\n", hmd->Manufacturer, hmd->ProductName);

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
	trackingcap |= ovrTrackingCap_Position; // position tracking
	trackingcap |= ovrTrackingCap_MagYawCorrection;
	ovrHmd_ConfigureTracking(hmd, trackingcap, 0);

	
	unsigned int hmd_caps = 0;
	hmd_caps |= ovrHmdCap_DynamicPrediction; // enable internal latency feedback
	// disable vsync; allow frame rate higher than display refresh rate, can cause tearing
	//hmd_caps |= ovrHmdCap_NoVSync;
	hmd_caps |= ovrHmdCap_LowPersistence; // Less blur during rotation; dimmer screen
	
	ovrHmd_SetEnabledCaps(hmd, hmd_caps);

	/* Distortion options
	 * See OVR_CAPI.h for additional options
	 */
	unsigned int distort_caps = 0;
	distort_caps |= ovrDistortionCap_LinuxDevFullscreen; // Screen rotation for DK2
	distort_caps |= ovrDistortionCap_Chromatic; // Chromatic aberration correction
	distort_caps |= ovrDistortionCap_Vignette; // Apply gradient to edge of image
	// distort_caps |= ovrDistortionCap_OverDrive; // Overdrive brightness transitions to compensate for DK2 artifacts

	/* Shift image based on time difference between
	 * ovrHmd_GetEyePose() and ovrHmd_EndFrame(). This option seems to
	 * reduce FPS on at least one machine. */
	//distort_caps |= ovrDistortionCap_TimeWarp; 
	
	if(!ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc)) {
		kuhl_errmsg("Failed to configure distortion renderer.\n");
		exit(EXIT_FAILURE);
	}

	/* disable health and safety warning */
	ovrhmd_EnableHSWDisplaySDKRender(hmd, 0);

	vec3f_copy(oculus_initialPos, pos);
	
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
		kuhl_msg("Using IVS head tracking mode, tracking object: %s\n", viewmat_vrpn_obj);
	}
	else if(strcasecmp(modeString, "dsight") == 0)
	{
		viewmat_mode = VIEWMAT_HMD_DSIGHT;
		viewmat_init_hmd_dsight();
		kuhl_msg("Using dSight HMD head tracking mode. Tracking object: %s\n", viewmat_vrpn_obj);
	}
	else if(strcasecmp(modeString, "oculus") == 0)
	{
		viewmat_mode = VIEWMAT_HMD_OCULUS;
		viewmat_init_hmd_oculus(pos);
		// TODO: Tracking options?
		kuhl_msg("Using Oculus HMD head tracking mode.\n");
	}
	else if(strcasecmp(modeString, "hmd") == 0)
	{
		viewmat_mode = VIEWMAT_HMD;
		viewmat_init_hmd(pos, look, up);
		kuhl_msg("Using HMD head tracking mode. Tracking object: %s\n", viewmat_vrpn_obj);
	}
	else if(strcasecmp(modeString, "none") == 0)
	{
		kuhl_msg("No view matrix handler is being used.\n");
		viewmat_mode = VIEWMAT_NONE;
		// Set our initial position, but don't handle mouse movement.
		mousemove_set(pos[0],pos[1],pos[2],
		              look[0],look[1],look[2],
		              up[0],up[1],up[2]);
	}
	else if(strcasecmp(modeString, "anaglyph") == 0)
	{
		kuhl_msg("Anaglyph image rendering. Use the red filter on the left eye and the cyan filter on the right eye.\n");
		viewmat_mode = VIEWMAT_ANAGLYPH;
		viewmat_init_mouse(pos, look, up);
	}
	else
	{
		if(strcasecmp(modeString, "mouse") == 0) // if no mode specified, default to mouse
			kuhl_msg("Using mouse movement.\n");
		else // if an unrecognized mode was specified.
			kuhl_msg("Unrecognized VIEWMAT_MODE: %s; using mouse movement instead.\n", modeString);
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

/** Get the view matrix from the mouse.
 *
 * @param viewmatrix The view matrix to be filled in.
 *
 * @param viewportNum If mouse mode is used with a left an right
 * screen for an HMD, we need to return different view matrices for
 * the left (0) and right (1) eyes.
 **/
static void viewmat_get_mouse(float viewmatrix[16], int viewportNum)
{
	float pos[3],look[3],up[3];
	mousemove_get(pos, look, up);

	float eyeDist = 0.055;  // TODO: Make this configurable.

	float lookVec[3], rightVec[3];
	vec3f_sub_new(lookVec, look, pos);
	vec3f_normalize(lookVec);
	vec3f_cross_new(rightVec, lookVec, up);
	vec3f_normalize(rightVec);
	if(viewportNum == 0)
		vec3f_scalarMult(rightVec, -eyeDist/2.0);
	else
		vec3f_scalarMult(rightVec, eyeDist/2.0);

	vec3f_add_new(look, look, rightVec);
	vec3f_add_new(pos, pos, rightVec);

	mat4f_lookatVec_new(viewmatrix, pos, look, up);

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
	/* Oculus recommends the order that we should render eyes. We assume
	 * that smaller viewportIDs are rendered first. So, we need to map
	 * the viewportIDs to the specific Oculus HMD eye. */
	ovrEyeType eye = hmd->EyeRenderOrder[viewportID];

	pose[eye] = ovrHmd_GetHmdPosePerEye(hmd, eye);

	float offsetMat[16], rotMat[16], posMat[16];
	mat4f_identity(offsetMat);
	mat4f_identity(rotMat);
	mat4f_identity(posMat);

	/* Construct a matrix which represents the amount to offset each
	 * eye. For the DK1, this seems to only be the IPD offset. */
	mat4f_translate_new(offsetMat,
	                    eye_rdesc[eye].HmdToEyeViewOffset.x,
	                    eye_rdesc[eye].HmdToEyeViewOffset.y,
	                    eye_rdesc[eye].HmdToEyeViewOffset.z);

	
	/* Construct a rotation matrix based on the Oculus orientation
	 * sensor */
	mat4f_rotateQuat_new(rotMat,
	                     pose[eye].Orientation.x,
	                     pose[eye].Orientation.y,
	                     pose[eye].Orientation.z,
	                     pose[eye].Orientation.w);
	mat4f_transpose(rotMat); /* orientation sensor rotates camera, not world */

	/* Matrix which translates the world to account for the position
	 * of the HMD (from the Oculus tracker). */
	mat4f_translate_new(posMat,
	                    -pose[eye].Position.x,
	                    -pose[eye].Position.y,
	                    -pose[eye].Position.z);

	// Translate the world based on the initial camera position
	// specified in viewmat_init(). You may choose to initialize the
	// camera position with y=1.5 meters to approximate a normal
	// standing eyeheight.
	float initPosVec[3];
	vec3f_scalarMult_new(initPosVec, oculus_initialPos, -1.0f);
	float initPosMat[16];
	mat4f_translateVec_new(initPosMat, initPosVec);
	// TODO: Could also get eyeheight via ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, 1.65)

	if(0)
	{
		printf("ViewportID=%d; eye=%s\n", viewportID, eye == ovrEye_Left ? "left" : "right");
		printf("OVR eye offset: ");
		mat4f_print(offsetMat);
		printf("OVR rotation sensing: ");
		mat4f_print(rotMat);
		printf("OVR position tracking: ");
		mat4f_print(posMat);
		printf("Initial position: ");
		mat4f_print(initPosMat);
	}
	
	// viewmatrix = offsetMat * rotMat *  posMat * initposmat
	mat4f_mult_mat4f_new(viewmatrix, offsetMat, rotMat);
	mat4f_mult_mat4f_new(viewmatrix, viewmatrix, posMat);
	mat4f_mult_mat4f_new(viewmatrix, viewmatrix, initPosMat);
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
	float f[6]; // left, right, top, bottom, near>0, far>0
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
			break;
		default:
			kuhl_errmsg("Unknown viewmat mode: %d\n", viewmat_mode);
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
		kuhl_errmsg("You requested a viewport that does not exist.\n");
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
