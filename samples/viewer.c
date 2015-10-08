/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Loads 3D model files displays them.
 *
 * @author Scott Kuhl
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#ifdef FREEGLUT
#include <GL/freeglut.h>
#else
#include <GLUT/glut.h>
#endif

#include "kuhl-util.h"
#include "vecmat.h"
#include "dgr.h"
#include "projmat.h"
#include "viewmat.h"

GLuint fpsLabel = 0;
float fpsLabelAspectRatio = 0;
kuhl_geometry labelQuad;
int renderStyle = 2;

GLuint program = 0; // id value for the GLSL program
kuhl_geometry *modelgeom = NULL;
kuhl_geometry *origingeom = NULL;
float bbox[6];

int fitToView=0;  // was --fit option used?

/** The following variable toggles the display an "origin+axis" marker
 * which draws a small box at the origin and draws lines of length 1
 * on each axis. Depending on which matrices are applied to the
 * marker, the marker will be in object, world, etc coordinates. */
int showOrigin=0; // was --origin option used?


/** If fitToView is set, this is the place to put the
 * center of the bottom face of the bounding box. If
 * fitToView is not set, this is the location in world
 * coordinates that we want to model's origin to appear at. */
float placeToPutModel[3] = { 0, 0, 0 };
/** SketchUp produces files that older versions of ASSIMP think 1 unit
 * is 1 inch. However, all of this software assumes that 1 unit is 1
 * meter. So, we need to convert some models from inches to
 * meters. Newer versions of ASSIMP correctly read the same files and
 * give us units in meters. */
#define INCHES_TO_METERS 0

#define GLSL_VERT_FILE "assimp.vert"
#define GLSL_FRAG_FILE "assimp.frag"

/* Called by GLUT whenever a key is pressed. */
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'q':
		case 'Q':
		case 27: // ASCII code for Escape key
			dgr_exit();
			exit(EXIT_SUCCESS);
			break;
		case 'f': // full screen
			glutFullScreen();
			break;
		case 'F': // switch to window from full screen mode
			glutPositionWindow(0,0);
			break;
		case 'r':
		{
			// Reload GLSL program from disk
			kuhl_delete_program(program);
			program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);
			/* Apply the program to the model geometry */
			kuhl_geometry_program(modelgeom, program, KG_FULL_LIST);
			/* and the fps label*/
			kuhl_geometry_program(&labelQuad, program, KG_FULL_LIST);

			break;
		}
		case 'w':
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
		case 'p':
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
		case 'c':
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
		case '+': // increase size of points and width of lines
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
		case '-': // decrease size of points and width of lines
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
		
		case ' ': // Toggle different sections of the GLSL fragment shader
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

	/* Whenever any key is pressed, request that display() get
	 * called. */ 
	glutPostRedisplay();
}


void get_model_matrix(float result[16])
{
	mat4f_identity(result);
	if(fitToView == 0)
	{
		/* Translate the model to where we were asked to put it */
		float translate[16];
		mat4f_translateVec_new(translate, placeToPutModel);

		/* Do inches to meters conversion if we are asked to. */
		float scale[16];
		mat4f_identity(scale);
		if(INCHES_TO_METERS)
		{
			float inchesToMeters=1/39.3701;
			mat4f_scale_new(scale, inchesToMeters, inchesToMeters, inchesToMeters);
		}
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


/* Called by GLUT whenever the window needs to be redrawn. This
 * function should not be called directly by the programmer. Instead,
 * we can call glutPostRedisplay() to request that GLUT call display()
 * at some point. */
static kuhl_fps_state fps_state;
void display()
{
	/* If we are using DGR, send or receive data to keep multiple
	 * processes/computers synchronized. */
	dgr_update();

	/* Get current frames per second calculations. */
	float fps = kuhl_getfps(&fps_state);

	if(dgr_is_enabled() == 0 || dgr_is_master())
	{
		// If DGR is being used, only display dgr counter if we are
		// the master process.

		// Check if FPS value was just updated by kuhl_getfps()
		if(fps_state.frame == 0)
		{
			char label[1024];
			snprintf(label, 1024, "FPS: %0.1f", fps);

			/* Delete old label if it exists */
			if(fpsLabel != 0) 
				glDeleteTextures(1, &fpsLabel);

			/* Make a new label */
			float labelColor[3] = { 1,1,1 };
			float labelBg[4] = { 0,0,0,.3 };
			/* Change the last parameter (point size) to adjust the
			 * size of the texture that the text is rendered in to. */
			fpsLabelAspectRatio = kuhl_make_label(label,
			                                      &fpsLabel,
			                                      labelColor, labelBg, 24);

			if(fpsLabel != 0)
				kuhl_geometry_texture(&labelQuad, fpsLabel, "tex", 1);
		}
	}
	
	/* Ensure the slaves use the same render style as the master
	 * process. */
	dgr_setget("style", &renderStyle, sizeof(int));

	
	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right. */
	viewmat_begin_frame();
	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		viewmat_begin_eye(viewportID);

		/* Where is the viewport that we are drawing onto and what is its size? */
		int viewport[4]; // x,y of lower left corner, width, height
		viewmat_get_viewport(viewport, viewportID);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Clear the current viewport. Without glScissor(), glClear()
		 * clears the entire screen. We could call glClear() before
		 * this viewport loop---but on order for all variations of
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
		kuhl_geometry_draw(modelgeom); /* Draw the model */
		kuhl_errorcheck();
		if(showOrigin)
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


		if(dgr_is_enabled() == 0 || dgr_is_master())
		{

			/* The shape of the frames per second quad depends on the
			 * aspect ratio of the label texture and the aspect ratio of
			 * the window (because we are placing the quad in normalized
			 * device coordinates). */
			int windowWidth, windowHeight;
			viewmat_window_size(&windowWidth, &windowHeight);
			float windowAspect = windowWidth / (float)windowHeight;
			
			float stretchLabel[16];
			mat4f_scale_new(stretchLabel, 1/8.0 * fpsLabelAspectRatio / windowAspect, 1/8.0, 1);

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
			kuhl_geometry_draw(&labelQuad); /* Draw the quad */
			glEnable(GL_DEPTH_TEST);
			kuhl_errorcheck();
		}

		glUseProgram(0); // stop using a GLSL program.

	} // finish viewport loop
	viewmat_end_frame();
	
	/* Update the model for the next frame based on the time. We
	 * convert the time to seconds and then use mod to cause the
	 * animation to repeat. */
	int time = glutGet(GLUT_ELAPSED_TIME);
	dgr_setget("time", &time, sizeof(int));
	kuhl_update_model(modelgeom, 0, ((time%10000)/1000.0));

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	//kuhl_video_record("videoout", 30);
	
	/* Ask GLUT to call display() again. We shouldn't call display()
	 * ourselves recursively because it will not leave time for GLUT
	 * to call other callback functions for when a key is pressed, the
	 * window is resized, etc. */
	glutPostRedisplay();
}

/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
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


int main(int argc, char** argv)
{
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

	/* set up our GLUT window */
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);

	/* Ask GLUT to for a double buffered, full color window that
	 * includes a depth buffer */
#ifdef FREEGLUT
	glutSetOption(GLUT_MULTISAMPLE, 4); // set msaa samples; default to 4
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitContextVersion(3,2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
#endif
	glutCreateWindow(argv[0]); // set window title to executable name
	glEnable(GL_MULTISAMPLE);

	/* Initialize GLEW */
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if(glewError != GLEW_OK)
	{
		fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(glewError));
		exit(EXIT_FAILURE);
	}
	/* When experimental features are turned on in GLEW, the first
	 * call to glGetError() or kuhl_errorcheck() may incorrectly
	 * report an error. So, we call glGetError() to ensure that a
	 * later call to glGetError() will only see correct errors. For
	 * details, see:
	 * http://www.opengl.org/wiki/OpenGL_Loading_Library */
	glGetError();

	// setup callbacks
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);

	dgr_init();     /* Initialize DGR based on environment variables. */
	projmat_init(); /* Figure out which projection matrix we should use based on environment variables */

	float initCamPos[3]  = {0,1.55,2}; // 1.55m is a good approx eyeheight
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);

	// Clear the screen while things might be loading
	glClearColor(.2,.2,.2,1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the model from the file
	modelgeom = kuhl_load_model(modelFilename, modelTexturePath, program, bbox);
	if(showOrigin)
		origingeom = kuhl_load_model("../models/origin/origin.obj", NULL, program, NULL);
	
	init_geometryQuad(&labelQuad, program);

	kuhl_getfps_init(&fps_state);
	
	/* Tell GLUT to start running the main loop and to call display(),
	 * keyboard(), etc callback methods as needed. */
	glutMainLoop();
	/* // An alternative approach:
	   while(1)
	   glutMainLoopEvent();
	*/

	exit(EXIT_SUCCESS);
}
