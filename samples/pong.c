#include <stdlib.h>
#include <unistd.h>
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

float vrpnPos[3];
float vrpnOrient[16];

time_t startTime = 0;
bool startedFlag = false;
bool hasAlreadyMissed = false; // ball went past paddle.

//Make the paddles
typedef struct
{
    float width;
    float increment;
    float thickness;
    float color1[3], color2[3];
    float xpos, ypos;
} Paddle;

Paddle paddleA = {.1, .02, .04, {87/255.0, 159/255.0, 210/255.0}, {19/255.0,119/255.0,189/255.0}, 0, .9}; //Create a blue paddle at the top of the screen
Paddle paddleB = {.1, .02, .04, {225/255.0,95/255.0,93/255.0}, {220/255.0,50/255.0,47/255.0}, 0, -.9}; //Create a red paddle at the bottom of the screen

//Make the ball
typedef struct
{
	float radius;
	int bounceCount; // counter of paddle hits so far
	int speedUp; // number of paddle hits before we speed up.
	float speed; // speed of ball (larger=slower)
	float minSpeed;
	float color[3], fastColor[3];
	float xdir, ydir;
	float xpos, ypos;
} Ball;

Ball ball = {.02, 0, 4, 70, 70, {0,0,0}, {146/255.0, 158/255.0, 64/255.0}, 0, 1, 0, 0}; //Create a ball that turns green when it speeds up.

void clampPaddles()
{
	// left screen boundary
	if(paddleA.xpos < -1+paddleA.width/2)
		paddleA.xpos = -1+paddleA.width/2;
	if(paddleB.xpos < -1+paddleB.width/2)
		paddleB.xpos = -1+paddleB.width/2;
	// right screen boundary
	if(paddleA.xpos > 1-paddleA.width/2)
		paddleA.xpos = 1-paddleA.width/2;
	if(paddleB.xpos > 1-paddleB.width/2)
		paddleB.xpos = 1-paddleB.width/2;
}

/* Called by GLUT whenever a key is pressed. */
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'q':
		case 'Q':
		case 27: // ASCII code for Escape key
			exit(0);
			break;
		case 'f': // full screen
			glutFullScreen();
			break;
		case 'F': // switch to window from full screen mode
			glutPositionWindow(0,0);
			break;
		case 'a':
			paddleA.xpos -= .01;
			paddleB.xpos += .01;
			clampPaddles();
			break;
		case 'd':
			paddleA.xpos += .01;
			paddleB.xpos -= .01;
			clampPaddles();
			break;
	}
}

void bounceBall()
{
  if(ball.speed > ball.minSpeed)
    {
      ball.speed = ball.minSpeed;
    }


	bool isBounce = false;
	if(ball.xpos+ball.radius > 1) // right wall
	{
		ball.xpos = 1-ball.radius;
		ball.xdir = -ball.xdir;
		isBounce = true;
	}
	if(ball.ypos > 1+ball.radius*screenAspectRatio*1.1) // top wall
	{
#if 0
		ball.ypos = 1;
		ball.ydir = -ball.ydir;
		isBounce = true;
#endif

		ball.bounceCount = 0;
		paddleA.width -= paddleA.increment;
		paddleB.width += paddleB.increment;
		if(paddleA.width < 0.001)
		{
			printf("player 1 (top) loses\n");
			exit(1);
		}
		else // missed paddle and didn't lose
		{
			sleep(1);
			ball.xpos = 0;
			ball.ypos = 0;
			ball.speed /= .7;
			ball.speedUp--;
			hasAlreadyMissed = false;
			
		}
	}
	if(ball.xpos-ball.radius < -1) // left wall
	{
		ball.xpos = -1+ball.radius;
		ball.xdir = -ball.xdir;
		isBounce = true;
	}

	if(ball.ypos < -1-ball.radius*screenAspectRatio*1.1) // bottom wall
	{
#if 0
		ball.ypos = -1;
		ball.ydir = -ball.ydir;
		isBounce = true;
#endif
		ball.bounceCount = 0;
		paddleA.width += paddleA.increment;
		paddleB.width -= paddleB.increment;
		if(paddleB.width < 0.001)
		{
			printf("player 2 (bottom) loses\n");
			exit(1);
		}
		else // missed paddle and didn't lose
		{
			sleep(1);
			ball.xpos = 0;
			ball.ypos = 0;
			ball.speed /= .7;
			ball.speedUp--;
			hasAlreadyMissed = false;
		}
	}

	
	if(!hasAlreadyMissed)
	{
		// check for player 1 (top) paddle hit
		if(ball.ypos > paddleA.ypos-ball.radius*screenAspectRatio && ball.ydir > 0)
		{
			// if we hit paddle
			if(ball.xpos+ball.radius*.9 > paddleA.xpos-paddleA.width/2 &&
			   ball.xpos-ball.radius*.9 < paddleA.xpos+paddleA.width/2)
			{
				ball.ypos = paddleA.ypos-ball.radius*screenAspectRatio;
				ball.ydir = -ball.ydir;
				isBounce = true;
				ball.bounceCount++;
			}
			else // missed paddle
			{
				hasAlreadyMissed = true;
			}
		}

		// check for player 2 (bottom) paddle hit
		if(ball.ypos < paddleB.ypos+ball.radius*screenAspectRatio && ball.ydir < 0)
		{
			// if we hit paddle
			if(ball.xpos+ball.radius*.9 > paddleB.xpos-paddleB.width/2 &&
			   ball.xpos-ball.radius*.9 < paddleB.xpos+paddleB.width/2)
			{
				ball.ypos = paddleB.ypos+ball.radius*screenAspectRatio;
				ball.ydir = -ball.ydir;
				isBounce = true;
				ball.bounceCount++;
			}
			else
			{
				hasAlreadyMissed = true;
			}
		}
	}
	
	// speedup the ball periodically
	if(ball.bounceCount == ball.speedUp)
	{
		ball.bounceCount = 0;
		ball.speed = ball.speed * .7;
		ball.speedUp++;
		ball.color[0] = ball.fastColor[0];
		ball.color[1] = ball.fastColor[1];
		ball.color[2] = ball.fastColor[2];
	}

	// add noise to bounces so they don't bounce perfectly.
	if(isBounce)
	{
		double noise = drand48()/30.0-(1/30.0/2);
		ball.xdir += noise;
		ball.ydir -= noise;

		const float minYdir = .2;
		if(fabs(ball.ydir) < minYdir)
		{
			printf("too little vertical movement\n");
			float diff = minYdir-ball.ydir;
			ball.xdir -= diff;
			ball.ydir += diff;
		}
	}

	
}

void display()
{
	/* If we are using DGR, send or receive data to keep multiple
	 * processes/computers synchronized. */
	dgr_update();
	
	/* Try to connect to VRPN server */
	vrpn_get(TRACKED_OBJ_A, NULL, vrpnPos, vrpnOrient);
	paddleA.xpos = vrpnPos[0];

	vrpn_get(TRACKED_OBJ_B, NULL, vrpnPos, vrpnOrient);
	paddleB.xpos = vrpnPos[0];
	
	
	
	
	if(!startedFlag)
	{
		if(time(NULL)-startTime < 5)
		{
			ball.xdir = 0;
			ball.ydir = 0;
		}
		else
		{
			srand48(startTime);
			startedFlag = true;
			ball.ydir = 1;
			if(drand48() < .5)
				ball.ydir = -1;

			ball.color[0] = ball.fastColor[0];
			ball.color[1] = ball.fastColor[1];
			ball.color[2] = ball.fastColor[2];
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
	glTranslatef(paddleA.xpos,0,0);
	glBegin(GL_QUADS);
	glColor3fv(paddleA.color1);
	glVertex3f( paddleA.width/2, paddleA.ypos+paddleA.thickness, 0); // top left
	glVertex3f(-paddleA.width/2, paddleA.ypos+paddleA.thickness, 0);
	glColor3fv(paddleA.color2);
	glVertex3f(-paddleA.width/2, paddleA.ypos, 0);
	glVertex3f( paddleA.width/2, paddleA.ypos, 0);
	glEnd();
	glPopMatrix();

	// bottom player (player 2) paddle
	glPushMatrix();
	glTranslatef(paddleB.xpos,0,0);
	glBegin(GL_QUADS);
	glColor3fv(paddleB.color1);
	glVertex3f( paddleB.width/2, paddleB.ypos, 0); // top left
	glVertex3f(-paddleB.width/2, paddleB.ypos, 0);
	glColor3fv(paddleB.color2);
	glVertex3f(-paddleB.width/2, paddleB.ypos-paddleB.thickness, 0);
	glVertex3f( paddleB.width/2, paddleB.ypos-paddleB.thickness, 0);
	glEnd();
	glPopMatrix();

	// ball
	glColor3fv(ball.color);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(ball.xpos, ball.ypos,0);
	
	// make ball round even though screen is stretched horizontally
	glScalef(1,screenAspectRatio,1);
	glutSolidSphere(ball.radius, 100, 100);
	glPopMatrix();

	ball.xpos+=ball.xdir/ball.speed;
	ball.ypos+=ball.ydir/ball.speed;
	ball.color[0] = ball.color[0] - .005;
	ball.color[1] = ball.color[1] - .005;
	ball.color[2] = ball.color[2] - .005;
	if(ball.color[0] < 0) ball.color[0] = 0;
	if(ball.color[1] < 0) ball.color[1] = 0;
	if(ball.color[2] < 0) ball.color[2] = 0;
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
	
	/* Initialize DGR */
	dgr_init();     /* Initialize DGR based on environment variables. */
	projmat_init(); /* Figure out which projection matrix we should use based on environment variables */
	
	glutMainLoop();
}
