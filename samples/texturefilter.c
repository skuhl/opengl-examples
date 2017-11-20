/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates drawing textured geometry.
 *
 * @author Scott Kuhl
 */

#include "libkuhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static GLuint program = 0; /**< id value for the GLSL program */
static kuhl_geometry quad;

static GLuint texId = 0;   // larger checkerboard
static GLuint texId1 = 0;  // 1px checkerboard
static GLuint usingTexture = 0;
static int anisoOn = 0;

void setTexFilter(GLuint mag, GLuint min)
{
	glBindTexture(GL_TEXTURE_2D, texId1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
}


/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action != GLFW_PRESS)
		return;

	switch(key)
	{
		case GLFW_KEY_T:
			if(usingTexture == texId)
			{
				usingTexture = texId1;
				printf("Using 1x1 checkerboard\n");
			}
			else
			{
				usingTexture = texId;
				printf("Using larger checkerboard\n");
			}
			kuhl_geometry_texture(&quad, usingTexture, "tex", KG_WARN);
			break;
		case GLFW_KEY_1:
			printf("\n");
			printf("Texture filtering: Nearest neighbor\n");
			setTexFilter(GL_NEAREST, GL_NEAREST);
			break;
		case GLFW_KEY_2:
			printf("\n");
			printf("Texture filtering: Bilinear\n");
			setTexFilter(GL_LINEAR, GL_LINEAR);
			break;
		case GLFW_KEY_3:
			printf("\n");
			printf("Texture filtering: Magnify=linear; minify=nearest mipmap & nearest neighbor in mipmap.\n");
			glBindTexture(GL_TEXTURE_2D, usingTexture);
			setTexFilter(GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST);
			break;
		case GLFW_KEY_4:
			printf("\n");
			printf("Texture filtering: Magnify=linear; minify=nearest mipmap & bilinear filtering in mipmap.\n");
			setTexFilter(GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST);
			break;
		case GLFW_KEY_5:
			printf("\n");
			printf("Texture filtering: Magnify=linear; minify=linear mipmap & nearest neighbor in mipmap.\n");
			setTexFilter(GL_LINEAR, GL_NEAREST_MIPMAP_LINEAR);
			break;
		case GLFW_KEY_6:
			printf("\n");
			printf("Texture filtering: Magnify=linear; minify=linear mipmap & bilinear in mipmap.\n");
			setTexFilter(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
			break;
		case GLFW_KEY_A:
			if(glewIsSupported("GL_EXT_texture_filter_anisotropic"))
			{
				if(anisoOn)
				{
					anisoOn = 0;
					glBindTexture(GL_TEXTURE_2D, texId1);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
					glBindTexture(GL_TEXTURE_2D, texId);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
					printf("Anisotropic filtering is off.\n");
				}
				else
				{
					anisoOn = 1;
					float maxAniso;
					glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
					glBindTexture(GL_TEXTURE_2D, texId1);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
					glBindTexture(GL_TEXTURE_2D, texId);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
					printf("Anisotropic filtering is on.\n");
				}
			}
			break;
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

		/* Calculate an angle to rotate the object. glfwGetTime() gets
		 * the time in seconds since GLFW was initialized. Rotates 45 degrees every second. */
		float angle = glfwGetTime()*2;

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
	GLfloat vertexPositions[] = {-10, 0, -10,
	                             10, 0, -10,
	                             10, 0, 10,
	                             -10, 0, 10 };
	kuhl_geometry_attrib(geom, vertexPositions,
	                     3, // number of components x,y,z
	                     "in_Position", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?

	GLfloat texCoord[] = {0, 0,
	                      10, 0,
	                      10, 10,
	                      0, 10 };
	kuhl_geometry_attrib(geom, texCoord,
	                     2, // number of components x,y,z
	                     "in_TexCoord", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?

	
	/* A list of triangles that we want to draw. "0" refers to the
	 * first vertex in our list of vertices. Every three numbers forms
	 * a single triangle. */
	GLuint indexData[] = { 0, 1, 2,  
	                       0, 2, 3 };
	kuhl_geometry_indices(geom, indexData, 6);

	/* Load the texture. It will be bound to texId */	
	kuhl_read_texture_file_wrap("../images/checkerboard-1px.png", &texId1, GL_REPEAT, GL_REPEAT);
	kuhl_read_texture_file_wrap("../images/checkerboard.png", &texId, GL_REPEAT, GL_REPEAT);
	usingTexture = texId1;
	
	/* Tell this piece of geometry to use the texture we just loaded. */
	kuhl_geometry_texture(geom, usingTexture, "tex", KG_WARN);

	/* Set default texture filtering options: */
	glBindTexture(GL_TEXTURE_2D, texId);
	/* Turn off anisotropic filtering once a key is hit (it should
	 * be on by default when the program starts). */
	if(glewIsSupported("GL_EXT_texture_filter_anisotropic"))
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1); // 1=disable
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, texId1);
	/* Turn off anisotropic filtering once a key is hit (it should
	 * be on by default when the program starts). */
	if(glewIsSupported("GL_EXT_texture_filter_anisotropic"))
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1); // 1=disable
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
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
	program = kuhl_create_program("texture.vert", "texture.frag");
	glUseProgram(program);
	kuhl_errorcheck();

	init_geometryQuad(&quad, program);
	
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,1.5,3}; // location of camera
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
