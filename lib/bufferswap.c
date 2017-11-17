#ifdef _WIN32
#include "windows-compat.h" // usleep() on windows
#else
#define _GNU_SOURCE
#include <unistd.h> // usleep() on Linux and Mac
#endif

#include <stdlib.h>
#include <GLFW/glfw3.h>
#include "kuhl-util.h"
#include "dgr.h"

static int viewmat_swapinterval = 0;
static float fps = 0;

/** Guesses or esimates the refresh rate of the monitor that is
    displaying our graphics.

    @return Number of monitor refreshes per second.
*/
int bufferswap_get_refresh_rate(void)
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
static void bufferswap_stats_fps(void)
{
#define FPS_SAMPLES 40
	static long list[FPS_SAMPLES];
	static int index = 0;         // where next value should be stored
	static int listfull = 0;      // have we filled up the array?

	long now = kuhl_microseconds();

	list[index] = now;

	/* List becomes full when we write last record in list */
	if(listfull == 0 && index == FPS_SAMPLES-1)
		listfull = 1;

	long newestIdx = index;
	long oldestIdx = (index+1)%FPS_SAMPLES;

	// change = time to render FPS_SAMPLES-1 frames. If the list
	// contains 40 timestamps, then the difference between the
	// newest and oldest timestamp is time between 39 frames.
	long change = list[newestIdx]-list[oldestIdx];
	
	/* Calculate FPS */
	if(listfull == 1)
	{
		float usecPerFrame = (float) change/(FPS_SAMPLES-1);
		float secPerFrame = usecPerFrame / 1000000.0f;
		fps = 1.0f/secPerFrame;

		int refreshRate = bufferswap_get_refresh_rate();
		if(refreshRate == 59)
			refreshRate = 60;
		long expectedTimePerFrame = (1.0/refreshRate*1000000);
		long timeLastFrame = list[newestIdx] - list[(newestIdx-1+FPS_SAMPLES)%FPS_SAMPLES];

		if(timeLastFrame > expectedTimePerFrame*1.5)
		{
			msg(MSG_DEBUG, "Skipped a frame. %ld usec between framebuffer swaps (budget %ld usec).", timeLastFrame, expectedTimePerFrame);
		}

#if 0
		// Show the time spent to render all of the frames that we
		// have records for. Note that index points to the oldest item
		// in the array. Since the array stores the time that each
		// frame happened, we subtract to get the time spent rendering
		// each frame.
		for(int i=oldestIdx+1; i<oldestIdx+FPS_SAMPLES; i++)
			printf("%ld ", list[i%FPS_SAMPLES]-list[(i-1)%FPS_SAMPLES]);
		printf("\n");
#endif
	}

	

	index = (index+1) % FPS_SAMPLES; // increment index, wrap around
	// msg(MSG_INFO, "FPS: %f", fps);
	
#undef FPS_SAMPLES
}

/** Retrieve the current FPS. This will work as long as you use
 * bufferswap() to swap your buffers. */
float bufferswap_fps(void)
{
	return fps;
}




static void bufferswap_simple(void)
{
	glfwSwapBuffers(kuhl_get_window());
	bufferswap_stats_fps();
	return;
}



static void bufferswap_latencyreduce()
{
	
	static int count = 0;
	if(count < 100)
		count++;
	static int avgRenderingLastFrame = -1;
	static int avgRenderingLastFrameDev = 0;
	static int avgWaitingForVsync = -1;
	static long postswap_prev = -1;
	static long postsleep_prev = -1;


	static int vsyncTime = -1; //**< microseconds/frame
	if(vsyncTime == -1)
	{
		int refreshRate = bufferswap_get_refresh_rate();
		if(refreshRate == 59)
			refreshRate = 60;
		// 1 / (frames/second) * 1000000 microseconds/second = microseconds/frame
		vsyncTime = (int) (1.0/refreshRate * 1000000);
		msg(MSG_INFO, "Latency reduction is turned on; assuming monitor is %dHz and we have %d microseconds/frame\n", refreshRate, vsyncTime);
		msg(MSG_INFO, "Set bufferswap.latencyreduce to 0 to disable latency reduction.\n", refreshRate, vsyncTime);
	}

	
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
	bufferswap_stats_fps();
	long postswap = kuhl_microseconds();



	int timeWaitingForVsync = postswap - preswap;

	if(count < 10) // initialize averages, skip first few frames.
	{
		if(count > 2)
			avgRenderingLastFrame = (float) (preswap - postsleep_prev);
		
		avgWaitingForVsync = (float) timeWaitingForVsync;
		postswap_prev = postswap;
		postsleep_prev = postswap; // we aren't sleeping.
		return;
	}

	/* Figure out if we missed a frame */
	int elapsed = postswap - postswap_prev;
	/* How much time does it appear that we miss the vsync by. */
	int missedBy = elapsed - vsyncTime;
	if(missedBy < 0)
		missedBy = 0;
	
	const float alpha = .98f; // weight to put on running average

	// Update average waiting for vsync time. Note: The wait time can
	// get large if the frame rate is low. I.e., if we miss a vsync,
	// we have to wait until the next vsync. We calculate the time
	// waiting for vsync for informational purposes only.
	avgWaitingForVsync    = alpha * avgWaitingForVsync    + (1-alpha) * timeWaitingForVsync;

	/* Update our estimates of the time it takes to render a
	 * frame---both the average and our estimate of the deviation.  */
	int timeRenderingLastFrame = preswap - postsleep_prev;
	avgRenderingLastFrame    = alpha * avgRenderingLastFrame    + (1-alpha) * timeRenderingLastFrame;
	avgRenderingLastFrameDev = alpha * avgRenderingLastFrameDev + (1-alpha) * abs(avgRenderingLastFrame-timeRenderingLastFrame);
	
	if(count < 60) // collected enough data so our averages are reasonable.
	{
		postswap_prev = postswap;
		postsleep_prev = postswap; // we aren't sleeping.
		return;
	}

	/* Add in more time based on our deviation estimate */
	int renderingTimeMax = avgRenderingLastFrame + avgRenderingLastFrameDev * 2;

	/* We have vsyncTime until the next vsync. Subtract out expected
	 * rendering time and the buffer time. Also subtract out
	 * additional time if we missed one or more frames. */
	static const int safetyNet = 1500;
	int sleepTime = (int) (vsyncTime - safetyNet - renderingTimeMax);

#if 0
	msg(MSG_INFO, "Sleeping for %6d = %6d(avail) - %d(safety) - %d(rendermax) \n", sleepTime, vsyncTime, safetyNet, renderingTimeMax);
	msg(MSG_INFO, "LastRender=%d AvgRender=%d AvgRenderDev=%d VsyncWait=%d AvgVsyncWait=%df MissedBy=%ld",
	    timeRenderingLastFrame, avgRenderingLastFrame, avgRenderingLastFrameDev, timeWaitingForVsync, avgWaitingForVsync, missedBy);
#endif
	
	postsleep_prev = postswap;
	postswap_prev = postswap;

	if(sleepTime > 0)
	{
		usleep(sleepTime);
		postsleep_prev = kuhl_microseconds();
	}

	return;
}

/** Get swap interval settings and apply them by calling glfwSwapInterval().
 */
static void bufferswap_init(void)
{
	viewmat_swapinterval = kuhl_config_int("bufferswap.swapinterval", -1, -1);

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
		msg(MSG_WARNING, "Set viewmat.swapinterval to -1 to swap buffers during monitor refresh (except when FPS drops below monitor refresh rate).");
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

/** Swaps the buffers using the appropriate settings based on the
    configuration file the user provided.
 */
void bufferswap(void)
{
	/* Call initialization function the first time bufferswap() is
	 * called. */
	static int needsInit = 1;
	if(needsInit)
	{
		bufferswap_init();
		needsInit = 0;
	}
	
	dgr_update(1,0); // DGR Master should send before blocking at swap.

	/* Swap the buffers */
	if(viewmat_swapinterval == 0 ||
	   kuhl_config_boolean("bufferswap.latencyreduce", 1,1) == 0) // if FPS is unrestricted.
		bufferswap_simple();
	else
		bufferswap_latencyreduce();

	dgr_update(0,1); // DGR Slave should receive after swap (and before drawing)
}
