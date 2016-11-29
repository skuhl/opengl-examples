/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file 
  This program can help you evaluate if tearing is occurring on your
  screen. If tearing does occur, you will see lines appearing on the
  screen. If tearing is not occurring, the program should just appear
  to be a flickering window.

   On a default Ubuntu machine, you may need to use a command such as:

   nvidia-settings --assign CurrentMetaMode="HDMI-0: nvidia-auto-select {ForceFullCompositionPipeline=On}"

   to eliminate tearing. Applying this metamode in your xorg.conf file
   may not work since the lightdm login manager may override it.

   The 'nvidia-settings' GUI also has a checkbox for sync to vblank.
   The 'ccsm' program can also allow you to set sync to vblank for the
   Ubuntu compositing window manager.

   Finally, if you are using multiple monitors on Ubuntu and none of
   the above options work, try setting the environment variables in
   /etc/profile as recommended on the following page:
   
   https://wiki.archlinux.org/index.php/NVIDIA

   @author Scott Kuhl, Sam Seltzer-Johnston

 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"

static GLuint program = 0; /**< id value for the GLSL program */


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
		case GLFW_KEY_F:
			// glutFullScreen();
			break;
		case GLFW_KEY_C:
			glfwSetInputMode(kuhl_get_window(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;
	}
}


/** Draws the 3D scene. */
void display()
{
	int tmp=1;
	// send something to DGR so the slaves don't think that the server has died.
	dgr_setget("dummy", &tmp, sizeof(int));

	static int frameCount = 0;
	frameCount++;
	if(frameCount > 60)
	{
		frameCount = 0;
		msg(MSG_INFO, "FPS: %.1f\n", bufferswap_fps());
	}

	static int toggle = 0;
	toggle++;
	if(toggle > 1)
		toggle = 0;
	
	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right). */
	viewmat_begin_frame();
	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		viewmat_begin_eye(viewportID);

		/* Where is the viewport that we are drawing onto and what is its size? */
		int viewport[4]; // x,y of lower left corner, width, height
		viewmat_get_viewport(viewport, viewportID);
		/* Tell OpenGL the area of the window that we will be drawing in. */
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Clear the current viewport. Without glScissor(), glClear()
		 * clears the entire screen. We could call glClear() before
		 * this viewport loop---but in order for all variations of
		 * this code to work (Oculus support, etc), we can only draw
		 * after viewmat_begin_eye(). */
		glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		glEnable(GL_SCISSOR_TEST);
		if(toggle)
			glClearColor(.2,.2,.2,0); // set clear color to grey
		else
			glClearColor(.3,.4,.4,0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}


int main(int argc, char** argv)
{
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program("triangle.vert", "triangle.frag");
	glUseProgram(program);
	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	glUniform1i(kuhl_get_uniform("red"), 1);
	kuhl_errorcheck();
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);


	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,0,10}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);
	
	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}

	exit(EXIT_SUCCESS);
}
