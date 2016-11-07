/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <map>
#include <string>

#ifndef MISSING_VRPN
#include <vrpn_Tracker.h>
#include <quat.h>
#endif

#include "windows-compat.h"
#include "kuhl-util.h"
#include "vecmat.h"
#include "kalman.h"
#include "vrpn-help.h"

#ifndef MISSING_VRPN


/** A struct which we will create for every single tracked
 * object. These will be in a map so that we can easily find the
 * struct that corresponds with an object using the "object\@tracker"
 * notation. */
typedef struct {
	vrpn_Tracker_Remote *tracker; /**< The VRPN tracker for this object */
	vrpn_TRACKERCB data; /**< Data we receive from VRPN (we receive it in the callback */
	int hasData; /**< Has data been written to? */
	int failCount; /**< Number of times vrpn_get() has been called with hasData == 0 */
	kuhl_fps_state fps_state; /**< Track how many records per second this object has sent us */
	kalman_state kalman[7]; /**< Kalman filter state for this object */
} TrackedObject;

/** A mapping of object\@tracker strings to vrpn_Tracker_Remote objects
 * so we can quickly find the appropriate object given an
 * object\@tracker string. */
std::map<std::string, TrackedObject*> nameToTracker;


static void smooth(TrackedObject *to)
{
	long microseconds = (to->data.msg_time.tv_sec* 1000000L) + to->data.msg_time.tv_usec;
	/* Smooth position */
	for(int i=0; i<3; i++)
	{
		to->data.pos[i] = kalman_estimate(&(to->kalman[i]),
		                                  to->data.pos[i],
		                                  microseconds);
	}

	/* Smooth orientation */
	for(int i=0; i<4; i++)
	{
		to->data.quat[i] = kalman_estimate(&(to->kalman[3+i]),
		                                   to->data.quat[i],
		                                   microseconds);
	}
	
	// printf("%ld, %lf, %lf\n", kuhl_milliseconds(), t.pos[0], smoothed);
}

static void vrpn_sanity_check(const struct timeval lastTime,
                              const struct timeval thisTime,
                              const char *name)
{
	long lastTime_usec = (lastTime.tv_sec * 1000000L) + lastTime.tv_usec;
	long thisTime_usec = (thisTime.tv_sec * 1000000L) + thisTime.tv_usec;
	long elapsed       = thisTime_usec - lastTime_usec;
	long budget        = 1000000/55; // 55 records per second
	if(elapsed > 1000000/55)
	{
		msg(MSG_WARNING, "It took %d microseconds between two records for VRPN object %s; time budget for 55 records per second is %d\n", elapsed, name, budget);
	}
}


/** A callback function that will get called whenever the tracker
 * provides us with new data. This may be called repeatedly for each
 * record that we have missed if many records have been delivered
 * since the last call to the VRPN mainloop() function. */
static void VRPN_CALLBACK handle_tracker(void *name, vrpn_TRACKERCB t)
{
	std::string s = (char*)name;
	TrackedObject *tracked = nameToTracker[s];
		
	float fps = kuhl_getfps(&(tracked->fps_state));
	if(tracked->fps_state.frame == 0)
		msg(MSG_INFO, "VRPN records per second: %.1f (%s)\n", fps, s.c_str());

	/* Some tracking systems return large values when a point gets
	 * lost. If the tracked point seems to be lost, ignore this
	 * update. */
	float pos[3], quat[4];
	vec3f_set(pos, t.pos[0], t.pos[1], t.pos[2]);
	vec4f_set(quat, t.quat[0], t.quat[1], t.quat[2], t.quat[3]);

	if(tracked->hasData)
		vrpn_sanity_check(tracked->data.msg_time, t.msg_time, (char*)name);

	if(0)
	{
		long microseconds = (t.msg_time.tv_sec* 1000000L) + t.msg_time.tv_usec;
		printf("Current time %ld; VRPN record time: %ld\n", kuhl_microseconds(), microseconds);
		printf("Received position from vrpn: ");
		vec3f_print(pos);
		printf("Received quat from vrpn: ");
		vec4f_print(quat);
	}
	
	if(vec3f_norm(pos) > 100)
		return;
	
	// Store the data so we can use it later.
	tracked->data = t;
	smooth(tracked);
	tracked->hasData = 1;
}

/** Establish a VRPN connection to a specified host.

    @param fullname A string containing either the hostname or
    something in the format of object\@hostname.

    @return Returns 0 if connection failed, 1 otherwise.
 */
static int vrpn_connect(const char *fullname)
{
	msg(MSG_INFO, "Connecting to VRPN server to track '%s'\n", fullname);

	/* If we are making a TCP connection and the server isn't up, the
	 * following function call may hang for a long time. Also, the
	 * documentation indicates that if we call this function multiple
	 * times with the same hostname, the same connection will be
	 * returned (and new connections won't be made). Accepts
	 * object@hostname notation. */
	vrpn_Connection *connection = vrpn_get_connection_by_name(fullname);
	/* Wait for a bit to see if we can connect. Sometimes we don't immediately connect! */
	for(int i=0; i<1000 && !connection->connected(); i++)
	{
		usleep(1000); // 1000 microseconds * 1000 = up to 1 second of waiting.
		connection->mainloop();
	}
	/* If connection failed, exit. */
	if(!connection->connected())
	{
		delete connection;
		msg(MSG_ERROR, "Failed to connect to tracker: %s\n", fullname);
		return 0;
	}

	/* Create a vrpn_Tracker_Remove object, register the callback function. */
	vrpn_Tracker_Remote *tkr = new vrpn_Tracker_Remote(fullname, connection);
	tkr->register_change_handler((void*) strdup(fullname), handle_tracker);

	/* Store all of the information we will need later about this tracked object */
	TrackedObject *to = (TrackedObject*) malloc(sizeof(TrackedObject));
	to->tracker = tkr;
	to->hasData = 0;
	to->failCount = 0;
	kuhl_getfps_init(&(to->fps_state));

	/* Initialize kalman filter */
	for(int i=0; i<3; i++) /* position */
		kalman_initialize(&(to->kalman[i]), 0.00004f, 0.01f);
	for(int i=3; i<7; i++) /* orientation */
		kalman_initialize(&(to->kalman[i]), 0.0001f, 0.01f);
		
	nameToTracker[std::string(fullname)] = to;
	return 1;
}

/** Retrieves the latest information from a VRPN connection and stores
    it in an appropriate TrackerObject.

 @param fullname A string in the format object\@hostname.

 @param pos An array to store the resulting position data.

 @param orient A matrix to store the resulting orientation data.

 @return Returns 1 on success; 0 on failure.
*/
static int vrpn_update(const char *fullname, float pos[3], float orient[16])
{
	TrackedObject *to = nameToTracker[fullname];
	if(to == NULL)
	{
		msg(MSG_FATAL, "vrpn_update() was called before vrpn_connect() was called for object '%s'", fullname);
		return 0;
	}
		
	/* If we already have a tracker object, ask it to run the main
	 * loop (and therefore call our handle_tracker() function if
	 * there is new data). */
	to->tracker->mainloop();

	if(to->hasData == 0) /* If mainloop() called our callback, hasData should be 1 */
	{
		const static int maxmessages = 4;  /** How many times should error messages be displayed */
		const static int messagemod = 500; /** How many times does vrpn_get() get called before message is printed */

		/* Don't repeatedly print messages about this */
		if(to->failCount >= maxmessages*messagemod)
			return 0;

		/* If our callback never seems to get called for this
		 * object, print a warning message about it */
		to->failCount++;
		if(to->failCount % messagemod == 0)
		{
			msg(MSG_WARNING, "VRPN has not received any data for %s", fullname);
			msg(MSG_WARNING, "As a result, you may see VRPN messages about receiving no response from server.");
			if(to->failCount == messagemod*maxmessages)
				msg(MSG_WARNING, "This is your last message about %s", fullname);
		}

		return 0;
	}

	/* If we get to here, to->hasData indicates that the VRPN callback
	 * has been called at least once and we have therefore received some data. */
	to->failCount = 0;
		
	vrpn_TRACKERCB t = to->data;
	float pos4[4];
	for(int i=0; i<3; i++)
		pos4[i] = t.pos[i];
	pos4[3]=1;

	double orientd[16];
	// Convert quaternion into orientation matrix.
	q_to_ogl_matrix(orientd, t.quat);
	for(int i=0; i<16; i++)
		orient[i] = (float) orientd[i];

	/* VICON in the MTU IVS lab is typically calibrated so that:
	 * X = points to the right (while facing screen)
	 * Y = points into the screen
	 * Z = up
	 * (right-handed coordinate system)
	 *
	 * PPT is typically calibrated so that:
	 * X = the points to the wall that has two closets at both corners
	 * Y = up
	 * Z = points to the door
	 * (right-handed coordinate system)
	 *
	 * By default, OpenGL assumes that:
	 * X = points to the right (while facing screen in the IVS lab)
	 * Y = up
	 * Z = points OUT of the screen (i.e., -Z points into the screen in the IVS lab)
	 * (right-handed coordinate system)
	 *
	 * Below, we convert the position and orientation
	 * information into the OpenGL convention.
	 */
	if(vrpn_is_vicon(fullname)) // MTU vicon tracker
	{
		float viconTransform[16] = { 1,0,0,0,  // column major order!
		                             0,0,-1,0,
		                             0,1,0,0,
		                             0,0,0,1 };
		mat4f_mult_mat4f_new(orient, viconTransform, orient);
		mat4f_mult_vec4f_new(pos4, viconTransform, pos4);
		vec3f_copy(pos,pos4);
		return 1; // we successfully collected some data
	}
	else // Non-Vicon tracker
	{
		/* Don't transform other tracking systems */
		// orient is already filled in
		vec3f_copy(pos, pos4);
		return 1; // we successfully collected some data
	}
}



#endif // ifndef MISSING_VRPN

extern "C" {

/** Returns 1 if we are connecting to the Vicon tracker in the IVS lab.
 */
int vrpn_is_vicon(const char *hostname)
{
	/* The hostname may or may not have tcp:// in front of it. */
	// TODO: Find a better way...
	if(strstr(hostname, "192.168.11.1") != NULL) // Hard-wired vicon IP address
		return 1;
	if(strstr(hostname, "141.219.") != NULL)
		return 1;
	return 0;
}

/** Returns the default hostname based on the vrpn.server
    configuration variable. Returns NULL on failure.

    @return NULL on failure or a string. The returned string should
    NOT be free()'d because the same string will be returned each time
    we call this function.
 */
const char* vrpn_default_host(void)
{
	const char* vrpnserver = kuhl_config_get("vrpn.server");
	msg(MSG_DEBUG, "Using VRPN server: %s\n", vrpnserver);
	return vrpnserver;
}



/** Given an VRPN object name and a (possibily NULL) hostname,
    generate a object\@hostname string. The ~/.vrpn-server file
    will be consulted if the hostname argument is NULL.

    @param object Name of the VRPN object.
    @param hostname Hostname or IP address of the VRPN server
    @param result The array for the resulting string.
 */
void vrpn_fullname(const char* object, const char* hostname, char result[256])
{
	if(object == NULL || strlen(object) == 0)
	{
		msg(MSG_FATAL, "Empty or NULL object name was passed into this function.\n");
		exit(EXIT_FAILURE);
	}
	if(hostname != NULL && strlen(hostname) == 0)
	{
		msg(MSG_FATAL, "Hostname is an empty string.\n");
		exit(EXIT_FAILURE);
	}
	
	/* Construct an object@hostname string. */
	if(hostname == NULL)
	{
		const char *hostnameInFile = vrpn_default_host();
		if(hostnameInFile == NULL)
		{
			msg(MSG_FATAL, "Failed to find hostname of VRPN server.\n");
			exit(EXIT_FAILURE);
		}
		   
		snprintf(result, 255, "%s@%s", object, hostnameInFile);
		return;
	}

	snprintf(result, 255, "%s@%s", object, hostname);
}

	

/** Uses the VRPN library to get the position and orientation of a
 * tracked object.
 *
 * @param object The name of the object being tracked.
 *
 * @param hostname The IP address or hostname of the VRPN server or
 * tracking system computer. If hostname is set to NULL, the
 * ~/.vrpn-server file is consulted.
 *
 * @param pos An array to be filled in with the position information
 * for the tracked object. If we are unable to track the object, a
 * message may be printed and pos will be set to a fixed value.
 *
 * @param orient An array to be filled in with the orientation matrix
 * for the tracked object. The orientation matrix is in row-major
 * order can be used with OpenGL. If the tracking system is moving an
 * object around on the screen, this matrix can be used directly. If
 * the tracking system is moving the OpenGL camera, this matrix may
 * need to be inverted. If we are unable to track the object, a
 * message may be printed and orient will be set to the identity
 * matrix.
 *
 * @return 1 if we returned data from the tracker. 0 if there was
 * problems connecting to the tracker.
 */
int vrpn_get(const char *object, const char *hostname, float pos[3], float orient[16])
{
	/* Set to default values */
	vec3f_set(pos, 10000,10000,10000);
	mat4f_identity(orient);
#ifdef MISSING_VRPN
	msg(MSG_ERROR, "You are missing VRPN support.\n");
	return 0;
#else

	/* Combine object and hostname into 'object@hostname'. Also, find
	 * default hostname if it is NULL. */
	char fullname[256];
	vrpn_fullname(object, hostname, fullname);

	/* Check if we have a tracker object for that string in our map. */
	if(nameToTracker.count(fullname))
		return vrpn_update(fullname, pos, orient);
	else
		return vrpn_connect(fullname);

#endif
}

/** Gets a set of records from VRPN before they are processed. This is
    currently used to analyze the measurement error of a stationary
    tracked point. If you just want information from the tracker for a
    regular program, see vrpn_get().

    @param object The object to collect information about.
    
    @param hostname The hostname of the VRPN server. If NULL, the
    hostname specified in ~/.vrpn-server is used.

    @param count Number of records.

    @return An array containing count * 7 floats. The first three
    floats are the x,y,z position. The next four floats are the
    quaternion. The returned data should be free()'d.
 */
float* vrpn_get_raw(const char *object, const char *hostname, int count)
{
#ifdef MISSING_VRPN
	msg(MSG_ERROR, "You are missing VRPN support.\n");
	return 0;
#else

	/* Make sure we are connected */
	float pos[4], orient[16];
	vrpn_get(object, hostname, pos, orient);

	char fullname[256];
	vrpn_fullname(object, hostname, fullname);

	TrackedObject *to = nameToTracker[std::string(fullname)];

	/* Disable kalman filtering */
	for(int i = 0; i<7; i++)
		to->kalman[i].isEnabled = 0;
	
	float *data = (float*) malloc(sizeof(float)*7*count);

	for(int i=0; i<count; i++)
	{
		while(to->hasData == 0)
			to->tracker->mainloop();
		to->hasData = 0;

		data[i*7+0] = to->data.pos[0];
		data[i*7+1] = to->data.pos[1];
		data[i*7+2] = to->data.pos[2];
		data[i*7+3] = to->data.quat[0];
		data[i*7+4] = to->data.quat[1];
		data[i*7+5] = to->data.quat[2];
		data[i*7+6] = to->data.quat[3];
	}
	return data;
#endif
}
	


} // extern C
