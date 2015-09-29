/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include <string>

#ifndef MISSING_VRPN
#include <vrpn_Tracker.h>
#include <quat.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "kuhl-util.h"
#include "vecmat.h"
#include "kalman.h"

#ifndef MISSING_VRPN


/** A mapping of object\@tracker strings to vrpn_Tracker_Remote objects
 * so we can quickly find the appropriate object given an
 * object\@tracker string. */
std::map<std::string, vrpn_Tracker_Remote*> nameToTracker;
/** A mapping of object\@tracker strings to the data that the callback
 * functions store the data into. */
std::map<std::string, vrpn_TRACKERCB> nameToCallbackData;

static kuhl_fps_state fps_state;
static kalman_state kalman;

static void smooth(vrpn_TRACKERCB &t)
{
#if 0
	// TODO: We need to use the times returned by VRPN, not our own time.
	double smoothed = kalman_estimate(&kalman, t.pos[0]);
	// printf("%ld, %lf, %lf\n", kuhl_milliseconds(), t.pos[0], smoothed);
	t.pos[0] = smoothed;
#endif
}


/** A callback function that will get called whenever the tracker
 * provides us with new data. This may be called repeatedly for each
 * record that we have missed if many records have been delivered
 * since the last call to the VRPN mainloop() function. */
static void VRPN_CALLBACK handle_tracker(void *name, vrpn_TRACKERCB t)
{
	float fps = kuhl_getfps(&fps_state);
	if(fps_state.frame == 0)
		msg(INFO, "VRPN records per second: %.1f\n", fps);

	/* Some tracking systems return large values when a point gets
	 * lost. If the tracked point seems to be lost, ignore this
	 * update. */
	float pos[3];
	vec3f_set(pos, t.pos[0], t.pos[1], t.pos[2]);
	
	long microseconds = (t.msg_time.tv_sec* 1000000L) + t.msg_time.tv_usec;

	if(0)
	{
		printf("Current time %ld; VRPN record time: %ld\n", kuhl_microseconds(), microseconds);
		printf("Received position from vrpn: ");
		vec3f_print(pos);
	}
	
	if(vec3f_norm(pos) > 100)
		return;
	
	// Store the data in our map so that someone can use it later.
	std::string s = (char*)name;
	nameToCallbackData[s] = t;
	smooth(nameToCallbackData[s]);
}

#endif

extern "C" {

/** Returns the default hostname based on the contents of
    "~/.vrpn-server". Returns NULL on failure.

    @return NULL on failure or a string which should eventually be
    free()'d.
 */
char* vrpn_default_host()
{
	/* Try reading VRPN server information from ~/.vrpn-server
	   
	   This file should contain a single line that says something like:
	   tcp://VRPN.SERVER.IP.ADDR
	*/
	const char *homedir = getenv("HOME");
	char path[1024];
	snprintf(path, 1024, "%s/.vrpn-server", homedir);
	FILE *f = fopen(path, "r");
	if(f == NULL)
	{
		msg(WARNING, "Can't open file %s to get VRPN server information.\n", path);
		return NULL;
	}
	char *vrpnString = (char*)malloc(1024);
	*vrpnString='\0';

	do
	{
		if(fgets(vrpnString, 1024, f) == NULL)
		{
			msg(WARNING, "Can't read %s to get VRPN server information.\n", path);
			return NULL;
		}
		kuhl_trim_whitespace(vrpnString);
	} while(*vrpnString == '#' || strlen(vrpnString) == 0);  // allow for comments and blank lines in vrpn-server file.
	fclose(f);

	// msg(DEBUG, "Found in %s: '%s'\n", path, vrpnString);
	return vrpnString;
	
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
	printf("You are missing VRPN support.\n");
	return 0;
#else
	if(object == NULL || strlen(object) == 0)
	{
		msg(WARNING, "Empty or NULL object name was passed into this function.\n");
		return 0;
	}
	if(hostname != NULL && strlen(hostname) == 0)
	{
		msg(WARNING, "Hostname is an empty string.\n");
		return 0;
	}
	
	/* Construct an object@hostname string. */
	std::string hostnamecpp;
	std::string objectcpp;
	if(hostname == NULL)
	{
		char *hostnameInFile = vrpn_default_host();
		if(hostnameInFile)
			hostnamecpp = hostnameInFile;
		else
		{
			msg(ERROR, "Failed to find hostname of VRPN server.\n");
			exit(EXIT_FAILURE);
		}
		
	}
	else
		hostnamecpp = hostname;

	objectcpp = object;
	std::string fullname = objectcpp + "@" + hostnamecpp;

	/* Check if we have a tracker object for that string in our map. */
	if(nameToTracker.count(fullname))
	{
		/* If we already have a tracker object, ask it to run the main
		 * loop (and therefore call our handle_tracker() function if
		 * there is new data). */
		nameToTracker[fullname]->mainloop();

		/* If our callback has been called, get the callback object
		 * and get the data out of it. */
		if(nameToCallbackData.count(fullname))
		{
			vrpn_TRACKERCB t = nameToCallbackData[fullname];
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
			 * (left-handed coordinate system)
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
			 * Z = points OUT of the screen (i.e., -Z points into the screen in te IVS lab)
			 * (right-handed coordinate system)
			 *
			 * Below, we convert the position and orientation
			 * information into the OpenGL convention.
			 */
			if(strlen(hostnamecpp.c_str()) > 14 && strncmp(hostnamecpp.c_str(), "tcp://141.219.", 14) == 0) // MTU vicon tracker
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
	}
	else
	{
		/* If this is our first time, create a tracker for the object@hostname string, register the callback handler. */
		msg(INFO, "Connecting to VRPN server: %s\n", hostnamecpp.c_str());
		// If we are making a TCP connection and the server isn't up, the following function call may hang for a long time
		vrpn_Connection *connection = vrpn_get_connection_by_name(hostnamecpp.c_str());

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
		    msg(ERROR, "Failed to connect to tracker: %s\n", fullname.c_str());
		    return 0;
		}
		vrpn_Tracker_Remote *tkr = new vrpn_Tracker_Remote(fullname.c_str(), connection);
		nameToTracker[fullname] = tkr;
		tkr->register_change_handler((void*) fullname.c_str(), handle_tracker);
		kuhl_getfps_init(&fps_state);
		kalman_initialize(&kalman, 0.1, 0.1);
	}
	return 0;
#endif
}


	
} // extern C
