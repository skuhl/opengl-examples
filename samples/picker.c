/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file This example demonstrates how to draw a HUD cursor and how
 * to use the stencil buffer to determine what piece of geometry the
 * cursor is on. For more information and details, see:
 * http://en.wikibooks.org/wiki/OpenGL_Programming/Object_selection
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

static kuhl_geometry cursor;
static kuhl_geometry triangle;
static kuhl_geometry quad;


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
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		/* Get the view or camera matrix; update the frustum values if needed. */
		float viewMat[16], perspective[16];
		viewmat_get(viewMat, perspective, viewportID);

		/* Calculate an angle to rotate the object. glfwGetTime() gets
		 * the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
		float angle = fmod(glfwGetTime()*45, 360);

		/* Make sure all computers/processes use the same angle */
		dgr_setget("angle", &angle, sizeof(GLfloat));

		/* Create a 4x4 rotation matrix based on the angle we computed. */
		float rotateMat[16];
		mat4f_rotateAxis_new(rotateMat, angle, 0,1,0);

		/* Create a scale matrix. */
		float scaleMatrix[16];
		mat4f_scale_new(scaleMatrix, 3, 3, 3);

		// Modelview = (viewMatrix * scaleMatrix) * rotationMatrix
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, scaleMatrix);
		mat4f_mult_mat4f_new(modelview, modelview, rotateMat);

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
		 * vertex programs immediately above. Use the stencil buffer
		 * to keep track of which object appears on top. */
		if(viewportID == 0)
			glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, -1);
		kuhl_geometry_draw(&triangle);
		
		glStencilFunc(GL_ALWAYS, 2, -1);
		kuhl_geometry_draw(&quad);
		glDisable(GL_STENCIL_TEST);
		
		/* If we have multiple viewports, only draw cursor in the
		 * first viewport. */
		if(viewportID == 0)
		{
			/* Draw the cursor in normalized device coordinates. Don't
			 * use any matrices. */
			float identity[16];
			mat4f_identity(identity);
			glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			                   1, 0, identity);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			                   1, 0, identity);

			/* Disable depth testing so the cursor isn't occluded by
			 * anything. */
			glDisable(GL_DEPTH_TEST);
			kuhl_geometry_draw(&cursor);
			glEnable(GL_DEPTH_TEST);

			/* When we render images on the Oculus, we are rendering
			 * into a multisampled framebuffer object, and we can't
			 * read from the multisample FBO until we have blitted it
			 * into a normal FBO. Here, we get the blitted FBO for the
			 * *previous* frame. */
			GLint fb = viewmat_get_framebuffer(viewportID);
			glBindFramebuffer(GL_FRAMEBUFFER, fb);
			
			GLuint stencilVal = 0;
			kuhl_errorcheck();
			glReadPixels(viewport[0]+viewport[2]/2, viewport[1]+viewport[3]/2,
			             1,1, // get data for 1x1 area (i.e., a pixel)
			             GL_STENCIL_INDEX, // query the stencil buffer
			             GL_UNSIGNED_INT,
			             &stencilVal);
			kuhl_errorcheck();
			if(stencilVal == 1)
				printf("Cursor is on triangle.\n");
			else if(stencilVal == 2)
				printf("Cursor is on quad.\n");
			else
				printf("Cursor isn't on anything.\n");
		}

		glUseProgram(0); // stop using a GLSL program.
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

	GLfloat colorData[] = { 1,0,0,
	                        0,1,0,
	                        0,0,1 };
	kuhl_geometry_attrib(geom, colorData, 3, "in_Color", KG_WARN);
}

void init_geometryCursor(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog, 4, GL_LINES);

	/* The data that we want to draw */
	GLfloat vertexData[] = {-.04, 0, 0,
	                         .04, 0, 0,
	                        0, -.04, 0,
	                        0,  .04, 0 };
	kuhl_geometry_attrib(geom, vertexData, 3, "in_Position", KG_WARN);

	GLfloat colorData[] = { 1,1,1,
	                        1,1,1,
	                        1,1,1,
	                        1,1,1 };
	kuhl_geometry_attrib(geom, colorData, 3, "in_Color", KG_WARN);
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
	GLfloat colorData[] = { 1,0,0,
	                        0,1,0,
	                        0,0,1,
	                        0,1,1 };
	kuhl_geometry_attrib(geom, colorData, 3, "in_Color", KG_WARN);
	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
	                       0, 2, 3 }; // indices of second triangle.
	kuhl_geometry_indices(geom, indexData, 6);

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
	program = kuhl_create_program("triangle-color.vert", "triangle-color.frag");
	glUseProgram(program);
	kuhl_errorcheck();
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	/* Create kuhl_geometry structs for the objects that we want to
	 * draw. */
	init_geometryCursor(&cursor, program);
	init_geometryTriangle(&triangle, program);
	init_geometryQuad(&quad, program);

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
