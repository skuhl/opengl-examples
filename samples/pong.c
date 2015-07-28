#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <GL/glew.h>
#include <GL/gl.h>
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
	float speed; // speed of ball (larger=faster)
	float minSpeed;
	float color[3], baseColor[3], fastColor[3];
	float xdir, ydir;
	float xpos, ypos;
} Ball;

Ball ball = {.02, 0, 4, .013, .013, {0,0,0}, {150/255.0,150/255.0,150/255.0}, {50/255.0, 210/255.0, 50/255.0}, 0, 1, 0, 0}; //Create a ball that turns green when it speeds up.

GLuint texIdEarth;
GLuint texIdClouds;
GLuint texIdStars;
float ticks = 200.0f;
float cloudTicks = 200.0f;
float planet[3] = {0.0f,0.0f,0.0f};
float screen_width = 0.0f, screen_height = 0.0f;
GLUquadricObj *earth = NULL;
GLUquadricObj *clouds = NULL;

void clampPaddles()
{
	float frustum[6];
	projmat_get_frustum(frustum, -1, -1);

    // left screen boundary
	if(paddleA.xpos < frustum[0]+paddleA.width/2)
		paddleA.xpos = frustum[0]+paddleA.width/2;
	if(paddleB.xpos < frustum[0]+paddleB.width/2)
		paddleB.xpos = frustum[0]+paddleB.width/2;
	// right screen boundary
	if(paddleA.xpos > frustum[1]-paddleA.width/2)
		paddleA.xpos = frustum[1]-paddleA.width/2;
	if(paddleB.xpos > frustum[1]-paddleB.width/2)
		paddleB.xpos = frustum[1]-paddleB.width/2;
}

/* Called by GLUT whenever a key is pressed. */
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'q':
		case 'Q':
		case 27: // ASCII code for Escape key
			dgr_exit();
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
	float frustum[6];
	projmat_get_frustum(frustum, -1, -1);
	
	if(ball.speed < ball.minSpeed)
    {
	    ball.speed = ball.minSpeed;
    }


	bool isBounce = false;
	if(ball.xpos+ball.radius > frustum[1]) // right wall
	{
		ball.xpos = frustum[1]-ball.radius;
		ball.xdir = -ball.xdir;
		isBounce = true;
	}
	if(ball.ypos > frustum[3]) // top wall
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
			msg(WARNING, "Player 1 (top) loses\n");
			dgr_exit();
			exit(0);
		}
		else // missed paddle and didn't lose
		{
			sleep(1);
			ball.xpos = (frustum[0]+frustum[1])/2.0;
			ball.ypos = (frustum[2]+frustum[3])/2.0;
			ball.speed *= .7; // slow down
			ball.speedUp--;
			hasAlreadyMissed = false;
			
		}
	}
	if(ball.xpos-ball.radius < frustum[0]) // left wall
	{
		ball.xpos = frustum[0]+ball.radius;
		ball.xdir = -ball.xdir;
		isBounce = true;
	}

	if(ball.ypos < frustum[2]) // bottom wall
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
			msg(WARNING, "Player 2 (bottom) loses\n");
			dgr_exit();
			exit(0);
		}
		else // missed paddle and didn't lose
		{
			sleep(1);
			ball.xpos = (frustum[0]+frustum[1])/2.0;
			ball.ypos = (frustum[2]+frustum[3])/2.0;
			ball.speed *= .7; // slow down
			ball.speedUp--;
			hasAlreadyMissed = false;
		}
	}

	
	if(!hasAlreadyMissed)
	{
		// check for player 1 (top) paddle hit
		if(ball.ypos > paddleA.ypos-ball.radius && ball.ydir > 0)
		{
			// if we hit paddle
			if(ball.xpos+ball.radius*.9 > paddleA.xpos-paddleA.width/2 &&
			   ball.xpos-ball.radius*.9 < paddleA.xpos+paddleA.width/2)
			{
				ball.ypos = paddleA.ypos-ball.radius;
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
		if(ball.ypos < paddleB.ypos+ball.radius && ball.ydir < 0)
		{
			// if we hit paddle
			if(ball.xpos+ball.radius*.9 > paddleB.xpos-paddleB.width/2 &&
			   ball.xpos-ball.radius*.9 < paddleB.xpos+paddleB.width/2)
			{
				ball.ypos = paddleB.ypos+ball.radius;
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
		ball.speed = ball.speed / .7; // speed up
		ball.speedUp++;
		ball.color[0] = ball.fastColor[0];
		ball.color[1] = ball.fastColor[1];
		ball.color[2] = ball.fastColor[2];
	}

	// add noise to bounces so they don't bounce perfectly.
	if(isBounce)
	{
		// add more noise as game speeds up.
		int scale = ball.speedUp;
		if(scale > 3)
			scale = 3;

		double newXdir;
		double newYdir;
		do
		{
			newXdir = ball.xdir + (drand48()-.5) / 8.0 * scale;
			newYdir = ball.ydir + (drand48()-.5) / 8.0 * scale;
			
			// normalize direction vector
			float dirLength = sqrtf(newXdir*newXdir + newYdir*newYdir);
			newXdir /= dirLength;
			newYdir /= dirLength;

			// Keep trying new values until we find something that
			// isn't moving too much left/right. Also, force bounces
			// to keep the ball bouncing in the same direction
			// vertically.
		} while(fabs(newYdir) < .2 || ball.ydir * newYdir < 0);
		ball.xdir = newXdir;
		ball.ydir = newYdir;
	}
}

void display()
{
	/* If we are using DGR, send or receive data to keep multiple
	 * processes/computers synchronized. */
	dgr_update();
	
	/* If DGR is enabled, only do this in the master*/
	if(dgr_is_enabled() == 0 || dgr_is_master())
	{
		//Grab the tracking data from VRPN
		/*vrpn_get(TRACKED_OBJ_A, NULL, vrpnPos, vrpnOrient);
		paddleA.xpos = vrpnPos[0];

		vrpn_get(TRACKED_OBJ_B, NULL, vrpnPos, vrpnOrient);
		paddleB.xpos = vrpnPos[0];*/
		
		//Start the ball moving
		if(!startedFlag)
		{
			if(true)//time(NULL)-startTime < 5)
			{
				ball.xdir = 0;
				ball.ydir = 0;
								ball.color[0] = ball.fastColor[0];
				ball.color[1] = ball.fastColor[1];
				ball.color[2] = ball.fastColor[2];
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
	}
	
	/* Syncronize the DGR objects */
	dgr_setget("paddleA", &paddleA, sizeof(Paddle));
	dgr_setget("paddleB", &paddleB, sizeof(Paddle));
	dgr_setget("ball", &ball, sizeof(Ball));
	dgr_setget("planet", planet, sizeof(float)*3);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor3f(1,1,1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float frustum[6];
	projmat_get_frustum(frustum, -1, -1);
	glOrtho(frustum[0], frustum[1], frustum[2], frustum[3], frustum[4], frustum[5]);
	  
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Pick a depth that is between the near and far planes.
	float depth = -(frustum[4] + frustum[5])/2.0;

	// Move the light source
	GLfloat position[] = { 1.0f, -1.0f, depth+5.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	
	// Draw the background stars
	float masterfrust[6];
	projmat_get_master_frustum(masterfrust);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, texIdStars);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	float tickmod = ticks / 200.0f;
	glBegin(GL_QUADS);
	glTexCoord2f(tickmod+1.0f, -tickmod);
	glVertex3f(masterfrust[1], masterfrust[3], depth-3.0);
	glTexCoord2f(tickmod, -tickmod);
	glVertex3f(masterfrust[0], masterfrust[3], depth-3.0);
	glTexCoord2f(tickmod, 1.0f-tickmod);
	glVertex3f(masterfrust[0], masterfrust[2], depth-3.0);
	glTexCoord2f(tickmod+1.0f, 1.0f-tickmod);
	glVertex3f(masterfrust[1], masterfrust[2], depth-3.0);
	glEnd();
	
	//Draw the earth   
   	glMatrixMode(GL_MODELVIEW);
   	glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texIdEarth);
	glTranslatef(planet[0], planet[1], depth-3.0);
	glRotatef(25.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(-90, 1.0f, 0.0f, 0.0f);
	glRotatef(ticks, 0.0f, 0.0f, 1.0f);
	ticks += .005;
	if(ticks > 360.0f)ticks = 0.0f;
	gluSphere(earth, planet[2]*1.65f, 200, 200);
	glPopMatrix();
    
    //Draw the clouds
   	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);   
   	glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texIdClouds);
    glLoadIdentity();
	glTranslatef(planet[0], planet[1], depth-3.0);
	glRotatef(25.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(-90, 1.0f, 0.0f, 0.0f);
	glRotatef(cloudTicks, 1.0f, 0.0f, 1.0f);
	cloudTicks += .005;
	if(cloudTicks > 360.0f)cloudTicks = 0.0f;
	gluSphere(clouds, planet[2]*1.66f, 200, 200);
	glPopMatrix();
	
    
    // Reset somethings for the rest of the scene
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	// top player (player 1) paddle
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(paddleA.xpos,0,depth+5.0f);
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
	glTranslatef(paddleB.xpos,0,depth+5.0f);
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
	glEnable(GL_LIGHTING);
    glColor3fv(ball.color);
	glPushMatrix();
	glTranslatef(ball.xpos, ball.ypos, depth+4.0f);
	glutSolidSphere(ball.radius, 100, 100);
	glPopMatrix();
	
	/* If DGR is enabled, only do this in the master*/
	if(dgr_is_enabled() == 0 || dgr_is_master())
	{
		ball.xpos += ball.xdir * ball.speed;
		ball.ypos += ball.ydir * ball.speed;
		ball.color[0] = ball.color[0] - .005;
		ball.color[1] = ball.color[1] - .005;
		ball.color[2] = ball.color[2] - .005;
		if(ball.color[0] < ball.baseColor[0]) ball.color[0] = ball.baseColor[0];
		if(ball.color[1] < ball.baseColor[1]) ball.color[1] = ball.baseColor[1];
		if(ball.color[2] < ball.baseColor[2]) ball.color[2] = ball.baseColor[2];
		bounceBall();
	}
	
	glFlush();
	glutSwapBuffers();
	glutPostRedisplay(); // call display() repeatedly

}

int main( int argc, char* argv[] )
{	
	startTime = time(NULL);
	
	/* Initialize glut */
	glutInit(&argc, argv); //initialize the toolkit
	glEnable(GL_POINT_SMOOTH);
	glutSetOption(GLUT_MULTISAMPLE, 4); // set msaa samples; default to 4
	/* Ask GLUT to for a double buffered, full color window that includes a depth buffer */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);  //set display mode
	glutInitWindowSize(768, 512); //set window size
	glutInitWindowPosition(0, 0); //set window position on screen
	glutCreateWindow(argv[0]); //open the screen window
	glEnable(GL_MULTISAMPLE);
	
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

	float frustum[6]; // left, right, bottom, top, near, far
	                  // 0     1      2       3    4     5
	projmat_get_frustum(frustum, -1, -1);
	ball.xpos = (frustum[0] + frustum[1])/2.0;
	ball.ypos = (frustum[2] + frustum[3])/2.0;
	paddleA.xpos = ball.xpos;
	paddleA.ypos = frustum[3]-(frustum[3]-frustum[2])/20.0;
	paddleA.width = (frustum[1]-frustum[0])/10.0;
	paddleA.increment = paddleA.width / 3.0;
	paddleA.thickness = (frustum[3]-frustum[2])/5.0;
	
	paddleB.xpos = paddleA.xpos;
	paddleB.ypos = frustum[2]+(frustum[3]-frustum[2])/20.0;
	paddleB.width = paddleA.width;
	paddleB.increment = paddleA.increment;
	paddleB.thickness = paddleA.thickness;
	
	msg(INFO, "Initial ball position %f %f\n", ball.xpos, ball.ypos);
	msg(INFO, "Initial paddle A position %f %f\n", paddleA.xpos, paddleA.ypos);
	msg(INFO, "Initial paddle B position %f %f\n", paddleB.xpos, paddleB.ypos);

	ball.radius = (frustum[1]-frustum[0])/50.0;
	
	planet[0] = ((frustum[0] + frustum[1])/2.0f) - ((frustum[1] - frustum[0])/2.4f);
	planet[1] = ((frustum[2] + frustum[3])/2.0f) - ((frustum[1] - frustum[0])*1.7f);
	planet[2] = (frustum[1] - frustum[0]);
	
	earth = gluNewQuadric();
	clouds = gluNewQuadric();
	gluQuadricDrawStyle(earth, GLU_FILL);
	gluQuadricTexture(earth, GL_TRUE);
	gluQuadricNormals(earth, GLU_SMOOTH);
	gluQuadricDrawStyle(clouds, GLU_FILL);
	gluQuadricTexture(clouds, GL_TRUE);
	gluQuadricNormals(clouds, GLU_SMOOTH);

	// These files are on the filesystem at MTU and are not available
	// to non-MTU users.
	kuhl_read_texture_file("pong/earth.png", &texIdEarth);
	kuhl_read_texture_file("pong/clouds.png", &texIdClouds);
	kuhl_read_texture_file("pong/stars.png", &texIdStars);

	glutMainLoop();
}
