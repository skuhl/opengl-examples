#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include "kuhl-util.h"
#include "vecmat.h"
#include "dgr.h"
#include "projmat.h"
#include "viewmat.h"
#include "vrpn-help.h"

#define TRACKED_OBJ_A "HandL"
#define TRACKED_OBJ_B "HandR"

// Make a new client
//Client MyClient;
//std::string HostName = "141.219.28.17:801";
//std::string HostName = "localhost:801";

// screen width/height indicate the size of the window on our screen (not the size of the display wall). The aspect ratio must match the actual display wall.
#define SCREEN_WIDTH ((1920*6)/8.0)  
#define SCREEN_HEIGHT ((1080.0*4)/8.0)
const float screenAspectRatio = SCREEN_WIDTH/SCREEN_HEIGHT;

float player1Xpos=0, player2Xpos=0;
float player1Color1[3] = {87/255.0, 159/255.0, 210/255.0}; // blue
float player1Color2[3] = {19/255.0,119/255.0,189/255.0};
float player2Color1[3] = {225/255.0,95/255.0,93/255.0}; // red
float player2Color2[3] = {220/255.0,50/255.0,47/255.0}; 
float vrpnPos[3];
float vrpnOrient[16];
float ballPos[2] = { 0,0 };
float ballDir[2] = { 0,1 };
float ballColor[3] = { 0,0,0 };
const float ballGreenColor[3] = { 146/255.0, 158/255.0, 64/255.0 };


int ballBouncesTillSpeedup = 4; // number of paddle hits before we speed up.
int ballBounceCount = 0; // counter of paddle hits so far
float ballSpeed = 70; // speed of ball (larger=slower)
const float ballSpeedSlowest = 70;

time_t startTime = 0;
bool startedFlag = false;
bool hasAlreadyMissed = false; // ball went past paddle.

float paddleWidth1 = .1;
float paddleWidth2 = .1;
const float paddleWidthIncrement = .02;
const float paddleThickness = .04;
const float player2Ypos = -.9;
const float player1Ypos = .9; //This should be -player2Ypos but c doesn't like expressions in intializers, so it's now harcoded
const float ballRadius = 0.02;

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
			//sleep(1);
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
			//sleep(1);
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
	/* If we are using DGR, send or receive data to keep multiple
	 * processes/computers synchronized. */
	//dgr_update();

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

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,1,-1,1,-1,1);
	  
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	
	/* Try to connect to VRPN server */
	vrpn_get(TRACKED_OBJ_A, NULL, vrpnPos, vrpnOrient);
	player1Xpos = vrpnPos[0];
	//printf("x1: %f\n", vrpnPos[0]);
	vrpn_get(TRACKED_OBJ_B, NULL, vrpnPos, vrpnOrient);
	player2Xpos = vrpnPos[0];
	//printf("x2: %f\n", vrpnPos[0]);
	
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
	startTime = time(NULL);
	
	/* Initialize glut */
	glutInit(&argc, argv); //initialize the toolkit
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  //set display mode
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT); //set window size
	glutInitWindowPosition(0, 0); //set window position on screen
	glutCreateWindow(argv[0]); //open the screen window
	
	/* Initialize glew */
	int glew_err = glewInit();
	if(glew_err != GLEW_OK)
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(glew_err));

	/* Initialize call backs */
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	
	glutMainLoop();
}
