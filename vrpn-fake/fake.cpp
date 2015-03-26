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

myTracker::myTracker( vrpn_Connection *c /*= 0 */ ) :
    vrpn_Tracker( OBJECT_NAME, c )
{
}

void
myTracker::mainloop()
{
    vrpn_gettimeofday(&_timestamp, NULL);
    vrpn_Tracker::timestamp = _timestamp;

    static float angle = 0;
    angle += 0.01f;

    // Position
    pos[0] = sinf( angle ); 
    pos[1] = 0.0f;
    pos[2] = 1.55f;
    printf("Pos = %f %f %f\n", pos[0], pos[1], pos[2]);

    // Orientation
    d_quat[0] = 1.0f;
    d_quat[1] = 0.0f;
    d_quat[2] = 0.0f;
    d_quat[3] = 1.0f;

    char msgbuf[1000];
    int  len = vrpn_Tracker::encode_to(msgbuf);

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
        usleep(10000);
    }
}
