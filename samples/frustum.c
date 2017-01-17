/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Implements a dynamic view frustum where the camera is
 * controlled by the keyboard.
 *
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libkuhl.h"

static GLuint program = 0; /**< id value for the GLSL program */

static float camPos[3]  = {0,1.5,0}; /**< location of camera, 1.5 m eyeheight */

/** Position of screen in world coordinates:
 left, right, bottom, top, near, far */
static const float screen[6] = { -2, 2, 0, 4, -1, -100 };

static kuhl_geometry *modelgeom = NULL;

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

		case GLFW_KEY_A: // left
			camPos[0] -= .2;
			break;
		case GLFW_KEY_D: // right
			camPos[0] += .2;
			break;
		case GLFW_KEY_W: // up
			camPos[1] += .2;
			break;
		case GLFW_KEY_S: // down
			camPos[1] -= .2;
			break;
		case GLFW_KEY_Z: // toward screen
			camPos[2] -= .2;
			if(screen[4]-camPos[2] > -.2)
				camPos[2] = .2+screen[4];
			break;
		case GLFW_KEY_X: // away from screen
			camPos[2] += .2;
			break;
	}
	printf("camera position: ");
	vec3f_print(camPos);

}

/** Draws the 3D scene. */
void display()
{
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
		glClearColor(.2,.2,.2,0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		/* Get the view matrix and the projection matrix */
		float viewMat[16], perspective[16];
		// Instead of using viewmat_*() functions to get view and
		// projection matrices, generate our own.
		// viewmat_get(viewMat, perspective, viewportID);
		const float camLook[3] = { camPos[0], camPos[1], camPos[2]-1 }; // look straight ahead
		const float camUp[3] = { 0, 1, 0 };
		mat4f_lookatVec_new(viewMat, camPos, camLook, camUp);
		mat4f_frustum_new(perspective,
		                  screen[0]-camPos[0], screen[1]-camPos[0], // left, right
		                  screen[2]-camPos[1], screen[3]-camPos[1], // bottom, top
		                  screen[4]-camPos[2], screen[5]-camPos[2]); // near, far

		/* Combine the scale and rotation matrices into a single model matrix.
		   modelMat = scaleMat * rotateMat
		*/
		float modelMat[16];
		mat4f_rotateAxis_new(modelMat, 90, 0, 1, 0);
		//mat4f_identity(modelMat);

		/* Construct a modelview matrix: modelview = viewMat * modelMat */
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, modelMat);

		/* Tell OpenGL which GLSL program the subsequent
		 * glUniformMatrix4fv() calls are for. */
		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();
		
		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   perspective); // value
		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		GLuint renderStyle = 2;
		glUniform1i(kuhl_get_uniform("renderStyle"), renderStyle);
		kuhl_errorcheck();
		/* Draw the geometry using the matrices that we sent to the
		 * vertex programs immediately above */
		kuhl_geometry_draw(modelgeom);

		glUseProgram(0); // stop using a GLSL program.
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
	program = kuhl_create_program("assimp.vert", "assimp.frag");
	modelgeom = kuhl_load_model("models/dabrovic-sponza/sponza.obj", NULL, program, NULL);
	if(modelgeom == 0)
	{
		msg(MSG_FATAL, "Dabrovic sponza scene is required for this example. If needed, modify the filename of the model in main().");
		msg(MSG_FATAL, "http://graphics.cs.williams.edu/data/meshes.xml");
		exit(EXIT_FAILURE);
	}


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
