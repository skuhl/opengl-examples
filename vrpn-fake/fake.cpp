/* This program simulates a VRPN server to help support debugging and
   testing without access to a tracking system.
   
   This file is based heavily on a VRPN server tutorial written by
   Sebastian Kuntz for VR Geeks (http://www.vrgeeks.org) in August
   2011.
*/

#define LINE_UP "\033[F"
#define LINE_CLEAR "\033[J"


#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>

#include "vrpn_Text.h"
#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Connection.h"

#include "vecmat.h"
#include "kuhl-util.h"


using namespace std;

class myTracker : public vrpn_Tracker
{
  public:
	myTracker( const char* name, bool n, bool v, bool q, vrpn_Connection *c = 0 );
	myTracker( char** files, bool n, bool v, bool q, vrpn_Connection *c = 0 );
	virtual ~myTracker() {};
	virtual void mainloop();

  protected:
	struct timeval _timestamp;
	kuhl_fps_state fps_state;
	
  private:
  	bool verbose;
  	bool quiet;
  	bool noise;
};

myTracker::myTracker( const char* name, bool n, bool v, bool q, vrpn_Connection *c ) :
	vrpn_Tracker( name, c )
{
	printf("Using tracker name: %s\n", name);
	verbose = v;
	quiet = q;
	noise = n;
	kuhl_getfps_init(&fps_state);
}

myTracker::myTracker( char** files, bool n, bool v, bool q, vrpn_Connection *c ) :
	vrpn_Tracker( "Tracker0", c )
{
	verbose = v;
	quiet = q;
	noise = n;
	kuhl_getfps_init(&fps_state);
}

long lastrecord = 0;
void myTracker::mainloop()
{

	vrpn_gettimeofday(&_timestamp, NULL);
	vrpn_Tracker::timestamp = _timestamp;

	if(!quiet)printf(LINE_CLEAR "Records sent per second: %.1f\n", kuhl_getfps(&fps_state));

	double angle = kuhl_milliseconds_start() / 1000.0;

	// Position
	pos[0] = sin( angle );
	pos[1] = 1.55f; // approx normal eyeheight
	pos[2] = 0.0f;

	if(noise)
	{
		double r[10]; // generate some random numbers to simulate imperfect tracking system.
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
	// mat3f_rotateEuler_new(rotMat, 0, 0, 0, "XYZ");  // no rotation
	mat3f_rotateEuler_new(rotMat, 0, angle*10, 0, "XYZ"); // yaw
	//mat3f_rotateEuler_new(rotMat, r[3]*.05, angle*10 + r[4]*.05, r[5]*.05, "XYZ"); // yaw + noise
	if(!quiet)mat3f_print(rotMat);

	// Convert rotation matrix into quaternion
	float quat[4];
	quatf_from_mat3f(quat, rotMat);
	for(int i=0; i<4; i++)
		d_quat[i] = quat[i];


	char msgbuf[1000];
	int len = vrpn_Tracker::encode_to(msgbuf);
	
	if(!quiet)printf(LINE_CLEAR "Microseconds since last record: %ld\n", kuhl_microseconds()-lastrecord);
	lastrecord = kuhl_microseconds();
	if (d_connection->pack_message(len, _timestamp, position_m_id, d_sender_id, msgbuf,
	                               vrpn_CONNECTION_LOW_LATENCY))
	{
		fprintf(stderr,"can't write message: tossing\n");
	}

	server_mainloop();
}

/**
 * -b (buffer)- Takes one parameter. The size of the file buffer. Default: 2048 data points.
 * TODO-f (files)- Takes one or more parameter, this will read from a log file instead of generating data.
 * -h (help)- Prints a helpful message
 * -n (noise)- Adds noise to each data point.
 * -q (quiet)- Turns off almost all debugging.
 * -t (tracker)- Takes one parameter. Uses the specified name for the tracked object.
 * -v (verbose)- Turns on some extra debugging.
 */
int main(int argc, char* argv[])
{	
	//Options that will be set by the arguments
	bool verbose = false;
	bool quiet = false;
	bool noise = false;
	const char* objectName = "Tracker0";
	int bufSize = 2048;
	
	//Start out with no file names, we will malloc later if we need it.
	//char** filesv = NULL;
	//int filesc = 0;

	//Check the arguments for any options supplied
	//See Linux man(3) getopt for more info
	int option = 0;
	const char* options = "b:hnqt:v";//f:";
	while((option = getopt(argc, argv, options)) != -1){

    	switch(option)
		{
        	case 'b':
				bufSize = atoi(optarg);
        		break;
        	/*case 'f':
				//decriment the option index since it get's auto-incremented twice by getopt to
				//pass over the expected single parameter. We need to support multiple params.
       			for(optind--; optind < argc && argv[optind][0] != '-' && strlen(argv[optind]); optind++)
				{
					//Read all of the file names
					filesc++;
					filesv = (char**)realloc(filesv, filesc * sizeof(char**));
					filesv[filesc-1] = argv[optind];
        		}
        		break;*/
        	case 'h':
				//free up file names incase the user specified them.
				//if(filesv != NULL)free(filesv);
				//print the help message
				printf("Usage: fake [OPTION]...\n");
				printf("Runs a fake vrpn server that simulates a real tracking system.\n");
				printf("If no data files are specified, data will be generated via \n");
				printf("\t-b [SIZE]\tBuffer: the size of the file buffer. Default: 2048 data points.\n");
				//printf("\t-f [FILE]...\tFiles: use the specified data files (one or more).\n");
				printf("\t-h\t\tHelp: print this message.\n");
				printf("\t-n\t\tNoise: adds noise to each data point.\n");
				printf("\t-q\t\tQuiet: turn off most of the debugging.\n");
				printf("\t-t [NAME]\tTracker: rename the tracked object.\n\t\t\t\t NOTE: does nothing if any files are specified.\n");
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
				objectName = optarg;
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
				//if(filesv != NULL)free(filesv);
				exit(1);
				break;
    	}
	}
	if(verbose)
	{
		printf("Options specified:\n");
		printf("  Verbose: %s\n", verbose ? "true" : "false");
		printf("  Quiet: %s\n", quiet ? "true" : "false");
		printf("  Noise: %s\n", noise ? "true" : "false");
		printf("  Buffer Size: %d\n", bufSize);
		printf("  Tracker name: %s\n", objectName);
		/*printf("  Number of files: %d\n", filesc);
		if(filesc > 0)printf("  Files:\n");
		for(int i = 0; i < filesc; i++)
		{
			printf("    %s\n", filesv[i]);
		}*/
		printf("-------------------\n");
	}
	
	if(verbose)printf("Opening VRPN connection\n");
	
	vrpn_Connection_IP* m_Connection = new vrpn_Connection_IP();
	myTracker* serverTracker = new myTracker(objectName, noise, verbose, quiet, m_Connection);

	cout << "Starting VRPN server." << endl;

	while(true)
	{
		serverTracker->mainloop();
		m_Connection->mainloop();
		//Clear out the last couple of lines so that the log isn't spammed
		if(!quiet)printf(LINE_UP LINE_UP LINE_UP LINE_UP LINE_UP LINE_UP LINE_UP);
		kuhl_limitfps(100);
	}
	//if(filesv != NULL)free(filesv);
}
