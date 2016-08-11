/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing a triangle with OpenGL 2.0
 *
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"

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
	}
}


void display()
{
	glClearColor(0,0,0,0); // set clear color to black
	// Clear the screen to black, clear the depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // turn on depth testing

	/* Turn on lighting. By default, the light is where the camera
	 * is. */
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	// Apply light to back and front faces the same way
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	/* Normal vectors should be normalized for proper
	   lighting. GL_NORMALIZE makes OpenGL normalize all normal vectors
	   (regardless of if they already are!). Even if your normal vectors
	   are normalized, they can still be scaled by glScale()---so
	   GL_NORMALIZE is a good idea. Improperly scaled normal vectors can
	   often result in unexpected lighting. */
	glEnable(GL_NORMALIZE);

	kuhl_errorcheck();
	

	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		/* Where is the viewport that we are drawing onto and what is its size? */
		int viewport[4];
		viewmat_get_viewport(viewport, viewportID);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	    
		/* Get the view/camera matrix, update view frustum if necessary. */
		float viewMat[16], projMat[16];
		viewmat_get(viewMat, projMat, viewportID);
	    
		/* Communicate matricies to OpenGL */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMultMatrixf(projMat);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf(viewMat); // view/camera matrix
		kuhl_errorcheck();

		/* Calculate an angle to rotate the object. glfwGetTime() gets
		 * the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
		float angle = fmod(glfwGetTime()*45, 360);
		dgr_setget("angle", &angle, sizeof(GLfloat));

		glScalef(3,3,3); /* Scale triangle (second!) */
#ifdef VICON
		float vpos[3], vorient[16];
		vrpn_get("Wand", NULL, vpos, vorient);
		vec3f_print(vpos);
		mat4f_print(vorient);
		glTranslatef(vpos[0], vpos[1], vpos[2]);
		glMultMatrixf(vorient);
#else
		glRotatef(angle, 0,1,0); /* Rotate triangle (first!) */
#endif

		/* Draw a triangle. Since triangles have two sides, we can
		 * distinguish between the sides of the triangles two different
		 * ways. If you draw the 1st, 2nd, and 3rd vertex in order and
		 * they follow a counter-clockwise order, they triangle is facing
		 * toward you. Furthermore, each vertex can have a normal vector
		 * associated with them. The normal vector may (or may not) point
		 * perpendicular to the face of the triangle and can also be used
		 * to indicate which direction the triangle is facing.
		 *
		 * glFrontFace(GL_CW) can be used to make clockwise ordering be
		 * the front of triangles.
		 *
		 * glCullFace() tells OpenGL that front-facing (GL_FRONT),
		 * back-facing (GL_BACK) or all (GL_FRONT_AND_BACK) should be
		 * culled or removed from the scene.
		 */
		glBegin(GL_TRIANGLES);
		glNormal3f(0,0,1);
		glVertex3f(0,0,0);
		glVertex3f(1,0,0);
		glVertex3f(1,1,0);
		glEnd(); // GL_TRIANGLES

		/* Check for errors. If there are errors, consider adding more
		 * calls to kuhl_errorcheck() in your code. */
		kuhl_errorcheck();
		viewmat_end_eye(viewportID);
	} // finish viewport loop

	viewmat_end_frame();
}

int main(int argc, char** argv)
{
	kuhl_ogl_init(&argc, argv, 512, 512, 20, 4);

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);

	float initPos[3] = {0,0,3};
	float initLook[3] = {0,0,0};
	float initUp[3] = {0,1,0};

	// Initialize DGR
	dgr_init();
	viewmat_init(initPos, initLook, initUp);

	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
