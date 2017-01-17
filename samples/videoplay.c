/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates using a video file as a texture.
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

static kuhl_geometry quad;
static video_state* video = NULL;
static char *videofilename = NULL;


/* Call to update the video_state struct with the latest information */
static void update_video()
{
	static GLuint texId = 0;
	static long startTime = 0;

	if(video == NULL) // if it is our first time
	{
		/* Get the next frame */
		video = video_get_next_frame(video, videofilename);
		if(video == NULL)
		{
			msg(MSG_FATAL, "Failed to load video file %s\n", videofilename);
			exit(EXIT_FAILURE);
		}
		
		/* Send the new frame to OpenGL */
		texId = kuhl_read_texture_array(video->data, 1920, 1080, 3, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		startTime = kuhl_microseconds();
		
		/* Tell this piece of geometry to use the texture we just loaded. */
		kuhl_geometry_texture(&quad, texId, "tex", KG_WARN);

		/* Get the frame that should be displayed next */
		video = video_get_next_frame(video, videofilename);
	}
	else
	{
		/* If previously loaded frame is to be displayed in the future, do nothing. */
		if(video->usec > kuhl_microseconds()-startTime)
			return;

		/* Display the frame we previously loaded */
		glDeleteTextures(1, &texId);
		texId = kuhl_read_texture_array(video->data, 1920, 1080, 3, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		/* Tell this piece of geometry to use the texture we just
		 * loaded. Frequently, texId won't change because we just
		 * deleted the texture and then reloaded a texture (causing
		 * the same id to be used again). */
		kuhl_geometry_texture(&quad, texId, "tex", KG_WARN);

		/* Get the next frame that should be displayed next */
		video = video_get_next_frame(video, videofilename);
	}

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
	}
}

/** Draws the 3D scene. */
void display()
{
	static int counter = 0;
	counter++;
	if(counter % 60 == 0)
		msg(MSG_INFO, "FPS: %0.2f\n", bufferswap_fps());
	
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
		   items before closer items. This program always draws the
		   geometry in the same order.). */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		/* Get the view or camera matrix; update the frustum values if needed. */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);

		/* Create a scale matrix.  Flip the video vertically since
		   ffmpeg reads in the textures are "upside down" according to
		   OpenGL. */
		float scaleMatrix[16];
		if(video != NULL)
			mat4f_scale_new(scaleMatrix, 3*video->aspectRatio, -3, 3);
		else
			mat4f_scale_new(scaleMatrix, 3, -3, 3);
		
		// Modelview = (viewMatrix * scaleMatrix) * rotationMatrix
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, scaleMatrix);

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
		kuhl_errorcheck();
		/* Draw the geometry using the matrices that we sent to the
		 * vertex programs immediately above */
		kuhl_geometry_draw(&quad);

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	update_video();
}

/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuad(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog,
	                  4, // number of vertices
	                  GL_TRIANGLES); // type of thing to draw

	/* Vertices that we want to form triangles out of. Every 3 numbers
	 * is a vertex position. Below, we provide indices to form
	 * triangles out of these vertices. */
	GLfloat vertexPositions[] = {0, 0, 0,
	                             1, 0, 0,
	                             1, 1, 0,
	                             0, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions,
	                     3, // number of components x,y,z
	                     "in_Position", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?

	/* A list of triangles that we want to draw. "0" refers to the
	 * first vertex in our list of vertices. Every three numbers forms
	 * a single triangle. */
	GLuint indexData[] = { 0, 1, 2,  
	                       0, 2, 3 };
	kuhl_geometry_indices(geom, indexData, 6);

	GLfloat texcoordData[] = {0, 0,
	                          1, 0,
	                          1, 1,
	                          0, 1};
	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);

	
	kuhl_errorcheck();
}


int main(int argc, char** argv)
{
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);

	if(argc != 2)
	{
		msg(MSG_FATAL, "Usage: %s videofile.mp4", argv[0]);
		exit(EXIT_FAILURE);
	}

	videofilename = argv[1];

	
	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program("texture.vert", "texture.frag");
	glUseProgram(program);
	kuhl_errorcheck();

	init_geometryQuad(&quad, program);
	
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
