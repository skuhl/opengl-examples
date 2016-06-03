/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include <unistd.h> // usleep()
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "kuhl-util.h"
#include "vecmat.h"
#include "mousemove.h"
#include "vrpn-help.h"
#include "orient-sensor.h"
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
static ovrHmd hmd;
static GLint leftFramebuffer, rightFramebuffer, leftFramebufferAA, rightFramebufferAA;
static ovrSizei recommendTexSizeL,recommendTexSizeR;
static ovrGLTexture EyeTexture[2];
static ovrEyeRenderDesc eye_rdesc[2];
static ovrFrameTiming timing;
static ovrPosef pose[2];
static float oculus_initialPos[3];

#endif



/** The display mode specifies how images are drawn to the screen. */
typedef enum
{
	VIEWMAT_DESKTOP,    /* Single window */
	VIEWMAT_IVS,        /* Michigan Tech's Immersive Visualization Studio */
	VIEWMAT_HMD,        /* Side-by-side view */
	VIEWMAT_OCULUS,     /* HMDs supported by libovr (Oculus DK1, DK2, etc). */
	VIEWMAT_ANAGLYPH,   /* Red-Cyan anaglyph images */
} ViewmatDisplayMode;
static ViewmatDisplayMode viewmat_display_mode = 0; /**< Currently active display mode */

/** The control mode specifies how the viewpoint position/orientation is determined. */
typedef enum
{
	VIEWMAT_CONTROL_NONE,
	VIEWMAT_CONTROL_MOUSE,
	VIEWMAT_CONTROL_VRPN,
	VIEWMAT_CONTROL_ORIENT,
	VIEWMAT_CONTROL_OCULUS
} ViewmatControlMode;
static ViewmatControlMode viewmat_control_mode = 0; /**< Currently active control mode */


#define MAX_VIEWPORTS 32 /**< Hard-coded maximum number of viewports supported. */
static float viewports[MAX_VIEWPORTS][4]; /**< Contains one or more viewports. The values are the x coordinate, y coordinate, viewport width, and viewport height */
static int viewports_size = 0; /**< Number of viewports in viewports array */
static const char *viewmat_vrpn_obj = NULL; /**< Name of the VRPN object that we are tracking */
static OrientSensorState viewmat_orientsense;

static int viewmat_swapinterval = 1;



/** TODO: Update for switch to GLFW:

 * Sometimes calls to glutGet(GLUT_WINDOW_*) take several milliseconds
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
		msg(MSG_ERROR, "width and/or height pointers were null.");
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
		glfwGetFramebufferSize(kuhl_get_window(), &savedWidth, &savedHeight);
		//savedWidth  = glutGet(GLUT_WINDOW_WIDTH);
		//savedHeight = glutGet(GLUT_WINDOW_HEIGHT);
		savedTime = kuhl_milliseconds();
		// msg(MSG_INFO, "Updated window size\n");
	}

	*width = savedWidth;
	*height = savedHeight;
}

/** Returns the aspect ratio of the current GLFW window. */
float viewmat_window_aspect_ratio(void)
{
	int w,h;
	viewmat_window_size(&w, &h);
	return w/(float)h;
}


/** Checks if the viewportID is an appropriate value. Exits if it is
 * invalid.
 */
static void viewmat_validate_viewportId(int viewportID)
{
	if(viewportID < 0 && viewportID >= viewports_size)
	{
		msg(MSG_FATAL, "Viewport %d does not exist. Number of viewports: %d\n",
		    viewportID, viewports_size);
		exit(EXIT_FAILURE);
	}
}



/** Should be called prior to rendering a frame. */
void viewmat_begin_frame(void)
{
#ifndef MISSING_OVR
	if(viewmat_display_mode == VIEWMAT_OCULUS)
	{
		if(hmd)
			timing = ovrHmd_BeginFrame(hmd, 0);
	}
#endif
}

/** Guesses or esimates the refresh rate of the monitor that is
 * displaying our graphics. */
int viewmat_get_refresh_rate(void)
{
	GLFWwindow *window = kuhl_get_window();

	/* If we are full screen, we can get the monitor we are on and get
	 * the refresh rate. */
	GLFWmonitor *monitor = glfwGetWindowMonitor(window);
	if(monitor != NULL) // monitor will be null if we are using a windowed mode window.
	{
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		if(mode != NULL)
			return mode->refreshRate;
	}

	/* We get here if we aren't full screen. */

	/* If there is only one monitor, we can figure out the refresh rate */
	int numMonitors = 0;
	GLFWmonitor** monitorList = glfwGetMonitors(&numMonitors);
	if(numMonitors == 1)
	{
		const GLFWvidmode *mode = glfwGetVideoMode(monitorList[0]);
		if(mode != NULL)
			return mode->refreshRate;
	}

	/* If there are multiple monitors using the same refresh rate, we
	 * can figure out the refresh rate. */
	const GLFWvidmode *mode = glfwGetVideoMode(monitorList[0]); // not checking for error.
	int firstRefresh = mode->refreshRate;
	int allSame = 1;
	for(int i=0; i<numMonitors; i++)
	{
		mode = glfwGetVideoMode(monitorList[i]); // not checking for error.
		// Allow for 59.9 to be the same as 60.
		if(abs(firstRefresh - mode->refreshRate) > 1)
			allSame = 0;
	}
	if(allSame)
		return firstRefresh;


	/* If we can't figure the refresh rate out, assume it is the same
	 * as the primary monitor. */
	monitor = glfwGetPrimaryMonitor();
	if(monitor == NULL)
		return 60;
	mode = glfwGetVideoMode(monitor);
	if(mode == NULL)
		return 60;
	return mode->refreshRate;
}


/** Call once per frame to update the 'fps' variable. */
static float fps = 0;
static void viewmat_stats_fps(void)
{
#define FPS_SAMPLES 40
	static long list[FPS_SAMPLES];
	static int index = 0;         // where next value should be stored
	static int listfull = 0;      // have we filled up the array?

	long now = kuhl_microseconds();

	if(listfull)
	{
		long oldest = list[index]; // the item we are going to overwrite is the oldest one in the list.
		long change = now-oldest;  // time to render FPS_SAMPLES frames in microseconds
		float usecPerFrame = (float) change/FPS_SAMPLES;
		float secPerFrame = usecPerFrame / 1000000.0f;
		fps = 1.0f/secPerFrame;
	}

	list[index] = now;
	
	index = (index+1) % FPS_SAMPLES; // increment index, wrap around
	if(index == 0)
		listfull = 1;
	
#undef FPS_SAMPLES
}

/** Retrieve the current FPS. This will work if you call
    viewmat_end_frame() to swap the buffers at the end of every frame. */
float viewmat_fps(void)
{
	return fps;
}


/** Swaps buffers and implements latency reduction. */
void viewmat_swap_buffers(void)
{
	if(viewmat_swapinterval == 0 || kuhl_config_boolean("viewmat.latencyreduce", 1,1) == 0) // if FPS is unrestricted.
	{
		dgr_update(1,0); // make sure master can send before blocking at swap
		glfwSwapBuffers(kuhl_get_window());
		viewmat_stats_fps();
		dgr_update(0,1); // make sure slave receives after blocking at swap
		return;
	}
	
	static int count = 0;
	if(count < 100)
		count++;
	static float avgRenderingLastFrame = -1;
	static float avgRenderingLastFrameDev = 0;
	static float avgWaitingForVsync = -1;
	static long postswap_prev = -1;
	static long postsleep_prev = -1;


	static int vsyncTime = -1; //**< microseconds/frame
	if(vsyncTime == -1)
	{
		int refreshRate = viewmat_get_refresh_rate();
		// 1 / (frames/second) * 1000000 microseconds/second = microseconds/frame
		vsyncTime = 1.0/refreshRate * 1000000;
		msg(MSG_INFO, "Latency reduction is turned on; assuming monitor is %dHz and we have %d microseconds/frame\n", refreshRate, vsyncTime);
		msg(MSG_INFO, "Set viewmat.latencyreduce to 0 to disable latency reduction.\n", refreshRate, vsyncTime);
	}

	
	dgr_update(1,0); // make sure master can send before blocking at swap
	GLFWwindow *window = kuhl_get_window();
	/* We can call glFinish() to ensure that all rendering is done so
	 * that our preswap time has a higher chance of being
	 * accurate. Otherwise, our the time spent in swapbuffers can be
	 * both the time waiting for vsync and the time waiting for the
	 * previous OpenGL calls to finish. For more information, see:
	 * https://www.opengl.org/wiki/Performance
	 *
	 * Skipping glFinish() may result in a slight performance increase
	 * and may have a detrimental impact on our latency reduction
	 * code.
	 */
	//glFinish();
	long preswap = kuhl_microseconds();
	glfwSwapBuffers(window);
	viewmat_stats_fps();
	long postswap = kuhl_microseconds();



	// Note: The dgr_update(0,1) call happens after the sleep at the
	// end of this function.
	int timeWaitingForVsync = postswap - preswap;

	if(count < 10) // initialize averages, skip first few frames.
	{
		if(count > 2)
			avgRenderingLastFrame = preswap - postsleep_prev;
		
		avgWaitingForVsync = timeWaitingForVsync;
		postswap_prev = postswap;
		postsleep_prev = postswap; // we aren't sleeping.
		return;
	}

	/* Figure out if we missed a frame */
	float missedFrames = 0.0f;
	long elapsed = postswap - postswap_prev;
	
	if(elapsed > (((long)vsyncTime)*3)/2)
	{
		static int maxMessages = 20;
		missedFrames = elapsed / (float) vsyncTime - 1.0f;
		if(count > 60) // don't print message for first 60 frames.
		{
			if(maxMessages >= 0)
				maxMessages--;

			if(maxMessages > 0)
				msg(MSG_INFO, "Missed approximately %0.2f frame(s), %ld usec have elapsed", missedFrames, elapsed);
			else
			{
				if(maxMessages == 0)
					msg(MSG_INFO, "No more messages about dropped frames, but messages will still be saved to log file.");
				msg(MSG_DEBUG, "Missed approximately %0.2f frame(s), %ld usec have elapsed", missedFrames, elapsed);
			}
#if 0				
			msg(MSG_WARNING, "Displayed previous frame at  %ld", postswap_prev);
			msg(MSG_WARNING, "Displayed this frame at      %ld", postswap);
			msg(MSG_WARNING, "Expected this frame before   %ld", postswap_prev+(((long)vsyncTime)*3)/2);
#endif
		}
	}

	const float alpha = .95; // weight to put on running average

	// Update average waiting for vsync time. Note: The wait time can
	// get large if the frame rate is low. I.e., if we miss a vsync,
	// we have to wait until the next vsync. We calculate the time
	// waiting for vsync for informational purposes only.
	avgWaitingForVsync    = alpha * avgWaitingForVsync    + (1-alpha) * timeWaitingForVsync;

	/* Update our estimates of the time it takes to render a
	 * frame---both the average and our estimate of the deviation.  */
	int timeRenderingLastFrame = preswap - postsleep_prev;
	avgRenderingLastFrame    = alpha * avgRenderingLastFrame + (1-alpha) * timeRenderingLastFrame;
	avgRenderingLastFrameDev = alpha * avgRenderingLastFrameDev + (1-alpha)*(fabsf(avgRenderingLastFrame-timeRenderingLastFrame));
	
	if(count < 60) // collected enough data so our averages are reasonable.
	{
		postswap_prev = postswap;
		postsleep_prev = postswap; // we aren't sleeping.
		return;
	}

	/* Add in more time based on our deviation estimate */
	float renderingTimeMax = avgRenderingLastFrame + avgRenderingLastFrameDev * 2;

	/** The ideal time that we want to save in microseconds. If we set
	 * this to 1000, and we have 16666 microseconds per frame, then we
	 * will try to sleep such that the rendering will finish at
	 * 16666-1000=15666 microseconds. If we set this value to 0, then
	 * we won't save any extra time and we risk missing a frame. */
	const int buffer_time = 1000;

	/* We have vsyncTime until the next vsync. Subtract out expected
	 * rendering time and the buffer time. Also subtract out
	 * additional time if we missed one or more frames. */
	int sleepTime = vsyncTime - renderingTimeMax - buffer_time - missedFrames*1000;

#if 0
	msg(MSG_INFO, "Sleeping for %6d = %6d(avail) - %6.0f(rendermax) - %d(buf) - %6.0f(missedframe)\n", sleepTime, vsyncTime, renderingTimeMax, buffer_time, missedFrames*1000);
	msg(MSG_INFO, "LastRender=%d AvgRender=%.0f AvgRenderDev=%.0f VsyncWait=%d AvgVsyncWait=%.0f",
	    timeRenderingLastFrame, avgRenderingLastFrame, avgRenderingLastFrameDev, timeWaitingForVsync, avgWaitingForVsync);
#endif
	
	postsleep_prev = postswap;
	postswap_prev = postswap;

	if(sleepTime > 0)
	{
		usleep(sleepTime);
		postsleep_prev = kuhl_microseconds();
	}

	dgr_update(0,1); // make sure slave receives after blocking at swap
	return;
}

/** Should be called when we have completed rendering a frame. For
 * HMDs, this should be called after both the left and right eyes have
 * been rendered. */
void viewmat_end_frame(void)
{
	if(viewmat_display_mode == VIEWMAT_OCULUS)
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
	else if(viewmat_display_mode == VIEWMAT_ANAGLYPH)
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	/* Need to swap front and back buffers here unless we are using
	 * Oculus. (Oculus draws to the screen directly). */
	if(viewmat_display_mode != VIEWMAT_OCULUS)
	{
		viewmat_swap_buffers();
	}
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
	viewmat_validate_viewportId(viewportID);
	
#ifndef MISSING_OVR
	if(viewmat_display_mode == VIEWMAT_OCULUS)
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
 * 0.
 */
void viewmat_begin_eye(int viewportID)
{
	viewmat_validate_viewportId(viewportID);
	
#ifndef MISSING_OVR
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

	if(viewmat_display_mode == VIEWMAT_ANAGLYPH)
	{
		if(viewportID == 0)
			glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
		else if(viewportID == 1)
			glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_FALSE);
		else
		{
			msg(MSG_FATAL, "Unknown viewport ID: %d\n",viewportID);
			exit(EXIT_FAILURE);
		}
	}
}

/** Sets up viewmat to only have one viewport. This can be called
 * every frame since resizing the window will change the size of the
 * viewport! */
static void viewmat_one_viewport(void)
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

static void viewmat_anaglyph_viewports(void)
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
static void viewmat_two_viewports(void)
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
static void viewmat_oculus_viewports(void)
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
static void viewmat_refresh_viewports(void)
{
	switch(viewmat_display_mode)
	{
		case VIEWMAT_DESKTOP:
		case VIEWMAT_IVS:
			viewmat_one_viewport();
			break;
		case VIEWMAT_HMD:
			viewmat_two_viewports();
			break;
		case VIEWMAT_OCULUS:
			viewmat_oculus_viewports();
			break;
		case VIEWMAT_ANAGLYPH:
			viewmat_anaglyph_viewports();
			break;
		default:
			msg(MSG_ERROR, "Unknown viewmat mode: %d\n", viewmat_display_mode);
			exit(EXIT_FAILURE);
	}
}

/** Checks if 'viewmat.vrpn.object' config variable is set. If it
    is, use VRPN to control the camera position and orientation.
    
    @return Returns 1 if viewmat.vrpn.object is set up, 0 otherwise.
*/
static int viewmat_init_vrpn(void)
{
	viewmat_vrpn_obj = NULL;
	
	const char* vrpnObjString = kuhl_config_get("viewmat.vrpn.object");
	if(vrpnObjString != NULL && strlen(vrpnObjString) > 0)
	{
		viewmat_vrpn_obj = vrpnObjString;
		msg(MSG_INFO, "View is following tracker object: %s\n", viewmat_vrpn_obj);
		
		/* Try to connect to VRPN server */
		float vrpnPos[3];
		float vrpnOrient[16];
		vrpn_get(viewmat_vrpn_obj, NULL, vrpnPos, vrpnOrient);
		return 1;
	}
	else
	{
		msg(MSG_FATAL, "viewmat is unable to control the camera with VRPN because you didn't specify the name of the tracked object that the camera should follow. Please fill in the name of the object in a configuration file with the key 'viewmat.vrpn.object'.");
		exit(EXIT_FAILURE);
	}
	return 0;
}


/** Checks if ORIENT_SENSE_TTY and ORIENT_SENSE_TYPE are set. if so,
    set up an orientation sensor.
    
    @return Returns 1 if orientation sensor is set up, 0 otherwise.
*/
static int viewmat_init_orient_sensor(void)
{
	if(getenv("ORIENT_SENSOR_TTY") != NULL &&
	   getenv("ORIENT_SENSOR_TYPE") != NULL)
	{
		msg(MSG_INFO, "Found an orientation sensor specified in an environment variable...connecting.");
		viewmat_orientsense = orient_sensor_init(NULL, ORIENT_SENSOR_NONE);
		return 1;
	}
	msg(MSG_INFO, "No orientation sensor found");
	
	return 0;
}



/** Initialize mouse movement. Or, if we have a VRPN object name we
 * are supposed to be tracking, get ready to use that instead. */
static void viewmat_init_mouse(const float pos[3], const float look[3], const float up[3])
{
	GLFWwindow *window = kuhl_get_window();
	glfwSetMouseButtonCallback(window, mousemove_glfwMouseButtonCallback);
	glfwSetCursorPosCallback(window, mousemove_glfwCursorPosCallback);
	glfwSetScrollCallback(window, mousemove_glfwScrollCallback);

	//glutMotionFunc(mousemove_glutMotionFunc);
	//glutMouseFunc(mousemove_glutMouseFunc);
	mousemove_set(pos[0],pos[1],pos[2],
	              look[0],look[1],look[2],
	              up[0],up[1],up[2]);
	mousemove_speed(0.05, 0.5);
}



/** Initialize the Oculus HMD.
 *
 * @param pos The position that we want the Oculus HMD to start at.
 */
static void viewmat_init_hmd_oculus(const float pos[3])
{
#ifdef MISSING_OVR
	msg(MSG_FATAL, "Oculus support is missing: You have not compiled this code against the LibOVR library.\n");
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
	viewmat_init_vrpn();

	
	// TODO: We are supposed to do these things when we are done:
	//ovrHmd_Destroy(hmd);
	//ovr_Shutdown();
#endif
}

/** Get the swap interval settings and call glfwSwapInterval().
 */
static void viewmat_init_swapinterval(void)
{
	viewmat_swapinterval = kuhl_config_int("viewmat.swapinterval", -1, -1);

	/* If swap_control_tear extension doesn't exist, don't use it. */
	if(!glfwExtensionSupported("GLX_EXT_swap_control_tear") &&
	   !glfwExtensionSupported("WGL_EXT_swap_control_tear"))
	{
		// https://www.opengl.org/registry/specs/EXT/glx_swap_control_tear.txt
		// https://www.opengl.org/registry/specs/EXT/wgl_swap_control_tear.txt

		msg(MSG_DEBUG, "Machine lacks support for swap_control_tear extension");
		if(viewmat_swapinterval == -1)
			viewmat_swapinterval = 1;
	}

	if(viewmat_swapinterval < -1 || viewmat_swapinterval > 1)
		msg(MSG_WARNING, "viewmat.swapinterval should be set to -1, 0 or 1. You have set it to %d\n", viewmat_swapinterval);

	/* If configuration requested 0 */
	if(viewmat_swapinterval == 0)
	{
		msg(MSG_WARNING, "Buffer swapping can happen at any time; FPS can go above monitor refresh rate; tearing may occur.");
		msg(MSG_WARNING, "Set viewmat.swapinterval to -1 to swap buffers during monitor refresh.");
	}

	/* Swap interval settings:

	   0  - Swap buffers whenever possible. Tearing can occur. FPS can
	        go above monitor refresh rate.

	   1  - Swap buffers only during monitor refresh. Tearing never occurs.

	   -1 - Swap buffers during monitor refresh if FPS is high
	        enough. If FPS drops below monitor refresh, tearing can
	        occur.
	*/
	glfwSwapInterval(viewmat_swapinterval);
}


/** Initialize viewmat and use the specified pos/look/up values as a
 * starting location when mouse movement is used.
 *
 * @param pos The position of the camera (if mouse movement is used)
 * @param look A point the camera is looking at (if mouse movement is used)
 * @param up An up vector for the camera (if mouse movement is used).
 */
void viewmat_init(const float pos[3], const float look[3], const float up[3])
{

	viewmat_init_swapinterval();
	
	const char* controlModeString = kuhl_config_get("viewmat.controlmode");

	/* Make an intelligent guess if unspecified */
	if(controlModeString == NULL) 
	{
		if(getenv("ORIENT_SENSOR_TTY") != NULL &&
		   getenv("ORIENT_SENSOR_TYPE") != NULL)
		{
			msg(MSG_INFO, "viewmat control Mode: Unspecified, but using orientation sensor.");
			controlModeString = "orient";
		}
		else if(getenv("VIEWMAT_VRPN_OBJECT") != NULL)
		{
			msg(MSG_INFO, "viewmat control Mode: Unspecified, but using VRPN.");
			controlModeString = "vrpn";
		}
		else
			controlModeString = "mouse";
	}

	/* Initialize control mode */
	if(strcasecmp(controlModeString, "mouse") == 0)
	{
		msg(MSG_INFO, "viewmat control mode: Mouse movement");
		viewmat_control_mode = VIEWMAT_CONTROL_MOUSE;
		viewmat_init_mouse(pos, look, up);
	}
	else if(strcasecmp(controlModeString, "none") == 0)
	{
		msg(MSG_INFO, "viewmat control mode: None (fixed view)");
		viewmat_control_mode = VIEWMAT_CONTROL_NONE;
		// Set our initial position, but don't handle mouse movement.
		mousemove_set(pos[0],pos[1],pos[2],
		              look[0],look[1],look[2],
		              up[0],up[1],up[2]);
	}
	else if(strcasecmp(controlModeString, "orient") == 0)
	{
		msg(MSG_INFO, "viewmat control mode: Orientation sensor");
		viewmat_control_mode = VIEWMAT_CONTROL_ORIENT;
		viewmat_init_orient_sensor();
	}
	else if(strcasecmp(controlModeString, "vrpn") == 0)
	{
		msg(MSG_INFO, "viewmat control mode: VRPN");
		viewmat_control_mode = VIEWMAT_CONTROL_VRPN;
		viewmat_init_vrpn();
	}
	else if(strcasecmp(controlModeString, "oculus") == 0)
	{
		msg(MSG_INFO, "viewmat control mode: Oculus");
		viewmat_control_mode = VIEWMAT_CONTROL_OCULUS;
	}
	else
	{
		msg(MSG_FATAL, "viewmat control mode: unhandled mode '%s'.", controlModeString);
		exit(EXIT_FAILURE);
	}


	const char* modeString = kuhl_config_get("viewmat.displaymode");
	if(modeString == NULL || strlen(modeString) == 0)
		modeString = "none";
	
	if(strcasecmp(modeString, "ivs") == 0)
	{
		viewmat_display_mode = VIEWMAT_IVS;
		msg(MSG_INFO, "viewmat display mode: IVS");
	}
	else if(strcasecmp(modeString, "oculus") == 0)
	{
		viewmat_display_mode = VIEWMAT_OCULUS;
		msg(MSG_INFO, "viewmat display mode: Using Oculus HMD.\n");
		viewmat_init_hmd_oculus(pos);
	}
	else if(strcasecmp(modeString, "hmd") == 0)
	{
		viewmat_display_mode = VIEWMAT_HMD;
		msg(MSG_INFO, "viewmat display mode: Side-by-side left/right view.\n");
	}
	else if(strcasecmp(modeString, "none") == 0)
	{
		viewmat_display_mode = VIEWMAT_DESKTOP;
		msg(MSG_INFO, "viewmat display mode: Single window desktop mode.\n");
	}
	else if(strcasecmp(modeString, "anaglyph") == 0)
	{
		viewmat_display_mode = VIEWMAT_ANAGLYPH;
		msg(MSG_INFO, "viewmat display mode: Anaglyph image rendering. Use the red filter on the left eye and the cyan filter on the right eye.\n");
		viewmat_init_mouse(pos, look, up);
	}
	else
	{
		msg(MSG_FATAL, "viewmat display mode: unhandled mode '%s'.", modeString);
		exit(EXIT_FAILURE);
	}

	/* We can't use the Oculus orientation sensor if we haven't
	 * initialized the Oculus code. We do that initialization in the
	 * Oculus display mode code. */
	if(viewmat_control_mode == VIEWMAT_CONTROL_OCULUS &&
	   viewmat_display_mode != VIEWMAT_OCULUS)
	{
		msg(MSG_FATAL, "viewmat: Oculus can only be used as a control mode if it is also used as a display mode.");
		exit(EXIT_FAILURE);
	}
	
	
	viewmat_refresh_viewports();

	// If there are two "viewports" then it is likely that we are
	// doing stereoscopic rendering. Displaying the mouse cursor can
	// interfere with stereo images, so we disable the cursor here.
	if(viewports_size == 2)
		//glutSetCursor(GLUT_CURSOR_NONE);
		// TODO: Switch to GLFW_CURSOR_DISABLED?
		glfwSetInputMode(kuhl_get_window(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

/** Some VRPN orientation sensors may be rotated differently than what we
 * expect them to be (for example, orientation is correct except that
 * the camera is pointing in the wrong direction). This function will
 * adjust the orientation matrix so that the camera is pointing in the
 * correct direction. */
static void viewmat_fix_rotation(float orient[16])
{
	// Fix rotation for a hard-wired orientation sensor.
	if(viewmat_control_mode == VIEWMAT_CONTROL_ORIENT)
	{
		float adjustLeft[16] = { 0, 1, 0, 0,
		                         0, 0, 1, 0,
		                         1, 0, 0, 0,
		                         0, 0, 0, 1 };
		mat4f_transpose(adjustLeft); // transpose to column-major order

		float adjustRight[16] = { 0, 0, -1, 0,
		                         -1, 0,  0, 0,
		                          0, 1,  0, 0,
		                          0, 0,  0, 1 };
		mat4f_transpose(adjustRight);

		mat4f_mult_mat4f_new(orient, adjustLeft, orient);
		mat4f_mult_mat4f_new(orient, orient, adjustRight);
		return;
	}

	// Fix rotation for VRPN
	if(viewmat_vrpn_obj == NULL || strlen(viewmat_vrpn_obj) == 0)
		return;

	const char *hostname = vrpn_default_host();
	if(hostname == NULL)
		return;
	
	/* Some objects in the IVS lab need to be rotated to match the
	 * orientation that we expect. Apply the fix here. */
	if(vrpn_is_vicon(hostname)) // MTU vicon tracker
	{
		/* Note, orient has not been transposed/inverted yet. Doing
		 * orient*offset will effectively effectively be rotating the
		 * camera---not the world. */ 
		if(strcmp(viewmat_vrpn_obj, "DK2") == 0)
		{
			float offsetVicon[16];
			mat4f_identity(offsetVicon);
			mat4f_rotateAxis_new(offsetVicon, 90, 1,0,0);
			mat4f_mult_mat4f_new(orient, orient, offsetVicon);
		}

		if(strcmp(viewmat_vrpn_obj, "DSight") == 0)
		{
			float offsetVicon1[16];
			mat4f_identity(offsetVicon1);
			mat4f_rotateAxis_new(offsetVicon1, 90, 1,0,0);
			float offsetVicon2[16];
			mat4f_identity(offsetVicon2);
			mat4f_rotateAxis_new(offsetVicon2, 180, 0,1,0);
			
			// orient = orient * offsetVicon1 * offsetVicon2
			mat4f_mult_mat4f_many(orient, orient, offsetVicon1, offsetVicon2, NULL);
		}
	}
}


/** Viewport 0 is the first viewport to render. In HMDs, the first
 * viewport to render is often the left eye. However, this does not
 * always need to be the case. */
viewmat_eye viewmat_viewport_to_eye(int viewportNum)
{
	if(viewmat_display_mode == VIEWMAT_OCULUS)
	{
#ifndef MISSING_OVR
		if(hmd->EyeRenderOrder[viewportNum] == ovrEye_Left)
			return VIEWMAT_EYE_LEFT;
		else if(hmd->EyeRenderOrder[viewportNum] == ovrEye_Right)
			return VIEWMAT_EYE_RIGHT;
		else
			return VIEWMAT_EYE_UNKNOWN;
#else
		return VIEWMAT_EYE_UNKNOWN;
#endif
	}

	if(viewports_size == 1 && viewportNum == 0)
		return VIEWMAT_EYE_MIDDLE;
	if(viewports_size == 2)
	{
		if(viewportNum == 0)
			return VIEWMAT_EYE_LEFT;
		else if(viewportNum == 1)
			return VIEWMAT_EYE_RIGHT;
		else
			return VIEWMAT_EYE_UNKNOWN;
	}
	return VIEWMAT_EYE_UNKNOWN;
}

/** Given a view matrix for a single eye, return a new view matrix. If
 * there are two viewports (one for each eye) different matrices will
 * be returned depending on the viewport.
 *
 * @param viewmatrix The new view matrix (filled in by this function)
 *
 * @param cyclopsViewMatrix A single view matrix. If two eyes, this
 * view matrix represents the point between the eyes.
 *
 * @param viewportNum The viewport number.
 */
static void viewmat_get_generic(float viewmatrix[16], const float cyclopsViewMatrix[16], const int viewportNum)
{
	/* Update the view matrix based on which eye we are rendering */
	float eyeDist = 0.055;  // TODO: Make this configurable.
	viewmat_eye eye = viewmat_viewport_to_eye(viewportNum);
	float eyeShift = 0;
	if(eye == VIEWMAT_EYE_LEFT)
		eyeShift = -eyeDist/2.0;
	else if(eye == VIEWMAT_EYE_RIGHT)
		eyeShift = eyeDist/2.0;
	float shiftMatrix[16];
	// Negate eyeShift because the matrix would shift the world, not
	// the eye by default.
	mat4f_translate_new(shiftMatrix, -eyeShift, 0, 0);

	/* Adjust the view matrix by the eye offset */
	mat4f_mult_mat4f_new(viewmatrix, shiftMatrix, cyclopsViewMatrix);
}


/** Get the view matrix from the mouse.
 *
 * @param viewmatrix The view matrix to be filled in.
 *
 * @param viewportNum If mouse mode is used with a left an right
 * screen for an HMD, we need to return different view matrices for
 * the left (0) and right (1) eyes.
 *
 * @return The eye that viewportNum corresponds to.
 **/
static void viewmat_get_mouse(float viewmatrix[16], int viewportNum)
{
	float pos[3],look[3],up[3];
	mousemove_get(pos, look, up);
	float cyclopsViewMatrix[16];
	mat4f_lookatVec_new(cyclopsViewMatrix, pos, look, up);
	viewmat_get_generic(viewmatrix, cyclopsViewMatrix, viewportNum);
}


static void viewmat_get_vrpn(float viewmatrix[16], int viewportNum)
{
	if(viewmat_vrpn_obj == NULL)
		return;
			
	float pos[3] = { 0,0,0 };
	float rotMat[16], posMat[16];
	vrpn_get(viewmat_vrpn_obj, NULL, pos, rotMat);
	mat4f_translate_new(posMat, -pos[0], -pos[1], -pos[2]); // position
	viewmat_fix_rotation(rotMat);
	mat4f_transpose(rotMat); /* orientation sensor rotates camera, not world */

	float cyclopsViewMatrix[16];
	mat4f_mult_mat4f_new(cyclopsViewMatrix, rotMat, posMat);

	viewmat_get_generic(viewmatrix, cyclopsViewMatrix, viewportNum);
}


/** Get the view matrix from an orientation sensor. */
static void viewmat_get_orient_sensor(float viewmatrix[16], int viewportNum)
{
	float quaternion[4];
	orient_sensor_get(&viewmat_orientsense, quaternion);
	
	float cyclopsViewMatrix[16];
	mat4f_rotateQuatVec_new(cyclopsViewMatrix, quaternion);
	viewmat_fix_rotation(cyclopsViewMatrix);
	
	/* set a default camera position */
	float pos[4] = { 0, 1.5, 0, 1 };
	mat4f_setColumn(cyclopsViewMatrix, pos, 3);
	mat4f_invert(cyclopsViewMatrix);

	viewmat_get_generic(viewmatrix, cyclopsViewMatrix, viewportNum);
}


/** Get view and projection matrices appropriate for the Oculus HMD */
static void viewmat_get_hmd_oculus(float viewmatrix[16], float projmatrix[16], int viewportID)
{
#ifndef MISSING_OVR
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
#else
	/* We shouldn't ever get here, but we'll generate a generic view
	 * and projection matrix just in case... */
	mat4f_lookat_new(viewmatrix,
	                 0,1.55,0,
	                 0,1.55,-1,
	                 0,1,0);
	mat4f_perspective_new(projmatrix, 50, 1, 0.5, 500);
#endif
}

/** Get a view matrix from VRPN and adjust the view frustum
 * appropriately.
 * @param viewmatrix The location where the viewmatrix should be stored.
 * @param frustum The location of the view frustum that should be adjusted.
 */
static void viewmat_get_ivs(float viewmatrix[16], float frustum[6])
{
	/* Only get information from VRPN if we are DGR master, or if DGR
	 * is being used at all. */
	float pos[3];
	if((dgr_is_enabled() && dgr_is_master()) || dgr_is_enabled()==0)
	{
		if(viewmat_control_mode == VIEWMAT_CONTROL_VRPN &&
		   viewmat_vrpn_obj != NULL)
		{
			/* get information from vrpn */
			float orient[16];
			vrpn_get(viewmat_vrpn_obj, NULL, pos, orient);
		}
		else
		{
			/* If no head tracking is available, assume the person is
			 * standing at the origin with a normal eye height */
			pos[0] = 0;
			pos[1] = 1.5; // normal eye height
			pos[2] = 0;
		}
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
		float flip = 1;
		/* In most cases, viewportID=0 is the left eye. However,
		 * Oculus may cause this to get swapped. */
		if(viewmat_viewport_to_eye(0) == VIEWMAT_EYE_RIGHT)
			flip = -1;

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
			msg(MSG_WARNING, "IPD=%.4f meters, delay=%ld us (IPD validation failed; occasional messages are OK!)\n", ipd, delay);
		}
		// msg(MSG_INFO, "IPD=%.4f meters, delay=%ld us\n", ipd, delay);

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
 * @param viewportID If there is only one viewport, set this to
 * 0. This value must be smaller than the value reported by
 * viewmat_num_viewports(). In an HMD, typically viewportID=0 is the
 * left eye and viewportID=1 is the right eye. However, some Oculus
 * HMDs will result in this being swapped. To definitively know which
 * eye this view matrix corresponds to, examine the return value of
 * this function.
 *
 * @return A viewmat_eye enum which indicates if this view matrix is
 * for the left, right, middle, or unknown eye.
 *
 */
viewmat_eye viewmat_get(float viewmatrix[16], float projmatrix[16], int viewportID)
{
	viewmat_eye eye = viewmat_viewport_to_eye(viewportID);
	
	int viewport[4]; // x,y of lower left corner, width, height
	viewmat_get_viewport(viewport, viewportID);

	/* Get the view or camera matrix; update the frustum values if needed. */
	float f[6]; // left, right, bottom, top, near>0, far>0
	projmat_get_frustum(f, viewport[2], viewport[3], viewportID);

	/* If we are running in IVS mode and using the tracking systems,
	 * all computers need to update their frustum differently. The
	 * master process will be controlled by VRPN, and all slaves will
	 * have their controllers set to "none". Here, we detect for this
	 * situation and make sure all processes work correctly.
	 */
	if(viewmat_display_mode == VIEWMAT_IVS &&
	   viewmat_control_mode == VIEWMAT_CONTROL_VRPN)
	{
		// Will update view matrix and frustum information
		viewmat_get_ivs(viewmatrix, f);
		mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
	}
	else
	{
		switch(viewmat_control_mode)
		{
			case VIEWMAT_CONTROL_MOUSE:    // mouse movement
				viewmat_get_mouse(viewmatrix, viewportID);
				mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
				break;
			case VIEWMAT_CONTROL_NONE:
				// Get the view matrix from the mouse movement code...but
				// we haven't registered mouse movement callback
				// functions, so mouse movement won't work.
				viewmat_get_mouse(viewmatrix, viewportID);
				mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
				break;
			case VIEWMAT_CONTROL_ORIENT:
				viewmat_get_orient_sensor(viewmatrix, viewportID);
				mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
				break;
			case VIEWMAT_CONTROL_OCULUS:
				viewmat_get_hmd_oculus(viewmatrix, projmatrix, viewportID);
				// previous function sets projmatrix for us...
				break;
			case VIEWMAT_CONTROL_VRPN:
				viewmat_get_vrpn(viewmatrix, viewportID);
				mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
				break;

			default:
				msg(MSG_FATAL, "Unknown viewmat control mode: %d\n", viewmat_control_mode);
				exit(EXIT_FAILURE);
		}
	}

	/* Send the view matrix to DGR. At some point in the future, we
	 * may have multiple computers using different view matrices. For
	 * now, even in IVS mode, all processes will use the same view
	 * matrix (IVS uses different view frustums per process). */
	char dgrkey[128];
	snprintf(dgrkey, 128, "!!viewmat%d", viewportID);
	dgr_setget(dgrkey, viewmatrix, sizeof(float)*16);

	/* Sanity checks */
	viewmat_validate_ipd(viewmatrix, viewportID);
	return eye;
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
	viewmat_validate_viewportId(viewportNum);

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


