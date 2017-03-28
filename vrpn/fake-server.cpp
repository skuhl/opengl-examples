/* This program simulates a VRPN server to help support debugging and
   testing without access to a tracking system.
   
   This file is based heavily on a VRPN server tutorial written by
   Sebastian Kuntz for VR Geeks (http://www.vrgeeks.org) in August
   2011.
   
   Modified by John Thomas to support multiple Tracked objects
   and reading from log files. (2015) 
*/

#define LINE_UP "\033[F"
#define LINE_CLEAR "\033[J"
#define DATA_TRACKER 0
#define FILE_TRACKER 1

#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <iostream>
#include <cstdlib>
#include <string>
#include <cerrno>
#include <time.h>

#include "vrpn_Text.h"
#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Connection.h"
#include "vecmat.h"
#include "kuhl-util.h"
#include "tdl-util.h"

using namespace std;

class myTracker : public vrpn_Tracker
{
  public:
	myTracker( const char* name, bool* flags, vrpn_Connection *c = 0, FILE* fs = NULL );
	virtual ~myTracker() {};
	virtual void mainloop();

  protected:
	struct timeval _timestamp;
	kuhl_fps_state fps_state;
	
  private:
  	bool verbose;
  	bool quiet;
  	bool noise;
  	bool type;
  	char* trackerName;
  	FILE *fs;
  	int modifier;
  	long lastrecord;
};

myTracker::myTracker( const char* name, bool* flags, vrpn_Connection *c, FILE* fs ) :
	vrpn_Tracker( name, c )
{
	printf("Using tracker name: %s\n", name);
	this->trackerName = (char*)name;
	this->verbose = flags[0];
	this->quiet = flags[1];
	this->noise = flags[2];
	this->type = flags[3];
	this->fs = fs;
	this->modifier = ((double) rand() / (RAND_MAX)) * (360);
	this->lastrecord = kuhl_microseconds();
	kuhl_getfps_init(&fps_state);
}

void myTracker::mainloop()
{
	vrpn_gettimeofday(&_timestamp, NULL);
	vrpn_Tracker::timestamp = _timestamp;

	float filePos[3];
	float fileOrient[9];
	static int timeThroughData = 1;
	if(type == FILE_TRACKER)
	{
		int readVal = tdl_read(fs, filePos, fileOrient);
	
		if(readVal == 1)  // end of file
		{
			timeThroughData++;

			// When we reach end of file, start over again from beginning of file.
			if(tdl_prepare(fs, NULL) == -1)
			{
				printf("Error going back to beginning of file.\n");
				exit(EXIT_FAILURE);
			}

			// read data so that server has info to serve
			tdl_read(fs, filePos, fileOrient);
		}
		else if(readVal == -1)  // error
		{
			exit(EXIT_FAILURE);
		}
	}
	
	if(!quiet)
	{
		printf(LINE_CLEAR "%s (%d time though file):\n", trackerName, timeThroughData);	
		printf(LINE_CLEAR "Records sent per second: %.1f\n", kuhl_getfps(&fps_state));	
	}
	
	double angle = kuhl_milliseconds_start() / 1000.0;

	// Position
	if(type == DATA_TRACKER)
	{
		pos[0] = sin( angle + this->modifier ) ;
		pos[1] = 1.55f; // approx normal eyeheight
		pos[2] = 0.0f;
	}
	else
	{
		pos[0] = filePos[0];
		pos[1] = filePos[1];
		pos[2] = filePos[2];
	}
	
	double r[10];
	if(noise)
	{
		// generate some random numbers to simulate imperfect tracking system
		for(int i=0; i<10; i++)
		{
			r[i] = 0;
			r[i] = kuhl_gauss(); // generate some random numbers
		}
	
		// Add random noise to position
		pos[0] += r[0] * .10;
		pos[1] += r[1] * .01;
		pos[2] += r[2] * .01;
	}

	if(!quiet)printf(LINE_CLEAR "Pos = %f %f %f\n", pos[0], pos[1], pos[2]);
	
	// Orientation
	float rotMat[9];
	if(type == DATA_TRACKER)
	{
		if(noise)
		{
			mat3f_rotateEuler_new(rotMat, r[3]*.05, angle*10 + r[4]*.05, r[5]*.05, "XYZ"); // yaw + noise
		}
		else
		{
			mat3f_rotateEuler_new(rotMat, 0, angle*10, 0, "XYZ"); // yaw
		}
	}
	else
	{
		rotMat[0] = fileOrient[0];
		rotMat[1] = fileOrient[1];
		rotMat[2] = fileOrient[2];
		rotMat[3] = fileOrient[3];
		rotMat[4] = fileOrient[4];
		rotMat[5] = fileOrient[5];
		rotMat[6] = fileOrient[6];
		rotMat[7] = fileOrient[7];
		rotMat[8] = fileOrient[8];
	}
	
	if(!quiet)mat3f_print(rotMat);
		
	// Convert rotation matrix into quaternion
	float quat[4];
	quatf_from_mat3f(quat, rotMat);
	for(int i=0; i<4; i++)
		d_quat[i] = quat[i];


	char msgbuf[1000];
	int len = vrpn_Tracker::encode_to(msgbuf);

	long now_microsecs = kuhl_microseconds();
	if(!quiet)printf(LINE_CLEAR "Microseconds since last record: %ld\n", now_microsecs-lastrecord);
	lastrecord = now_microsecs;
	if (d_connection->pack_message(len, _timestamp, position_m_id, d_sender_id, msgbuf,
	                               vrpn_CONNECTION_LOW_LATENCY))
	{
		fprintf(stderr,"can't write message: tossing\n");
	}

	server_mainloop();
}

/**
 * -f (files)- Takes one or more parameters, this will read from a log file instead of generating data.
 * -h (help)- Prints a helpful message
 * -n (noise)- Adds noise to each data point.
 * -q (quiet)- Turns off almost all debugging.
 * -t (tracker)- Takes one or more parameters. Uses the specified names for the tracked objects, multiple names will create multiple objects.
 * -v (verbose)- Turns on some extra debugging.
 */
int main(int argc, char* argv[])
{	
	srand(time(NULL));
	//Options that will be set by the arguments
	bool verbose = false;
	bool quiet = false;
	bool noise = false;
	char** objNamesv = NULL;
	int objNamesc = 0;
	
	//Start out with no file names, we will malloc later if we need it.
	char** filesv = NULL;
	int filesc = 0;

	//Check the arguments for any options supplied
	//See Linux man(3) getopt for more info
	int option = 0;
	const char* options = "f:hnqt:v";
	while((option = getopt(argc, argv, options)) != -1){

    	switch(option)
		{
        	case 'f':
				//decriment the option index since it get's auto-incremented twice by getopt to
				//pass over the expected single parameter. We need to support multiple params.
       			for(optind--; optind < argc && argv[optind][0] != '-' && strlen(argv[optind]); optind++)
				{
					//Read all of the file names
					filesc++;
					filesv = (char**)realloc(filesv, filesc * sizeof(char**));
					filesv[filesc-1] = argv[optind];
        		}
        		break;
        	case 'h':
				//free up file names incase the user specified them.
				if(filesv != NULL)free(filesv);
				if(objNamesv != NULL)free(objNamesv);
				//print the help message
				printf("Usage: fake [OPTION]...\n");
				printf("Runs a fake vrpn server that simulates a real tracking system.\n");
				printf("If no data files are specified, data will be generated.\n");
				printf("\t-f [FILE]...\tFiles: use the specified data files (one or more).\n");
				printf("\t-h\t\tHelp: print this message.\n");
				printf("\t-n\t\tNoise: adds noise to each data point.\n");
				printf("\t-q\t\tQuiet: turn off most of the debugging.\n");
				printf("\t-t [NAME]...\tTracker: use the specified names for tracked objects.\n\t\t\t\t NOTE: does nothing if any files are specified.\n");
				printf("\t-v\t\tVerbose: turn on extra debugging.\n");
				exit(0);
        		break;
        	case 'n':
				noise = true;
        		break;
        	case 'q':
				quiet = true;
				verbose = false;
        		break;
        	case 't':
        		//decriment the option index since it get's auto-incremented twice by getopt to
				//pass over the expected single parameter. We need to support multiple params.
       			for(optind--; optind < argc && argv[optind][0] != '-' && strlen(argv[optind]); optind++)
				{
					//Read all of the file names
					objNamesc++;
					objNamesv = (char**)realloc(objNamesv, objNamesc * sizeof(char**));
					objNamesv[objNamesc-1] = argv[optind];
        		}
        		break;
        	case 'v':
				verbose = true;
				quiet = false;
        		break;
			default:
				//This is an urecognized option
				if(optind < argc)
				{
					fprintf(stderr,"Unknow option: %c\n", (char)option);
				}
				//free up files incase the user specified them.
				if(filesv != NULL)free(filesv);
				if(objNamesv != NULL)free(objNamesv);
				exit(1);
				break;
    	}
	}
	
	//if the user didn't specify a tracker or a file, use the default traker name.
	if(filesc == 0 && objNamesc == 0)
	{
		objNamesv = (char**)malloc(sizeof(char**));
		objNamesv[0] = (char*)("Tracker0");
		objNamesv[1] = (char*)("Tracker1");
		objNamesc = 2;
	}
	
	if(verbose)
	{
		printf("Options specified:\n");
		printf("  Verbose: %s\n", verbose ? "true" : "false");
		printf("  Quiet: %s\n", quiet ? "true" : "false");
		printf("  Noise: %s\n", noise ? "true" : "false");
		printf("  Number of trackers: %d\n", objNamesc);
		if(objNamesc > 0)printf("  Trackers:\n");
		for(int i = 0; i < objNamesc; i++)
		{
			printf("    %s\n", objNamesv[i]);
		}
		printf("  Number of files: %d\n", filesc);
		if(filesc > 0)printf("  Files:\n");
		for(int i = 0; i < filesc; i++)
		{
			printf("    %s\n", filesv[i]);
		}
		printf("-------------------\n");
	}

	
	if(verbose)printf("Opening VRPN connection\n");
	vrpn_Connection_IP* m_Connection = new vrpn_Connection_IP();
	
	//Set the tracker type to file if files were specified, otherwise set it to data type.
	bool trackerst = filesc > 0 ? FILE_TRACKER : DATA_TRACKER;
	
	//Set the tracker count to the number of either files or data trackers depending on the type.
	int trackersc = trackerst ? filesc : objNamesc;
	
	//Make an array of tracker objects that is the size of the number of trackers we need.
	myTracker* trackersv[128];
	if(trackersc > 128)
	{
		fprintf(stderr, "Too many tracker objects specified.\n");
		exit(EXIT_FAILURE);
	}
	
	bool flags[4] = {verbose, quiet, noise, trackerst};
	for(int i = 0; i < trackersc; i++)
	{
		if(trackerst == DATA_TRACKER)
		{
			trackersv[i] = new myTracker(objNamesv[i], flags, m_Connection);
		}
		else
		{
			FILE *fs = fopen(filesv[i], "r");
			if(fs == NULL)
			{
				fprintf(stderr, "Failed to open file \"%s\": %s\n", filesv[i], strerror(errno));
    			exit(1);
    		}
    		
    		char* name;
			tdl_prepare(fs, &name);	
			
			if(verbose)printf("Creating tracker for %s from file %s\n", name, filesv[i]);
			trackersv[i] = new myTracker(name, flags, m_Connection, fs);
			
		}
	}

	printf("Starting VRPN server.\n");
	
	while(true)
	{
		for(int i = 0; i < trackersc; i++)
		{
			trackersv[i]->mainloop();
		}
		for(int i = 0; i < trackersc; i++)
		{
			//Clear out the last couple of lines so that the log isn't spammed
			if(!quiet)printf(LINE_UP LINE_UP LINE_UP LINE_UP LINE_UP LINE_UP LINE_UP LINE_UP);
		}

		m_Connection->mainloop();
		kuhl_limitfps(100);
	}
	if(filesv != NULL)free(filesv);
	if(objNamesv != NULL)free(objNamesv);
}
