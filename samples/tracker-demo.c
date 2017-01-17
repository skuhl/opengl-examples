/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Tracks multiple positions and orientations using VRPN and
 * draws a small model at each of the locations.
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

static int global_argc = 0;
static char **global_argv;

static kuhl_geometry *modelgeom  = NULL;

static kuhl_geometry quad;
static GLuint *label = NULL;
static float *labelAspectRatio = NULL;

/** Initial position of the camera. 1.55 is a good approximate
 * eyeheight in meters.*/
static const float initCamPos[3]  = {0,1.55,5};

/** A point that the camera should initially be looking at. If
 * fitToView is set, this will also be the position that model will be
 * translated to. */
static const float initCamLook[3] = {0,0,-5};

/** A vector indicating which direction is up. */
static const float initCamUp[3]   = {0,1,0};


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

/** Draw a object at the location and orientation of the tracked vrpn object */
void drawObject(const int objectIndex, float viewMat[16])
{
	const char *vrpnObject = global_argv[objectIndex];
	const float scaleFactor = .5;
	
	float pos[4], orient[16];
	vrpn_get(vrpnObject, NULL, pos, orient);
	float modelMat[16],translate[16],scale[16];
	mat4f_scale_new(scale, scaleFactor, scaleFactor, scaleFactor);
	mat4f_translateVec_new(translate, pos);
	mat4f_mult_mat4f_many(modelMat, translate, orient, scale, NULL);

	float modelview[16];
	mat4f_mult_mat4f_new(modelview, viewMat, modelMat); // modelview = view * model

	/* Send the modelview matrix to the vertex program. */
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
	                   1, // number of 4x4 float matrices
	                   0, // transpose
	                   modelview); // value

	glUniform1i(kuhl_get_uniform("renderStyle"), 2);

	kuhl_errorcheck();
	kuhl_geometry_draw(modelgeom); /* Draw the model */
	kuhl_errorcheck();

	/* Transparency of labels may not appear right because we aren't
	 * sorting them by depth. */
	float labelScale[16];
	mat4f_scale_new(labelScale, 1, 1/labelAspectRatio[objectIndex-1], 1);
	mat4f_mult_mat4f_new(modelview, modelview, labelScale);
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, 0, modelview);
	glUniform1i(kuhl_get_uniform("renderStyle"), 1);
	kuhl_geometry_texture(&quad, label[objectIndex-1], "tex", 1);
	kuhl_geometry_draw(&quad);

#if 0
	printf("%s is at\n", vrpnObject);
	vec3f_print(pos);
	mat4f_print(orient);
#endif
}


void init_geometryQuad(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog,
	                  4, // number of vertices
	                  GL_TRIANGLES); // type of thing to draw

	/* The data that we want to draw */
	GLfloat vertexPositions[] = {0, 0, 0,
	                             1, 0, 0,
	                             1, 1, 0,
	                             0, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions,
	                     3, // number of components x,y,z
	                     "in_Position", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?

	GLfloat texcoord[] = {0, 0,
	                      1, 0,
	                      1, 1,
	                      0, 1};
	kuhl_geometry_attrib(geom, texcoord,
	                     2, // number of components x,y,z
	                     "in_TexCoord", // GLSL variable
	                     KG_WARN); // warn if attribute is missing in GLSL program?


	

	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
	                       0, 2, 3 }; // indices of second triangle.
	kuhl_geometry_indices(geom, indexData, 6);

	kuhl_errorcheck();
}


/** Draws the 3D scene. */
void display()
{
	static int frameCount = 0;
	frameCount++;
	if(frameCount > 60)
	{
		frameCount = 0;
		msg(MSG_INFO, "FPS: %.1f\n", bufferswap_fps());
	}
	
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



		for(int i=1; i<global_argc; i++)
			drawObject(i, viewMat);

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();
	
	/* Update the model for the next frame based on the time. We
	 * convert the time to seconds and then use mod to cause the
	 * animation to repeat. */
	double time = glfwGetTime();
	dgr_setget("time", &time, sizeof(double));
	kuhl_update_model(modelgeom, 0, fmod(time, 10));

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

	if(argc == 1)
	{
		msg(MSG_FATAL, "You didn't provide the name of the object(s) that you want to track.");
		msg(MSG_FATAL, "Usage: %s vrpnObjectName1 vrpnObjectName2 ...", argv[0]);
		exit(EXIT_FAILURE);
	}
	global_argc = argc;
	global_argv = argv;


	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);

	dgr_init();     /* Initialize DGR based on environment variables. */

	viewmat_init(initCamPos, initCamLook, initCamUp);

	// Clear the screen while things might be loading
	glClearColor(.2,.2,.2,1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the model from the file
	//modelgeom = kuhl_load_model("../models/cube/cube.obj", NULL, program, NULL);
	modelgeom = kuhl_load_model("../models/origin/origin.obj", NULL, program, NULL);

	init_geometryQuad(&quad, program);
	label            = malloc(sizeof(GLuint)*argc-1);
	labelAspectRatio = malloc(sizeof(float)*argc-1);
	float labelColor[3] = { 1,1,1 };
	float labelBg[4] = { 0,0,0,.3 };
	for(int i=1; i<argc; i++)
	{
		labelAspectRatio[i-1] = kuhl_make_label(argv[i],
		                                        &(label[i-1]),
		                                        labelColor, labelBg, 24);
	}
	

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
