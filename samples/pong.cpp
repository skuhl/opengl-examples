#include "Client.h"

#include <GL/glew.h>
#include <GL/glut.h>

#include <math.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <ctime>

#ifdef WIN32
#include <conio.h>   // For _kbhit()
#include <cstdio>   // For getchar()
#include <windows.h> // For Sleep()
#endif // WIN32

#include <time.h>

using namespace ViconDataStreamSDK::CPP;

// Make a new client
Client MyClient;
std::string HostName = "141.219.28.17:801";
//std::string HostName = "localhost:801";

// screen width/height indicate the size of the window on our screen (not the size of the display wall). The aspect ratio must match the actual display wall.
const GLdouble SCREEN_WIDTH = (1920*6)/8.0;  
const GLdouble SCREEN_HEIGHT = (1080.0*4)/8.0;
const float screenAspectRatio = SCREEN_WIDTH/SCREEN_HEIGHT;

float player1Xpos=0, player2Xpos=0;
float player1Color1[3] = {87/255.0, 159/255.0, 210/255.0}; // blue
float player1Color2[3] = {19/255.0,119/255.0,189/255.0};
float player2Color1[3] = {225/255.0,95/255.0,93/255.0}; // red
float player2Color2[3] = {220/255.0,50/255.0,47/255.0}; 

float ballPos[2] = { 0,0 };
float ballDir[2] = { 0,1 };
float ballColor[3] = { 0,0,0 };
const float ballGreenColor[3] = { 146/255.0, 158/255.0, 64/255.0 };


int ballBouncesTillSpeedup = 4; // number of paddle hits before we speed up.
int ballBounceCount = 0; // counter of paddle hits so far
float ballSpeed = 70; // speed of ball (larger=slower)
const float ballSpeedSlowest = 70;

time_t startTime = time(NULL);
bool startedFlag = false;
bool hasAlreadyMissed = false; // ball went past paddle.

float paddleWidth1 = .1;
float paddleWidth2 = .1;
const float paddleWidthIncrement = .02;
const float paddleThickness = .04;
const float player2Ypos = -.9;
const float player1Ypos = -player2Ypos;
const float ballRadius = 0.02;



namespace
{
	std::string Adapt( const bool i_Value )
	{
		return i_Value ? "True" : "False";
	}

	std::string Adapt( const Direction::Enum i_Direction )
	{
		switch( i_Direction )
		{
			case Direction::Forward:
				return "Forward";
			case Direction::Backward:
				return "Backward";
			case Direction::Left:
				return "Left";
			case Direction::Right:
				return "Right";
			case Direction::Up:
				return "Up";
			case Direction::Down:
				return "Down";
			default:
				return "Unknown";
		}
	}

	std::string Adapt( const DeviceType::Enum i_DeviceType )
	{
		switch( i_DeviceType )
		{
			case DeviceType::ForcePlate:
				return "ForcePlate";
			case DeviceType::Unknown:
			default:
				return "Unknown";
		}
	}

	std::string Adapt( const Unit::Enum i_Unit )
	{
		switch( i_Unit )
		{
			case Unit::Meter:
				return "Meter";
			case Unit::Volt:
				return "Volt";
			case Unit::NewtonMeter:
				return "NewtonMeter";
			case Unit::Newton:
				return "Newton";
			case Unit::Kilogram:
				return "Kilogram";
			case Unit::Second:
				return "Second";
			case Unit::Ampere:
				return "Ampere";
			case Unit::Kelvin:
				return "Kelvin";
			case Unit::Mole:
				return "Mole";
			case Unit::Candela:
				return "Candela";
			case Unit::Radian:
				return "Radian";
			case Unit::Steradian:
				return "Steradian";
			case Unit::MeterSquared:
				return "MeterSquared";
			case Unit::MeterCubed:
				return "MeterCubed";
			case Unit::MeterPerSecond:
				return "MeterPerSecond";
			case Unit::MeterPerSecondSquared:
				return "MeterPerSecondSquared";
			case Unit::RadianPerSecond:
				return "RadianPerSecond";
			case Unit::RadianPerSecondSquared:
				return "RadianPerSecondSquared";
			case Unit::Hertz:
				return "Hertz";
			case Unit::Joule:
				return "Joule";
			case Unit::Watt:
				return "Watt";
			case Unit::Pascal:
				return "Pascal";
			case Unit::Lumen:
				return "Lumen";
			case Unit::Lux:
				return "Lux";
			case Unit::Coulomb:
				return "Coulomb";
			case Unit::Ohm:
				return "Ohm";
			case Unit::Farad:
				return "Farad";
			case Unit::Weber:
				return "Weber";
			case Unit::Tesla:
				return "Tesla";
			case Unit::Henry:
				return "Henry";
			case Unit::Siemens:
				return "Siemens";
			case Unit::Becquerel:
				return "Becquerel";
			case Unit::Gray:
				return "Gray";
			case Unit::Sievert:
				return "Sievert";
			case Unit::Katal:
				return "Katal";

			case Unit::Unknown:
			default:
				return "Unknown";
		}
	}
}



void viconExit()
{
    MyClient.DisableSegmentData();
//    MyClient.DisableMarkerData();
//    MyClient.DisableUnlabeledMarkerData();
//    MyClient.DisableDeviceData();

	// TODO: Disconnect seems to cause a hang. -Scott Kuhl
    // Disconnect and dispose
    int t = clock();
    std::cout << " Disconnecting..." << std::endl;
    MyClient.Disconnect();
    int dt = clock() - t;
    double secs = (double) (dt)/(double)CLOCKS_PER_SEC;
    std::cout << " Disconnect time = " << secs << " secs" << std::endl;
}

void viconInit()
{
    // Connect to a server
    std::cout << "Connecting to " << HostName << " ..." << std::flush;
	int attemptConnectCount = 0;
	const int MAX_CONNECT_ATTEMPTS=2;
    while( !MyClient.IsConnected().Connected && attemptConnectCount < MAX_CONNECT_ATTEMPTS)
    {
		attemptConnectCount++;
		bool ok = false;
		ok =( MyClient.Connect( HostName ).Result == Result::Success );
		if(!ok)
			std::cout << "Warning - connect failed..." << std::endl;
		std::cout << ".";
		sleep(1);
    }
	if(attemptConnectCount == MAX_CONNECT_ATTEMPTS)
	{
		printf("Giving up making connection to Vicon system\n");
		return;
	}
    std::cout << std::endl;

    // Enable some different data types
    MyClient.EnableSegmentData();
    //MyClient.EnableMarkerData();
    //MyClient.EnableUnlabeledMarkerData();
    //MyClient.EnableDeviceData();

    std::cout << "Segment Data Enabled: "          << Adapt( MyClient.IsSegmentDataEnabled().Enabled )         << std::endl;
    std::cout << "Marker Data Enabled: "           << Adapt( MyClient.IsMarkerDataEnabled().Enabled )          << std::endl;
    std::cout << "Unlabeled Marker Data Enabled: " << Adapt( MyClient.IsUnlabeledMarkerDataEnabled().Enabled ) << std::endl;
    std::cout << "Device Data Enabled: "           << Adapt( MyClient.IsDeviceDataEnabled().Enabled )          << std::endl;

    // Set the streaming mode
    //MyClient.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ClientPull );
    // MyClient.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ClientPullPreFetch );
    MyClient.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ServerPush );

    // Set the global up axis
    MyClient.SetAxisMapping( Direction::Forward, 
                             Direction::Left, 
                             Direction::Up ); // Z-up
    // MyClient.SetGlobalUpAxis( Direction::Forward, 
    //                           Direction::Up, 
    //                           Direction::Right ); // Y-up

    Output_GetAxisMapping _Output_GetAxisMapping = MyClient.GetAxisMapping();
    std::cout << "Axis Mapping: X-" << Adapt( _Output_GetAxisMapping.XAxis ) 
			  << " Y-" << Adapt( _Output_GetAxisMapping.YAxis ) 
			  << " Z-" << Adapt( _Output_GetAxisMapping.ZAxis ) << std::endl;

    // Discover the version number
    Output_GetVersion _Output_GetVersion = MyClient.GetVersion();
    std::cout << "Version: " << _Output_GetVersion.Major << "." 
			  << _Output_GetVersion.Minor << "." 
			  << _Output_GetVersion.Point << std::endl;

}



// an atexit() callback:
void exitCallback()
{
	viconExit();
	return;
}


void clampPaddles()
{
	// left screen boundary
	if(player1Xpos < -1+paddleWidth1/2)
		player1Xpos = -1+paddleWidth1/2;
	if(player2Xpos < -1+paddleWidth2/2)
		player2Xpos = -1+paddleWidth2/2;
	// right screen boundary
	if(player1Xpos > 1-paddleWidth1/2)
		player1Xpos = 1-paddleWidth1/2;
	if(player2Xpos > 1-paddleWidth2/2)
		player2Xpos = 1-paddleWidth2/2;
}

void bounceBall()
{
  if(ballSpeed > ballSpeedSlowest)
    {
      ballSpeed = ballSpeedSlowest;
    }


	bool isBounce = false;
	if(ballPos[0]+ballRadius > 1) // right wall
	{
		ballPos[0] = 1-ballRadius;
		ballDir[0] = -ballDir[0];
		isBounce = true;
	}
	if(ballPos[1] > 1+ballRadius*screenAspectRatio*1.1) // top wall
	{
#if 0
		ballPos[1] = 1;
		ballDir[1] = -ballDir[1];
		isBounce = true;
#endif

		ballBounceCount = 0;
		paddleWidth1 -= paddleWidthIncrement;
		paddleWidth2 += paddleWidthIncrement;
		if(paddleWidth1 < 0.001)
		{
			printf("player 1 (top) loses\n");
			exit(1);
		}
		else // missed paddle and didn't lose
		{
			sleep(1);
			ballPos[0] = 0;
			ballPos[1] = 0;
			ballSpeed /= .7;
			ballBouncesTillSpeedup--;
			hasAlreadyMissed = false;
			
		}
	}
	if(ballPos[0]-ballRadius < -1) // left wall
	{
		ballPos[0] = -1+ballRadius;
		ballDir[0] = -ballDir[0];
		isBounce = true;
	}

	if(ballPos[1] < -1-ballRadius*screenAspectRatio*1.1) // bottom wall
	{
#if 0
		ballPos[1] = -1;
		ballDir[1] = -ballDir[1];
		isBounce = true;
#endif
		ballBounceCount = 0;
		paddleWidth1 += paddleWidthIncrement;
		paddleWidth2 -= paddleWidthIncrement;
		if(paddleWidth2 < 0.001)
		{
			printf("player 2 (bottom) loses\n");
			exit(1);
		}
		else // missed paddle and didn't lose
		{
			sleep(1);
			ballPos[0] = 0;
			ballPos[1] = 0;
			ballSpeed /= .7;
			ballBouncesTillSpeedup--;
			hasAlreadyMissed = false;
		}
	}

	
	if(!hasAlreadyMissed)
	{
		// check for player 1 (top) paddle hit
		if(ballPos[1] > player1Ypos-ballRadius*screenAspectRatio && ballDir[1] > 0)
		{
			// if we hit paddle
			if(ballPos[0]+ballRadius*.9 > player1Xpos-paddleWidth1/2 &&
			   ballPos[0]-ballRadius*.9 < player1Xpos+paddleWidth1/2)
			{
				ballPos[1] = player1Ypos-ballRadius*screenAspectRatio;
				ballDir[1] = -ballDir[1];
				isBounce = true;
				ballBounceCount++;
			}
			else // missed paddle
			{
				hasAlreadyMissed = true;
			}
		}

		// check for player 2 (bottom) paddle hit
		if(ballPos[1] < player2Ypos+ballRadius*screenAspectRatio && ballDir[1] < 0)
		{
			// if we hit paddle
			if(ballPos[0]+ballRadius*.9 > player2Xpos-paddleWidth2/2 &&
			   ballPos[0]-ballRadius*.9 < player2Xpos+paddleWidth2/2)
			{
				ballPos[1] = player2Ypos+ballRadius*screenAspectRatio;
				ballDir[1] = -ballDir[1];
				isBounce = true;
				ballBounceCount++;
			}
			else
			{
				hasAlreadyMissed = true;
			}
		}
	}
	
	// speedup the ball periodically
	if(ballBounceCount == ballBouncesTillSpeedup)
	{
		ballBounceCount = 0;
		ballSpeed = ballSpeed * .7;
		ballBouncesTillSpeedup++;
		ballColor[0] = ballGreenColor[0];
		ballColor[1] = ballGreenColor[1];
		ballColor[2] = ballGreenColor[2];
	}

	// add noise to bounces so they don't bounce perfectly.
	if(isBounce)
	{
		double noise = drand48()/30.0-(1/30.0/2);
		ballDir[0] += noise;
		ballDir[1] -= noise;

		const float minYdir = .2;
		if(fabs(ballDir[1]) < minYdir)
		{
			printf("too little vertical movement\n");
			float diff = minYdir-ballDir[1];
			ballDir[0] -= diff;
			ballDir[1] += diff;
		}
	}

	
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 27 || key == 'q')  // escape key, exit program
		exit(0);

	if(key == 'a')
	{
		player1Xpos -= .01;
		player2Xpos += .01;
		clampPaddles();
	}

	if(key == 'd')
	{
		player1Xpos += .01;
		player2Xpos -= .01;
		clampPaddles();
	}
}


void display()
{
  //  usleep(10000);


	if(!startedFlag)
	{
		if(time(NULL)-startTime < 5)
		{
			ballDir[0] = 0;
			ballDir[1] = 0;
		}
		else
		{
			srand48(startTime);
			startedFlag = true;
			ballDir[1] = 1;
			if(drand48() < .5)
				ballDir[1] = -1;

			ballColor[0] = ballGreenColor[0];
			ballColor[1] = ballGreenColor[1];
			ballColor[2] = ballGreenColor[2];
		}
	}

	glEnable(GL_LIGHTING) ;
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor3f(1,1,1);
	
	// Get a frame
	if(MyClient.GetFrame().Result != Result::Success )
		printf("WARNING: Inside display() and there is no data from Vicon...\n");

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,1,-1,1,-1,1);
	  
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		  
	// units are in millimeters, lets switch to meters
	Output_GetSegmentGlobalTranslation globalTranslate1 = MyClient.GetSegmentGlobalTranslation("HandL", "HandL");
	Output_GetSegmentGlobalTranslation globalTranslate2 = MyClient.GetSegmentGlobalTranslation("HandR", "HandR");
	
	// divide by 1000 to convert from mm to meters. Screen coordinates
	// range from -1 to 1 so we need to divide by ~3 to make the
	// paddle movement on screen approximately match the movement in
	// the room.
	player1Xpos = globalTranslate1.Translation[0]/1000/2;
	player2Xpos = globalTranslate2.Translation[0]/1000/2;
	

	glDisable(GL_LIGHTING);
	// background quad
	glBegin(GL_QUADS);
	glColor3f(.2,.2,.2);
	glVertex3f(1, 1, -.1); // top left
	glVertex3f(-1,1, -.1);
	glColor3f(1,1,1);
	glVertex3f(-1,-1,-.1);
	glVertex3f( 1,-1,-.1);
	glEnd();

	// top player (player 1) paddle
	glPushMatrix();
	glTranslatef(player1Xpos,0,0);
	glBegin(GL_QUADS);
	glColor3fv(player1Color1);
	glVertex3f( paddleWidth1/2, player1Ypos+paddleThickness, 0); // top left
	glVertex3f(-paddleWidth1/2, player1Ypos+paddleThickness, 0);
	glColor3fv(player1Color2);
	glVertex3f(-paddleWidth1/2, player1Ypos, 0);
	glVertex3f( paddleWidth1/2, player1Ypos, 0);
	glEnd();
	glPopMatrix();

	// bottom player (player 2) paddle
	glPushMatrix();
	glTranslatef(player2Xpos,0,0);
	glBegin(GL_QUADS);
	glColor3fv(player2Color1);
	glVertex3f( paddleWidth2/2, player2Ypos, 0); // top left
	glVertex3f(-paddleWidth2/2, player2Ypos, 0);
	glColor3fv(player2Color2);
	glVertex3f(-paddleWidth2/2, player2Ypos-paddleThickness, 0);
	glVertex3f( paddleWidth2/2, player2Ypos-paddleThickness, 0);
	glEnd();
	glPopMatrix();

	// ball
	glColor3fv(ballColor);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(ballPos[0], ballPos[1],0);
	// make ball round even though screen is stretched horizontally
	glScalef(1,screenAspectRatio,1);
	glutSolidSphere(ballRadius, 100, 100);
	glPopMatrix();

	ballPos[0]+=ballDir[0]/ballSpeed;
	ballPos[1]+=ballDir[1]/ballSpeed;
	ballColor[0] = ballColor[0] - .005;
	ballColor[1] = ballColor[1] - .005;
	ballColor[2] = ballColor[2] - .005;
	if(ballColor[0] < 0) ballColor[0] = 0;
	if(ballColor[1] < 0) ballColor[1] = 0;
	if(ballColor[2] < 0) ballColor[2] = 0;
	bounceBall();

	
	glFlush();
	glutSwapBuffers();
	glutPostRedisplay(); // call display() repeatedly

}


int main( int argc, char* argv[] )
{
	glutInit(&argc, argv); //initialize the toolkit
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  //set display mode
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT); //set window size
	glutInitWindowPosition(0, 0); //set window position on screen
	glutCreateWindow(argv[0]); //open the screen window

	int glew_err = glewInit();
	if(glew_err != GLEW_OK)
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(glew_err));

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	atexit(exitCallback);
	viconInit();

	glutMainLoop();

}
