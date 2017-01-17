#include <stdlib.h>
#ifdef __linux__
#include <unistd.h>
#endif
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"

// Set this to 1 if you want to use the tracking system to control the
// paddles. Set it to 0 to use the keyboard to control the paddles.
#define USE_VRPN 0

// Names of the tracked objects which will form the paddles.
#define TRACKED_OBJ_A "HandL"
#define TRACKED_OBJ_B "HandR"

// These images are available to MTU students on the Linux file system
// on most machines. These files are not included in the git
// repository.
#define STARS "pong/stars.png"
#define EARTH "pong/earth.png"
#define CLOUDS "pong/clouds.png"


#define GS_WAITING 0
#define GS_READY 1
#define GS_PLAYING 2
#define GS_SCORED 3

typedef struct
{
    float width;
    float increment;
    float thickness;
    float color1[3], color2[3];
    float xpos, ypos;
    bool ready;
} Paddle;

typedef struct
{
	float radius;
	int bounceCount; // counter of paddle hits so far
	int speedUp, baseSpeedUp; // number of paddle hits before we speed up.
	float speed; // speed of ball (larger=faster)
	float minSpeed;
	float color[3], baseColor[3], fastColor[3];
	float xdir, ydir;
	float xpos, ypos;
} Ball;


static float vrpnPos[3];
static float vrpnOrient[16];

static time_t startTime = 0;
static int gameState = GS_WAITING;

static Paddle paddleA = {.1, .02, .04, {87/255.0, 159/255.0, 210/255.0}, {19/255.0,119/255.0,189/255.0}, 0, .9, false}; //Create a blue paddle at the top of the screen
static Paddle paddleB = {.1, .02, .04, {220/255.0,50/255.0,47/255.0}, {225/255.0,95/255.0,93/255.0}, 0, -.9, false}; //Create a red paddle at the bottom of the screen
static Ball ball = {.02, 0, 4, 4, .013, .013, {0,0,0}, {255/255.0, 0/255.0, 0/255.0}, {0/255.0, 255/255.0, 0/255.0}, 0, 1, 0, 0}; //Create a ball that turns green when it speeds up.
static float planet[3] = {0.0f,0.0f,0.0f};

static GLUquadricObj *earth = NULL;
static GLUquadricObj *clouds = NULL;
static GLuint texIdEarth;
static GLuint texIdClouds;
static GLuint texIdStars;
static float ticks = 200.0f;

void drawPaddle(Paddle paddle, float depth);

void clampPaddles()
{
	float frustum[6];
	viewmat_get_frustum(frustum, 0);

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

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action != GLFW_PRESS)
		return;
	
	switch(key)
	{
		case GLFW_KEY_Q:
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
#if 0
		case 'f': // full screen
			glutFullScreen();
			break;
		case 'F': // switch to window from full screen mode
			glutPositionWindow(0,0);
			break;
#endif
		case GLFW_KEY_A:
			paddleA.xpos -= .01;
			clampPaddles();
			break;
		case GLFW_KEY_S:
			paddleA.ready = true;
			break;
		case GLFW_KEY_D:
			paddleA.xpos += .01;
			clampPaddles();
			break;
		case GLFW_KEY_J:
			paddleB.xpos -= .01;
			clampPaddles();
			break;
		case GLFW_KEY_K:
			paddleB.ready = true;
			break;
		case GLFW_KEY_L:
			paddleB.xpos += .01;
			clampPaddles();
			break;
	}
}

void game()
{
	float frustum[6];
	viewmat_get_frustum(frustum, 0);

	if(USE_VRPN)
	{
		vrpn_get(TRACKED_OBJ_A, NULL, vrpnPos, vrpnOrient);
		paddleA.xpos = vrpnPos[0];
		if(vrpnPos[1] <= .5)
			paddleA.ready = true;
		
		vrpn_get(TRACKED_OBJ_B, NULL, vrpnPos, vrpnOrient);
		paddleB.xpos = vrpnPos[0];
		if(vrpnPos[1] <= .5)
			paddleB.ready = true;
	}
	
	//Preform the action based on the game state
	switch(gameState)
	{
		//This state indicates that atleast one player is not ready
		case GS_WAITING:
	
			//When both players are ready, shift to the ready state
			if(paddleA.ready && paddleB.ready)
			{
				startTime = time(NULL);
				gameState = GS_READY;
			}
			else
			{
				// Reset the ball to it's starting state
				ball.xpos = (frustum[0]+frustum[1])/2.0;
				ball.ypos = (frustum[2]+frustum[3])/2.0;
				ball.xdir = 0;
				ball.ydir = 0;
				ball.color[0] = ball.baseColor[0];
				ball.color[1] = ball.baseColor[1];
				ball.color[2] = ball.baseColor[2];
			}
			break;
			
		//This state indicates that both players are ready to play
		case GS_READY:
		
			//We should wait in this state for 2 seconds
			if(time(NULL)-startTime >= 2)
			{
				//Start the ball moving either up or down.
				srand48(startTime);
				ball.ydir = 1;
				if(drand48() < .5)
					ball.ydir = -1;
				gameState = GS_PLAYING;
			}
			break;
			
		//This state indicates that the game is currently being played
		case GS_PLAYING:
			
			// Move the ball
			ball.xpos += ball.xdir * ball.speed;
			ball.ypos += ball.ydir * ball.speed;
	
			//Make sure the ball has not slowed down too much
			if(ball.speed < ball.minSpeed)
			{
				ball.speed = ball.minSpeed;
			}

			bool isBounce = false;
	
			//Handle the sides of the play area
			if(ball.xpos-ball.radius < frustum[0]) // left wall
			{
				ball.xpos = frustum[0]+ball.radius;
				ball.xdir = -ball.xdir;
				isBounce = true;
			}

			if(ball.xpos+ball.radius > frustum[1]) // right wall
			{
				ball.xpos = frustum[1]-ball.radius;
				ball.xdir = -ball.xdir;
				isBounce = true;
			}
	
	
			// Handle the Top and the bottom of the play area
			if(ball.ypos > frustum[3] || ball.ypos < frustum[2]) // top orr bottom wall
			{
				gameState = GS_SCORED;
				break;
			}

			
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
			else // If a speedup didn't happen, make the ball more green
			{
				float step = (float)ball.bounceCount / ((float)ball.speedUp-1);
				ball.color[0] = ball.baseColor[0] + ((ball.fastColor[0] - ball.baseColor[0]) * step);
				ball.color[1] = ball.baseColor[1] + ((ball.fastColor[1] - ball.baseColor[1]) * step);
				ball.color[2] = ball.baseColor[2] + ((ball.fastColor[2] - ball.baseColor[2]) * step);
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
			
			break;
			
		//This state indicates that one player just scored
		case GS_SCORED:
			
			//Reset the bounce count, then figure out who scored
			ball.bounceCount = 0;
			bool paddleAScored = (ball.ypos < frustum[2]);
			
			//Change the paddle widths based on who scored
			paddleA.width += paddleA.increment * (paddleAScored ? 1 :-1);
			paddleB.width += paddleB.increment * (paddleAScored ? -1 :1);
			
			if(paddleA.width < 0.001 || paddleB.width < 0.001)//Check if some one lost the game
			{
				msg(MSG_WARNING, "%s Player wins!\n", (paddleAScored ? "Red" : "Blue"));
				
				//Reset the paddles for the next game
				paddleA.width = paddleB.width = (frustum[1]-frustum[0])/10.0;
				
				//Reset the ball for the next game
				ball.speed = ball.minSpeed = (frustum[3]-frustum[2]) / 178.462f;
				ball.speedUp = ball.baseSpeedUp;
			}
			else // Only lost the point, not the game;
			{
				ball.speed *= .7; // slow down
				ball.speedUp--;
			}
			
			//Set the players to not ready and transition to the waiting state
			paddleA.ready = paddleB.ready = false;
			gameState = GS_WAITING;
			break;
	}
}

void display()
{
	viewmat_begin_frame();
	viewmat_begin_eye(0);
	
	/* Syncronize the DGR objects */
	dgr_setget("paddleA", &paddleA, sizeof(Paddle));
	dgr_setget("paddleB", &paddleB, sizeof(Paddle));
	dgr_setget("ball", &ball, sizeof(Ball));
	dgr_setget("planet", planet, sizeof(float)*3);
	dgr_setget("state", &gameState, sizeof(int));

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
	viewmat_get_frustum(frustum, 0);
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
	viewmat_get_master_frustum(masterfrust);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, texIdStars);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	// Draw the background quad with the scrolling star texture
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
	glRotatef(ticks, 1.0f, 0.0f, 1.0f);
	gluSphere(clouds, planet[2]*1.652f, 200, 200);
	glPopMatrix();
	
    // Reset somethings for the rest of the scene
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   
	
	// top player (player 1) paddle	
	drawPaddle(paddleA, depth+5.0f);

	// bottom player (player 2) paddle
	drawPaddle(paddleB, depth+5.0f);
	
	glDisable(GL_BLEND);
	
	// ball
	glEnable(GL_LIGHTING);
    glColor3fv(ball.color);
	glPushMatrix();
	glTranslatef(ball.xpos, ball.ypos, depth+4.0f);
	// glutSolidSphere(ball.radius, 100, 100);
	GLUquadric* sphere;
	sphere = gluNewQuadric();
	gluQuadricNormals(sphere, GL_SMOOTH);
	gluSphere(sphere, ball.radius, 100, 100);
	glPopMatrix();
	
	/* If DGR is enabled, only do this in the master*/
	if(dgr_is_enabled() == 0 || dgr_is_master())
	{
		// Run the game code
		game();	
	}

	viewmat_end_eye(0);
	viewmat_end_frame();
}

void drawPaddle(Paddle paddle, float depth)
{
	glPushMatrix();
	
	//Draw the paddle
	glTranslatef(paddle.xpos-paddle.width/2, paddle.ypos, depth);
	glBegin(GL_QUADS);
	glColor3fv(paddle.color1);
    glVertex3f(0.0f, paddle.thickness, 0.0f); // top left
	glVertex3f(paddle.width, paddle.thickness, 0.0f);
	glColor3fv(paddle.color2);
	glVertex3f(paddle.width, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glPopMatrix();
	
	//Draw a glow around the paddle

	if((gameState == GS_WAITING || gameState == GS_READY) && paddle.ready)
	{
		float heavyGlow[4] = {0.0f,1.0f,0.0f,0.5f};
		float lightGlow[4] = {0.0f,1.0f,0.0f,0.0f};
	
		glPushMatrix();
		glTranslatef(paddle.xpos-paddle.width/2, paddle.ypos, depth+1.1f);
		glBegin(GL_QUADS);
		
		glColor4fv(heavyGlow);
		glVertex3f(0.0f, paddle.thickness, 0.0f); // top left
		glVertex3f(paddle.width, paddle.thickness, 0.0f);

		glColor4fv(lightGlow);
		glVertex3f(paddle.width, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		
		glEnd();
		glPopMatrix();
	}

}

int main( int argc, char* argv[] )
{	
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 768, 512, 20, 4);
	glEnable(GL_POINT_SMOOTH);
	
	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);
	
	/* Initialize DGR */
	dgr_init();     /* Initialize DGR based on environment variables. */
	float initCamPos[3]  = {0,0,10}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);

	float frustum[6]; // left, right, bottom, top, near, far
	                  // 0     1      2       3    4     5
	
	viewmat_get_frustum(frustum, 0);
	ball.xpos = (frustum[0] + frustum[1])/2.0;
	ball.ypos = (frustum[2] + frustum[3])/2.0;
	ball.speed = ball.minSpeed = (frustum[3]-frustum[2]) / 178.462f;
	
	paddleA.xpos = ball.xpos;
	paddleA.ypos = frustum[3]-(frustum[3]-frustum[2])/20.0;
	paddleA.width = (frustum[1]-frustum[0])/10.0;
	paddleA.increment = paddleA.width / 3.0;
	paddleA.thickness = (frustum[3]-frustum[2])/25.0;
	
	paddleB.xpos = paddleA.xpos;
	paddleB.ypos = frustum[2]+(frustum[3]-frustum[2])/20.0;
	paddleB.width = paddleA.width;
	paddleB.increment = paddleA.increment;
	paddleB.thickness = -paddleA.thickness;
	
	msg(MSG_INFO, "Initial ball position %f %f\n", ball.xpos, ball.ypos);
	msg(MSG_INFO, "Initial Ball speed: %f\n", frustum[3]-frustum[2], ball.speed);
	msg(MSG_INFO, "Initial paddle A position %f %f\n", paddleA.xpos, paddleA.ypos);
	msg(MSG_INFO, "Initial paddle B position %f %f\n", paddleB.xpos, paddleB.ypos);

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
		
	kuhl_read_texture_file(EARTH, &texIdEarth);
	kuhl_read_texture_file(CLOUDS, &texIdClouds);
	kuhl_read_texture_file(STARS, &texIdStars);

	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
