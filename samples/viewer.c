/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Loads 3D model files displays them.
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

static int renderStyle = 2;

static kuhl_geometry *fpsgeom = NULL;
static kuhl_geometry *modelgeom  = NULL;
static kuhl_geometry *origingeom = NULL;
static float bbox[6];

static int fitToView=0;  // was --fit option used?

/** The following variable toggles the display an "origin+axis" marker
 * which draws a small box at the origin and draws lines of length 1
 * on each axis. Depending on which matrices are applied to the
 * marker, the marker will be in object, world, etc coordinates. */
static int showOrigin=0; // was --origin option used?


/** Initial position of the camera. 1.55 is a good approximate
 * eyeheight in meters.*/
static const float initCamPos[3]  = {0.0f,1.55f,0.0f};

/** A point that the camera should initially be looking at. If
 * fitToView is set, this will also be the position that model will be
 * translated to. */
static const float initCamLook[3] = {0.0f,0.0f,-5.0f};

/** A vector indicating which direction is up. */
static const float initCamUp[3]   = {0.0f,1.0f,0.0f};


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
			/* and the fps label*/
			kuhl_geometry_program(fpsgeom, program, KG_FULL_LIST);

			break;
		}
		case GLFW_KEY_W:
		{
			// Toggle between wireframe and solid
			int polygonMode;
			glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
			if(polygonMode == GL_LINE)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		}
		case GLFW_KEY_P:
		{
			// Toggle between points and solid
			int polygonMode;
			glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
			if(polygonMode == GL_POINT)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			break;
		}
		case GLFW_KEY_C:
		{
			// Toggle front, back, and no culling
			int cullMode;
			glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
			if(glIsEnabled(GL_CULL_FACE))
			{
				if(cullMode == GL_FRONT)
				{
					glCullFace(GL_BACK);
					printf("Culling: Culling back faces; drawing front faces\n");
				}
				else
				{
					glDisable(GL_CULL_FACE);
					printf("Culling: No culling; drawing all faces.\n");
				}
			}
			else
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				printf("Culling: Culling front faces; drawing back faces\n");
			}
			kuhl_errorcheck();
			break;
		}
		case GLFW_KEY_D: // toggle depth clamping
		{
			if(glIsEnabled(GL_DEPTH_CLAMP))
			{
				printf("Depth clamping disabled\n");
				glDisable(GL_DEPTH_CLAMP); // default
				glDepthFunc(GL_LESS);      // default
			}
			else
			{
				/* With depth clamping, vertices beyond the near and
				   far planes are clamped to the near and far
				   planes. Since multiple layers of vertices will be
				   clamped to the same depth value, depth testing
				   beyond the near and far planes won't work. */
				printf("Depth clamping enabled\n");
				glEnable(GL_DEPTH_CLAMP);
				glDepthFunc(GL_LEQUAL); // makes far clamping work.
			}
			break;
		}
		case GLFW_KEY_EQUAL:  // The = and + key on most keyboards
		case GLFW_KEY_KP_ADD: // increase size of points and width of lines
		{
			// How can we distinguish between '=' and '+'? The 'mods'
			// variable should contain GLFW_MOD_SHIFT if the shift key
			// is pressed along with the '=' key. However, we accept
			// both versions.
			
			GLfloat currentPtSize;
			GLfloat sizeRange[2] = { -1.0f, -1.0f };
			glGetFloatv(GL_POINT_SIZE, &currentPtSize);
			glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, sizeRange);
			GLfloat temp = currentPtSize+1;
			if(temp > sizeRange[1])
				temp = sizeRange[1];
			glPointSize(temp);
			printf("Point size is %f (can be between %f and %f)\n", temp, sizeRange[0], sizeRange[1]);
			kuhl_errorcheck();

			// The only line width guaranteed to be available is
			// 1. Larger sizes will be available if your OpenGL
			// implementation or graphics card supports it.
			GLfloat currentLineWidth;
			GLfloat widthRange[2] = { -1.0f, -1.0f };
			glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
			glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, widthRange);
			temp = currentLineWidth+1;
			if(temp > widthRange[1])
				temp = widthRange[1];
			glLineWidth(temp);
			printf("Line width is %f (can be between %f and %f)\n", temp, widthRange[0], widthRange[1]);
			kuhl_errorcheck();
			break;
		}
		case GLFW_KEY_MINUS: // decrease size of points and width of lines
		case GLFW_KEY_KP_SUBTRACT:
		{
			GLfloat currentPtSize;
			GLfloat sizeRange[2] = { -1.0f, -1.0f };
			glGetFloatv(GL_POINT_SIZE, &currentPtSize);
			glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, sizeRange);
			GLfloat temp = currentPtSize-1;
			if(temp < sizeRange[0])
				temp = sizeRange[0];
			glPointSize(temp);
			printf("Point size is %f (can be between %f and %f)\n", temp, sizeRange[0], sizeRange[1]);
			kuhl_errorcheck();

			GLfloat currentLineWidth;
			GLfloat widthRange[2] = { -1.0f, -1.0f };
			glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
			glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, widthRange);
			temp = currentLineWidth-1;
			if(temp < widthRange[0])
				temp = widthRange[0];
			glLineWidth(temp);
			printf("Line width is %f (can be between %f and %f)\n", temp, widthRange[0], widthRange[1]);
			kuhl_errorcheck();
			break;
		}
		// Toggle different sections of the GLSL fragment shader
		case GLFW_KEY_SPACE:
		case GLFW_KEY_PERIOD:
			renderStyle++;
			if(renderStyle > 9)
				renderStyle = 0;
			switch(renderStyle)
			{
				case 0: printf("Render style: Diffuse (headlamp light)\n"); break;
				case 1: printf("Render style: Texture (color is used on non-textured geometry)\n"); break;
				case 2: printf("Render style: Texture+diffuse (color is used on non-textured geometry)\n"); break;
				case 3: printf("Render style: Vertex color\n"); break;
				case 4: printf("Render style: Vertex color + diffuse (headlamp light)\n"); break;
				case 5: printf("Render style: Normals\n"); break;
				case 6: printf("Render style: Texture coordinates\n"); break;
				case 7: printf("Render style: Front (green) and back (red) faces based on winding\n"); break;
				case 8: printf("Render style: Front (green) and back (red) based on normals\n"); break;
				case 9: printf("Render style: Depth (white=far; black=close)\n"); break;
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
		return;
	}
}



/** Draws the 3D scene. */
void display()
{
	/* Display FPS if we are a DGR master OR if we are running without DGR. */
	if(dgr_is_master())
	{
		static long lasttime = 0;
		long now = kuhl_milliseconds();
		if(now - lasttime > 200) // reduce number to increase frequency of FPS label updates.
		{
			lasttime = now;
			
			float fps = bufferswap_fps(); // get current fps
			char message[1024];
			snprintf(message, 1024, "FPS: %0.2f", fps); // make a string with fps in it
			float labelColor[3] = { 1.0f,1.0f,1.0f };
			float labelBg[4] = { 0.0f,0.0f,0.0f,.3f };

			// If DGR is being used, only display FPS info if we are
			// the master process.
			fpsgeom = kuhl_label_geom(fpsgeom, program, NULL, message, labelColor, labelBg, 24);
		}
	}
	
	/* Ensure the slaves use the same render style as the master
	 * process. */
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
		glClearColor(.2f,.2f,.2f,0.0f); // set clear color to grey
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

		glUniform1i(kuhl_get_uniform("renderStyle"), renderStyle);

		kuhl_errorcheck();
		kuhl_geometry_draw(modelgeom); /* Draw the model */
		kuhl_errorcheck();
		if(showOrigin && origingeom != NULL)
		{
			/* Save current line width */
			GLfloat origLineWidth;
			glGetFloatv(GL_LINE_WIDTH, &origLineWidth);
			glLineWidth(4); // make lines thick
			
			/* Object coordinate system origin */
			kuhl_geometry_draw(origingeom); /* Draw the origin marker */

			/* World coordinate origin */
			mat4f_copy(modelview, viewMat);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			                   1, // number of 4x4 float matrices
			                   0, // transpose
			                   modelview); // value
			kuhl_geometry_draw(origingeom); /* Draw the origin marker */

			/* Restore line width */
			glLineWidth(origLineWidth);
		}

		// aspect ratio will be zero when the program starts (and FPS hasn't been computed yet)
		if(dgr_is_master())
		{
			float stretchLabel[16];
			mat4f_scale_new(stretchLabel, 1/8.0f / viewmat_window_aspect_ratio(), 1/8.0f, 1.0f);
			
			/* Position label in the upper left corner of the screen */
			float transLabel[16];
			mat4f_translate_new(transLabel, -.9f, .8f, 0.0f);
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

	/* Update the model for the next frame based on the time. We
	 * convert the time to seconds and then use mod to cause the
	 * animation to repeat. */
	double time = glfwGetTime();
	dgr_setget("time", &time, sizeof(double));
	kuhl_update_model(modelgeom, 0, fmodf((float)time,10.0f));
	
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

	int currentArgIndex = 1; // skip program name
	int usageError = 0;
	while(argc > currentArgIndex)
	{
		if(strcmp(argv[currentArgIndex], "--fit") == 0)
			fitToView = 1;
		else if(strcmp(argv[currentArgIndex], "--origin") == 0)
			showOrigin = 1;
		else if(modelFilename == NULL)
		{
			modelFilename = argv[currentArgIndex];
			modelTexturePath = NULL;
		}
		else if(modelTexturePath == NULL)
			modelTexturePath = argv[currentArgIndex];
		else
		{
			usageError = 1;
		}
		currentArgIndex++;
	}

	// If we have no model to load or if there were too many arguments.
	if(modelFilename == NULL || usageError)
	{
		printf("Usage:\n"
		       "%s [--fit] [--origin] modelFile     - Textures are assumed to be in the same directory as the model.\n"
		       "- or -\n"
		       "%s [--fit] [--origin] modelFile texturePath\n"
		       "If the optional --fit parameter is included, the model will be scaled and translated to fit within the approximate view of the camera\n"
		       "If the optional --origin parameter is included, a box will is drawn at the origin and unit-length lines are drawn down each axis.\n",
		       argv[0], argv[0]);
		exit(EXIT_FAILURE);
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
	glClearColor(.2f,.2f,.2f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the model from the file
	modelgeom = kuhl_load_model(modelFilename, modelTexturePath, program, bbox);
	origingeom = kuhl_load_model("../models/origin/origin.obj", modelTexturePath, program, NULL);



	
	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
