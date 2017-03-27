/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file This program demonstrates prerendering a scene into a
 * texture before displaying it.
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
static GLuint prerendProgram = 0;

#define USE_MSAA 1
static GLuint prerenderFrameBuffer = 0;
static GLuint prerenderFrameBufferAA = 0;
static GLuint prerenderTexID = 0;

static GLuint prerenderWidth = 1024;
static GLuint prerenderHeight = 1024;

static kuhl_geometry triangle;
static kuhl_geometry quad;
static kuhl_geometry prerendQuad;


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

		case GLFW_KEY_R:
		{
			// Reload GLSL program from disk
			kuhl_delete_program(prerendProgram);
			prerendProgram = kuhl_create_program("prerend.vert", "prerend.frag");
			/* Apply the program to the model geometry */
			kuhl_geometry_program(&prerendQuad, prerendProgram, KG_FULL_LIST);
			break;
		}
	}

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
		viewmat_get(viewMat, perspective, viewportID);

		// The perspective projection matrix we receive from
		// viewmat_get() accounts for the aspect ratio of the
		// window. Our aspect ratio should be fixed to the aspect
		// ratio of the texture that we are prerendering to.
		mat4f_perspective_new(perspective, 70, prerenderWidth/(float)prerenderHeight, .1, 100);


		/* Calculate an angle to rotate the object. glfwGetTime() gets
		 * the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
		float angle = fmod(glfwGetTime()*45, 360);

		/* Make sure all computers/processes use the same angle */
		dgr_setget("angle", &angle, sizeof(GLfloat));

		/* Create a 4x4 rotation matrix based on the angle we computed. */
		float rotateMat[16];
		mat4f_rotateAxis_new(rotateMat, angle, 0,1,0);

		/* Create a scale matrix. */
		float scaleMat[16];
		mat4f_scale_new(scaleMat, 3, 3, 3);

		// Modelview = (viewMatrix * scaleMat) * rotationMatrix
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, scaleMat);
		mat4f_mult_mat4f_new(modelview, modelview, rotateMat);

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

		/* Setup prerender directly to texture once (and reuse the
		 * framebuffer object for subsequent draw commands). */
		if(prerenderFrameBuffer == 0)
		{
#if USE_MSAA==1
			/* Generate a MSAA framebuffer + texture */
			GLuint prerenderTexIDAA;
			prerenderFrameBufferAA = kuhl_gen_framebuffer_msaa(prerenderWidth, prerenderHeight,
			                                                   &prerenderTexIDAA,
			                                                   NULL, 16);
#endif
			prerenderFrameBuffer = kuhl_gen_framebuffer(prerenderWidth, prerenderHeight,
			                                            &prerenderTexID,
			                                            NULL);
			/* Apply the texture to our geometry and draw the quad. */
			kuhl_geometry_texture(&prerendQuad, prerenderTexID, "tex", 1);
		}
		/* Switch to framebuffer and set the OpenGL viewport to cover
		 * the entire framebuffer. */
#if USE_MSAA==1
		glBindFramebuffer(GL_FRAMEBUFFER, prerenderFrameBufferAA);
#else
		glBindFramebuffer(GL_FRAMEBUFFER, prerenderFrameBuffer);
#endif
		glViewport(0,0,prerenderWidth, prerenderHeight);
		kuhl_errorcheck();

		/* Clear the framebuffer and the depth buffer. */
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		/* Draw the geometry using the matrices that we sent to the
		 * vertex programs immediately above */
		kuhl_geometry_draw(&triangle);
		kuhl_geometry_draw(&quad);

		/* Stop rendering to texture */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		kuhl_errorcheck();
		
#if USE_MSAA==1
		/* Copy the MSAA framebuffer into the normal framebuffer */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, prerenderFrameBufferAA);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prerenderFrameBuffer);
		glBlitFramebuffer(0,0,prerenderWidth,prerenderHeight,
		                  0,0,prerenderWidth,prerenderHeight,
		                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
		kuhl_errorcheck();
#endif

		/* Set up the viewport to draw on the screen */
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		glUseProgram(prerendProgram);
		kuhl_geometry_draw(&prerendQuad);


		
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

}

void init_geometryTriangle(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog, 3, // num vertices
	                  GL_TRIANGLES); // primitive type

	/* Vertices that we want to form triangles out of. Every 3 numbers
	 * is a vertex position. Since no indices are provided, every
	 * three vertex positions form a single triangle.*/
	GLfloat vertexPositions[] = {0, 0, 0,
	                             1, 0, 0,
	                             1, 1, 0};
	kuhl_geometry_attrib(geom, vertexPositions, // data
	                     3, // number of components (x,y,z)
	                     "in_Position", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?
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
	GLfloat vertexPositions[] = {0+1.1, 0, 0,
	                             1+1.1, 0, 0,
	                             1+1.1, 1, 0,
	                             0+1.1, 1, 0 };
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

	kuhl_errorcheck();
}

/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuadPrerender(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog, 4, GL_TRIANGLES);


	/* The data that we want to draw */
	GLfloat vertexPositions[] = {-1, -1, 0,
	                        1, -1, 0,
	                        1, 1, 0,
	                        -1, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions,
	                     3, // number of components x,y,z
	                     "in_Position", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?

	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
	                       0, 2, 3 }; // indices of second triangle.
	kuhl_geometry_indices(geom, indexData, 6);

	GLfloat texcoordData[] = {0, 0,
	                          1, 0,
	                          1, 1,
	                          0, 1};
	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);
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

	/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
	   the correct program. */
	glUseProgram(program);
	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	glUniform1i(kuhl_get_uniform("red"), 0);
	kuhl_errorcheck();
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	/* Create kuhl_geometry structs for the objects that we want to
	 * draw. */
	init_geometryTriangle(&triangle, program);
	init_geometryQuad(&quad, program);

	prerendProgram = kuhl_create_program("prerend.vert", "prerend.frag");
	init_geometryQuadPrerender(&prerendQuad, prerendProgram);	

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,0,3}; // location of camera
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
