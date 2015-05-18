/* This program simulates a VRPN server to help support debugging and
   testing without access to a tracking system.
   
   This file is based heavily on a VRPN server tutorial written by
   Sebastian Kuntz for VR Geeks (http://www.vrgeeks.org) in August
   2011.
*/

#define OBJECT_NAME "Tracker0"

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <iostream>

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
	myTracker( vrpn_Connection *c = 0 );
	virtual ~myTracker() {};

	virtual void mainloop();

  protected:
	struct timeval _timestamp;
	kuhl_fps_state fps_state;
};

myTracker::myTracker( vrpn_Connection *c ) :
	vrpn_Tracker( OBJECT_NAME, c )
{
	kuhl_getfps_init(&fps_state);
}


long lastrecord = 0;
void myTracker::mainloop()
{
	vrpn_gettimeofday(&_timestamp, NULL);
	vrpn_Tracker::timestamp = _timestamp;

	printf("\n");
	printf("Records sent per second: %.1f\n", kuhl_getfps(&fps_state));

	double angle = kuhl_milliseconds_start() / 1000.0;

	// Position
	pos[0] = sin( angle );
	pos[1] = 1.55f; // approx normal eyeheight
	pos[2] = 0.0f;

#if 0
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
#endif
	printf("Pos = %f %f %f\n", pos[0], pos[1], pos[2]);

	// Orientation
	float rotMat[9];
	// mat3f_rotateEuler_new(rotMat, 0, 0, 0, "XYZ");  // no rotation
	mat3f_rotateEuler_new(rotMat, 0, angle*10, 0, "XYZ"); // yaw
	//mat3f_rotateEuler_new(rotMat, r[3]*.05, angle*10 + r[4]*.05, r[5]*.05, "XYZ"); // yaw + noise
	mat3f_print(rotMat);

	// Convert rotation matrix into quaternion
	float quat[4];
	quatf_from_mat3f(quat, rotMat);
	for(int i=0; i<4; i++)
		d_quat[i] = quat[i];


	char msgbuf[1000];
	int len = vrpn_Tracker::encode_to(msgbuf);

	printf("Microseconds since last record: %ld\n", kuhl_microseconds()-lastrecord);
	lastrecord = kuhl_microseconds();
	if (d_connection->pack_message(len, _timestamp, position_m_id, d_sender_id, msgbuf,
	                               vrpn_CONNECTION_LOW_LATENCY))
	{
		fprintf(stderr,"can't write message: tossing\n");
	}

	server_mainloop();
}

int main(int argc, char* argv[])
{
	vrpn_Connection_IP* m_Connection = new vrpn_Connection_IP();
	myTracker* serverTracker = new myTracker(m_Connection);

	cout << "Starting VRPN server." << endl;

	while(true)
	{
		serverTracker->mainloop();
		m_Connection->mainloop();

		kuhl_limitfps(100);
	}
}
