/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file This program demonstrates different ways to interpolate
 * between two orientations.
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
static kuhl_geometry *modelgeom = NULL;
static float bbox[6];

static int fitToView=1;  // was --fit option used?


/** Initial position of the camera. 1.55 is a good approximate
 * eyeheight in meters.*/
static const float initCamPos[3]  = {0,1.55,0};

/** A point that the camera should initially be looking at. If
 * fitToView is set, this will also be the position that model will be
 * translated to. */
static const float initCamLook[3] = {0,0,-5};

/** A vector indicating which direction is up. */
static const float initCamUp[3]   = {0,1,0};

static int rotateStyle = 0;

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
		case GLFW_KEY_R:
		{
			// Reload GLSL program from disk
			kuhl_delete_program(program);
			program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);
			/* Apply the program to the model geometry */
			kuhl_geometry_program(modelgeom, program, KG_FULL_LIST);

			break;
		}
				
		case GLFW_KEY_SPACE:
			rotateStyle++;
			if(rotateStyle > 3)
				rotateStyle = 0;
			switch(rotateStyle)
			{
				case 0: printf("Interpolate Euler angles\n"); break;
				case 1: printf("Interpolate rotation matrices\n"); break;
				case 2: printf("Interpolate quaternions\n"); break;
				case 3: printf("Interpolate quaternion (slerp)\n"); break;
			}
			break;
	}
}


/** Gets a model matrix which is appropriate for the model that we have loaded. */
void get_model_matrix(float result[16])
{
	mat4f_identity(result);

	if(fitToView == 1) // if --fit option was provided.
	{
		float fitMat[16];
		float transMat[16];
		
		/* Get a matrix to scale+translate the model based on the bounding
		 * box. If the last parameter is 1, the bounding box will sit on
		 * the XZ plane. If it is set to 0, the bounding box will be
		 * centered at the specified point. */
		kuhl_bbox_fit(fitMat, bbox, 1);

		/* Translate the model to the point the camera is looking at. */
		mat4f_translateVec_new(transMat, initCamLook);

		mat4f_mult_mat4f_new(result, transMat, fitMat);
	}
	else  // if NOT fitting to view.
	{
		msg(MSG_ERROR, "Fit to view must be used for this program to work.");
		return;
	}
	float rotateAnimate[16];
	mat4f_identity(rotateAnimate);
	float percentComplete = fmod(glfwGetTime(),4)/4.0;
	
	float startEuler[3] = { 0, 0, 0 };
	float endEuler[3] = { 0, -150, 0 };
	float startMatrix[16], endMatrix[16];
	mat4f_rotateEuler_new(startMatrix, startEuler[0], startEuler[1], startEuler[2], "XYZ");
	mat4f_rotateEuler_new(endMatrix, endEuler[0], endEuler[1], endEuler[2], "XYZ");

	if(rotateStyle == 0) // Interpolate eulers.
	{
		// When we interpolate Euler angles, the object might appear
		// to "wobble" some between orientations.
		float interpolate[3] = { 0,0,0 };
		vec3f_scalarMult(startEuler, 1-percentComplete);
		vec3f_scalarMult(endEuler, percentComplete);
		vec3f_add_new(interpolate, startEuler, endEuler);
		mat4f_rotateEuler_new(rotateAnimate, interpolate[0], interpolate[1], interpolate[2], "XYZ");
	}
	else if(rotateStyle == 1) // Interpolate matrices
	{
		// When we interpolate matrices, the scaling of the object may
		// change in unexpected ways between orientations.
		for(int i=0; i<16; i++)
		{
			rotateAnimate[i] = startMatrix[i] * (1-percentComplete) +
				endMatrix[i] * percentComplete;
		}
	}
	else if(rotateStyle == 2) // interpolate quaternions - linear
	{
		// When we interpolate quaternions, the rotation looks
		// good---but it may speed up or slow down slightly during the
		// rotation.
		float startQuat[4], endQuat[4];
		float interpQuat[4];
		quatf_from_mat4f(startQuat, startMatrix);
		quatf_from_mat4f(endQuat, endMatrix);
		
		/* If the rotation would be more than 180 degrees, rotate the
		 * other way instead. */
		float dotProd = 0;
		for(int i=0; i<4; i++)
			dotProd += startQuat[i]*endQuat[i];
		if(dotProd < 0)
			vec4f_scalarMult(endQuat, -1);
		
		/* Do the linear interpolation */
		for(int i=0; i<4; i++)
		{
			interpQuat[i] = startQuat[i]*(1-percentComplete) +
			                endQuat[i]  *percentComplete;
		}
		
		quatf_normalize(interpQuat);
		mat4f_rotateQuatVec_new(rotateAnimate, interpQuat);
	}
	else if(rotateStyle == 3) // quaternion slerp
	{
		// The best way to interpolate rotations is to use spherical
		// linear interpolation of quaternions. This will look almost
		// the same as linearly interpolating quaternions.
		float startQuat[4], endQuat[4];
		float interpQuat[4];
		quatf_from_mat4f(startQuat, startMatrix);
		quatf_from_mat4f(endQuat, endMatrix);
		quatf_slerp_new(interpQuat, startQuat, endQuat, percentComplete);
		mat4f_rotateQuatVec_new(rotateAnimate, interpQuat);
	}
	
	float fitMat[16];
	float transMat[16];
	
	/* Get a matrix to scale+translate the model based on the bounding
	 * box. If the last parameter is 1, the bounding box will sit on
	 * the XZ plane. If it is set to 0, the bounding box will be
	 * centered at the specified point. */
	kuhl_bbox_fit(fitMat, bbox, 1);

	/* Translate the model to the point the camera is looking at. */
	mat4f_translateVec_new(transMat, initCamLook);

	/* Create a single model matrix. */
	mat4f_mult_mat4f_new(result, rotateAnimate, fitMat);
	mat4f_mult_mat4f_new(result, transMat, result);
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

		float modelMat[16];
		get_model_matrix(modelMat);
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, modelMat); // modelview = view * model

		/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value

		glUniform1i(kuhl_get_uniform("renderStyle"), 0);

		kuhl_errorcheck();
		kuhl_geometry_draw(modelgeom); /* Draw the model */
		kuhl_errorcheck();

		glUseProgram(0); // stop using a GLSL program.
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	//kuhl_video_record("videoout", 30);
}

int main(int argc, char** argv)
{
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);
	
	
	char *modelFilename    = NULL;
	char *modelTexturePath = NULL;
	
	if(argc == 2)
	{
		modelFilename = argv[1];
		modelTexturePath = NULL;
	}
	else if(argc == 3)
	{
		modelFilename = argv[1];
		modelTexturePath = argv[2];
	}
	else
	{
		printf("Usage:\n"
		       "%s modelFile     - Textures are assumed to be in the same directory as the model.\n"
		       "- or -\n"
		       "%s modelFile texturePath\n", argv[0], argv[0]);
		exit(1);
	}


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
	modelgeom = kuhl_load_model(modelFilename, modelTexturePath, program, bbox);
	if (modelgeom == NULL)
	{
		msg(MSG_FATAL, "Unable to load the requested model: %s", modelFilename);
		exit(EXIT_FAILURE);
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
