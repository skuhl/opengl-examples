/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file This program demonstrates how to access individual vertices
 * inside of a model.
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

static int renderStyle = 2;

static kuhl_geometry *modelgeom = NULL;
static float bbox[6];

/** Set this variable to 1 to force this program to scale the entire
 * model and translate it so that we can see the entire model. This is
 * a useful setting to use when you are loading a new model that you
 * are unsure about the units and position of the model geometry. */
#define FIT_TO_VIEW 1
/** If FIT_TO_VIEW is set, this is the place to put the
 * center of the bottom face of the bounding box. If
 * FIT_TO_VIEW is not set, this is the location in world
 * coordinates that we want to model's origin to appear at. */
static float placeToPutModel[3] = { 0, 0, 0 };

typedef struct {
	GLfloat velocity[3];
} particle;

static particle **particles;

#define GLSL_VERT_FILE "assimp.vert"
#define GLSL_FRAG_FILE "assimp.frag"

/** Give each vertex a velocity when the explosion occurs. */
void explode()
{
	kuhl_geometry *g = modelgeom;
	for(unsigned int i=0; i<kuhl_geometry_count(modelgeom); i++)
	{
		/* Get the normal information from each of the vertices. */
		GLint numFloats = 0;
		GLfloat *norm = kuhl_geometry_attrib_get(g, "in_Normal",
		                                         &numFloats);
		
		/* Calculate the velocity of each vertex when the explosion occurs */
		for(unsigned int j=0; j<g->vertex_count; j++)
		{
			// Start by setting the velocity equal to the normal to
			// make the particles move out.
			vec3f_copy(particles[i][j].velocity, &norm[j*3]);

			// Scale the initial velocity
			vec3f_scalarMult(particles[i][j].velocity, 10);

			// Instead of moving the particles only in the direction
			// of the normal, make them move 'up' (in object
			// coordinates) too.
			particles[i][j].velocity[1] += .5;

			// Add a bit of randomness
			for(int k=0; k<3; k++)
				particles[i][j].velocity[k] += (drand48()-.5);
		}
		g = g->next;
	}
}

/** Update the vertex positions and the velocity stored in the
 * particles array. */
void update()
{
	kuhl_geometry *g = modelgeom;
	for(unsigned int i=0; i<kuhl_geometry_count(modelgeom); i++)
	{
		int numFloats = 0;
		GLfloat *pos = kuhl_geometry_attrib_get(g, "in_Position",
		                                        &numFloats);

		for(unsigned int j=0; j<g->vertex_count; j++)
		{
			/* If the first point isn't moving, don't update anything */
			if(vec3f_norm(particles[i][j].velocity) == 0)
				return;

			/* Gravity is pushing particles down -Y, but we are
			 * operating in object coordinates. If GeomTransform
			 * (i.e., g->matrix) is used to rotate the model, then
			 * gravity might not push the particles down in world
			 * coordinates. */
			float accel[3] = { 0, -1, 0};
			float timestep = 0.1f; // change this to change speed of explosion
			for(int k=0; k<3; k++)
			{
				pos[j*3+k] += timestep * (particles[i][j].velocity[k] + timestep * accel[k]/2);
				particles[i][j].velocity[k] += timestep * accel[k];
			}
#if 1   /* Bounce the particles off the xz-plane. */
			if(pos[j*3+1] < 0)
			{
				/* How much velocity is lost when a bounce occurs? */
				float velocityLossFactor = .4;
				/* If particle fell through floor, negate its position */
				pos[j*3+1] *= -velocityLossFactor;
				/* Negative the Y velocity */
				particles[i][j].velocity[1] *= -1;
				/* Scale velocity in all directions */
				vec3f_scalarMult(particles[i][j].velocity, velocityLossFactor); // lose energy on bounce
			}
#endif
		}
		g = g->next;
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
		case GLFW_KEY_KP_ADD: // increase size of points and width of lines
		{
			GLfloat currentPtSize;
			GLfloat sizeRange[2];
			glGetFloatv(GL_POINT_SIZE, &currentPtSize);
			glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, sizeRange);
			GLfloat temp = currentPtSize+1;
			if(temp > sizeRange[1])
				temp = sizeRange[1];
			glPointSize(temp);
			printf("Point size is %f (can be between %f and %f)\n", temp, sizeRange[0], sizeRange[1]);
			kuhl_errorcheck();

			GLfloat currentLineWidth;
			GLfloat widthRange[2];
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
			GLfloat sizeRange[2];
			glGetFloatv(GL_POINT_SIZE, &currentPtSize);
			glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, sizeRange);
			GLfloat temp = currentPtSize-1;
			if(temp < sizeRange[0])
				temp = sizeRange[0];
			glPointSize(temp);
			printf("Point size is %f (can be between %f and %f)\n", temp, sizeRange[0], sizeRange[1]);
			kuhl_errorcheck();

			GLfloat currentLineWidth;
			GLfloat widthRange[2];
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
		case GLFW_KEY_X:
			explode();
			break;
		case GLFW_KEY_Z:
			update();
			break;
		case GLFW_KEY_SPACE: // Toggle different sections of the GLSL fragment shader
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
	if(FIT_TO_VIEW == 0)
	{
		/* Translate the model to where we were asked to put it */
		float translate[16];
		mat4f_translateVec_new(translate, placeToPutModel);

		/* Do inches to meters conversion if we are asked to. */
		float scale[16];
		mat4f_identity(scale);
		mat4f_mult_mat4f_new(result, translate, scale);
		return;
	}
	
	/* Get a matrix to scale+translate the model based on the bounding
	 * box. If the last parameter is 1, the bounding box will sit on
	 * the XZ plane. If it is set to 0, the bounding box will be
	 * centered at the specified point. */
	float fitMatrix[16];
	kuhl_bbox_fit(fitMatrix, bbox, 1);

	/* Get a matrix that moves the model to the correct location. */
	float moveToLookPoint[16];
	mat4f_translateVec_new(moveToLookPoint, placeToPutModel);

	/* Create a single model matrix. */
	mat4f_mult_mat4f_new(result, moveToLookPoint, fitMatrix);
}


/** Draws the 3D scene. */
void display()
{
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

		kuhl_limitfps(60);
		update();
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
		exit(EXIT_FAILURE);
	}

	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,1.55,2}; // 1.55m is a good approx eyeheight
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);

	// Clear the screen while things might be loading
	glClearColor(.2,.2,.2,1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the model from the file
	modelgeom = kuhl_load_model(modelFilename, modelTexturePath, program, bbox);
	if(modelgeom == NULL)
	{
		msg(MSG_FATAL, "Unable to load the requested model: %s", modelFilename);
		exit(EXIT_FAILURE);
	}

	/* Count the number of kuhl_geometry objects for this model */
	unsigned int geomCount = kuhl_geometry_count(modelgeom);
	
	/* Allocate an array of particle arrays */
	particles = malloc(sizeof(particle*)*geomCount);
	int i = 0;
	for(kuhl_geometry *g = modelgeom; g != NULL; g=g->next)
	{
		/* allocate space to store velocity information for all of the
		 * vertices in this kuhl_geometry */
		particles[i] = malloc(sizeof(particle)*g->vertex_count);
		for(unsigned int j=0; j<g->vertex_count; j++)
			vec3f_set(particles[i][j].velocity, 0,0,0);

		/* Change the geometry to be drawn as points */
		g->primitive_type = GL_POINTS; // Comment out this line to default to triangle rendering.
		i++;
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
