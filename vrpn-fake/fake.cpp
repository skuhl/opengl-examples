// This file is based heavily on a VRPN server tutorial written by
// Sebastian Kuntz for VR Geeks (http://www.vrgeeks.org) in August
// 2011.

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

#define OBJECT_NAME "Tracker0"

using namespace std;

class myTracker : public vrpn_Tracker
{
  public:
	myTracker( vrpn_Connection *c = 0 );
	virtual ~myTracker() {};

	virtual void mainloop();

  protected:
	struct timeval _timestamp;
};

myTracker::myTracker( vrpn_Connection *c ) :
	vrpn_Tracker( OBJECT_NAME, c )
{
}

void myTracker::mainloop()
{
	vrpn_gettimeofday(&_timestamp, NULL);
	vrpn_Tracker::timestamp = _timestamp;

	static float angle = 0;
	angle += 0.01f;

	// Position
	pos[0] = sinf( angle ); 
	pos[1] = 1.55f; // approx normal eyeheight
	pos[2] = 0.0f;
	printf("Pos = %f %f %f\n", pos[0], pos[1], pos[2]);

	// Orientation
	float rotMat[9];
	// mat3f_rotateEuler_new(rotMat, 0, 0, 0, "XYZ");
	mat3f_rotateEuler_new(rotMat, 0, angle*10, 0, "XYZ");
	mat3f_print(rotMat);

	// Convert rotation matrix into quaternion
	float quat[4];
	quatf_from_mat3f(quat, rotMat);
	for(int i=0; i<4; i++)
		d_quat[i] = quat[i];


	char msgbuf[1000];
	int len = vrpn_Tracker::encode_to(msgbuf);

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

	cout << "Created VRPN server." << endl;

	while(true)
	{
		serverTracker->mainloop();
		m_Connection->mainloop();

		int second = 1000000;
		usleep(second / 100); // 1 second
	}
}
