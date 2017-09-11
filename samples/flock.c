/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Draws a single model repeatedly. Useful for doing very
 * simple performance measurements.
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

static kuhl_geometry *fpsgeom = NULL;
static kuhl_geometry *modelgeom = NULL;
static float bbox[6], fitMatrix[16];

/** Initial position of the camera. 1.55 is a good approximate
 * eyeheight in meters.*/
static const float initCamPos[3]  = {0,1.55,0};

/** A point that the camera should initially be looking at. If
 * fitToView is set, this will also be the position that model will be
 * translated to. */
static const float initCamLook[3] = {0,0,-5};

/** A vector indicating which direction is up. */
static const float initCamUp[3]   = {0,1,0};



#define NUM_MODELS 5000
static float positions[NUM_MODELS][3];

#define GLSL_VERT_FILE "assimp.vert"
#define GLSL_FRAG_FILE "assimp.frag"

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


void get_model_matrix(float result[16], float placeToPutModel[3])
{
	float transMat[16];

	/* Translate the model to the point the camera is looking at. */
	mat4f_translateVec_new(transMat, initCamLook);
	mat4f_mult_mat4f_new(result, transMat, fitMatrix);

	// Then translate based on placeToPutModel
	mat4f_translateVec_new(transMat, placeToPutModel);
	mat4f_mult_mat4f_new(result, transMat, result);
	
}


/** Draws the 3D scene. */
void display()
{
	/* Display FPS if we are a DGR master OR if we are running without
	 * dgr. */
	if(dgr_is_master())
	{
		static unsigned int fpscount = 0;
		fpscount++;
		if(fpscount % 10 == 0)
		{
			float fps = bufferswap_fps(); // get current fps
			char message[1024];
			snprintf(message, 1024, "FPS: %0.2f", fps); // make a string with fps on it
			float labelColor[3] = { 1,1,1 };
			float labelBg[4] = { 0,0,0,.3 };

			// If DGR is being used, only display FPS info if we are
			// the master process.
			fpsgeom = kuhl_label_geom(fpsgeom, program, NULL, message, labelColor, labelBg, 24);
		}
	}
	
	/* Ensure the slaves use the same render style as the master
	 * process. */
	int renderStyle = 2;
	dgr_setget("style", &renderStyle, sizeof(int));

	
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

		/* Turn on blending (note, if you are using transparent textures,
		   the transparency may not look correct unless you draw further
		   items before closer items.). */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		/* Get the view or camera matrix; update the frustum values if needed. */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);

		glUseProgram(program);
		kuhl_errorcheck();
		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   perspective); // value

		glUniform1i(kuhl_get_uniform("renderStyle"), renderStyle);

		float modelview[16];
		for(int i=0; i<NUM_MODELS; i++)
		{
			float modelMat[16];
			get_model_matrix(modelMat, positions[i]);

			mat4f_mult_mat4f_new(modelview, viewMat, modelMat); // modelview = view * model

			/* Send the modelview matrix to the vertex program. */
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			                   1, // number of 4x4 float matrices
			                   0, // transpose
			                   modelview); // value

			kuhl_errorcheck();
			kuhl_geometry_draw(modelgeom); /* Draw the model */
			kuhl_errorcheck();
		}

		// aspect ratio will be zero when the program starts (and FPS hasn't been computed yet)
		if(dgr_is_master())
		{
			float stretchLabel[16];
			mat4f_scale_new(stretchLabel, 1/8.0 / viewmat_window_aspect_ratio(), 1/8.0, 1);
			
			/* Position label in the upper left corner of the screen */
			float transLabel[16];
			mat4f_translate_new(transLabel, -.9, .8, 0);
			mat4f_mult_mat4f_new(modelview, transLabel, stretchLabel);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, 0, modelview);

			/* Make sure we don't use a projection matrix */
			float identity[16];
			mat4f_identity(identity);
			glUniformMatrix4fv(kuhl_get_uniform("Projection"), 1, 0, identity);

			/* Don't use depth testing and make sure we use the texture
			 * rendering style */
			glDisable(GL_DEPTH_TEST);
			glUniform1i(kuhl_get_uniform("renderStyle"), 1);
			kuhl_geometry_draw(fpsgeom); /* Draw the quad */
			glEnable(GL_DEPTH_TEST);
			kuhl_errorcheck();
		}

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Update the model for the next frame based on the time. We
	 * convert the time to seconds and then use mod to cause the
	 * animation to repeat. */
	double time = glfwGetTime();
	dgr_setget("time", &time, sizeof(double));
	kuhl_update_model(modelgeom, 0, fmod(time,10));

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	//kuhl_video_record("videoout", 30);
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
	program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);

	dgr_init();     /* Initialize DGR based on environment variables. */
	viewmat_init(initCamPos, initCamLook, initCamUp);

	// Clear the screen while things might be loading
	glClearColor(.2,.2,.2,1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the model from the file
	const char *modelFile = "../models/duck/duck.dae";
	modelgeom = kuhl_load_model(modelFile, NULL, program, bbox);
	if(modelgeom == NULL)
	{
		msg(MSG_FATAL, "Unable to load model: %s", modelFile);
		exit(EXIT_FAILURE);
	}
	kuhl_bbox_fit(fitMatrix, bbox, 1);


	for(int i=0; i<NUM_MODELS; i++)
	{
		positions[i][0] = drand48()*50-25;
		positions[i][1] = drand48()*50-25;
		positions[i][2] = drand48()*50-25;
	}
	
	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
