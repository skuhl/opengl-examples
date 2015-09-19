/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Demonstrates inverse kinematics using a Jacobian transpose approach.
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
int renderStyle = 0;

GLuint program = 0; // id value for the GLSL program
kuhl_geometry *modelgeom = NULL;
float bbox[6];

/** Set this variable to 1 to force this program to scale the entire
 * model and translate it so that we can see the entire model. This is
 * a useful setting to use when you are loading a new model that you
 * are unsure about the units and position of the model geometry. */
#define FIT_TO_VIEW 0
/** If FIT_TO_VIEW is set, this is the place to put the
 * center of the bottom face of the bounding box. If
 * FIT_TO_VIEW is not set, this is the location in world
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

float angles[] = { 10, 15, 20,  // arm 1
                   20, 25, 30,  // arm 2
};
int anglesCount = 6;
float target[4] = { 0, 4, 0, 1};

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
		case 'a': target[0]+=.05; break;
		case 'A': target[0]-=.05; break;
		case 's': target[1]+=.05; break;
		case 'S': target[1]-=.05; break;
		case 'd': target[2]+=.05; break;
		case 'D': target[2]-=.05; break;
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


/* Get arm matrices given a set of angles. The arm2 matrix already has
 * the arm1 matrix applied to it. */
void get_arm_matrices(float arm1[16], float arm2[16], float angles[])
{
	list *stack = list_new(16, sizeof(float)*16, NULL);

	float baseRotate[16];
	mat4f_rotateEuler_new(baseRotate, angles[0], angles[1], angles[2], "XYZ");
	mat4f_stack_mult(stack, baseRotate);
	mat4f_stack_push(stack);

	float scale[16];
	mat4f_scale_new(scale, .5, 4, .5);
	float decenter[16];
	mat4f_translate_new(decenter, 0, .5, 0);

	mat4f_stack_mult(stack, scale);
	mat4f_stack_mult(stack, decenter);
	mat4f_stack_peek(stack, arm1);
	mat4f_stack_pop(stack);

	float trans[16];
	mat4f_translate_new(trans, 0, 4, 0);
	mat4f_stack_mult(stack, trans);

	mat4f_rotateEuler_new(baseRotate, angles[3], angles[4], angles[5], "XYZ");
	mat4f_stack_mult(stack, baseRotate);
	mat4f_stack_push(stack);

	mat4f_stack_mult(stack, scale);
	mat4f_stack_mult(stack, decenter);
	mat4f_stack_peek(stack, arm2);
	mat4f_stack_pop(stack);

	list_free(stack);
}

/* Given a list of angles, calculate end effector location */
void end_effector_loc(float loc[4], float angles[])
{
	float arm1mat[16], arm2mat[16];
	get_arm_matrices(arm1mat, arm2mat, angles);
	vec4f_set(loc, 0, .5, 0, 1);
	mat4f_mult_vec4f_new(loc, arm2mat, loc);
}

/* Get a jacobian matrix. It is 3 elements tall and angleCount
 * elements wide. Each column represents how (x,y,z) of the end
 * effector will change given a small change in the angle. */
float* get_jacobian(float delta)
{
	float *jacobian = malloc(sizeof(float)*3*anglesCount);

	float origLoc[3];
	end_effector_loc(origLoc, angles);
	
	for(int i=0; i<anglesCount; i++)
	{
		angles[i] += delta;
		float newLoc[3];
		end_effector_loc(newLoc, angles);
		float deltaLoc[3];
		vec3f_sub_new(deltaLoc, newLoc, origLoc);
		for(int j=0; j<3; j++)
			jacobian[i*3+j] = deltaLoc[j];
		angles[i] -= delta;
	}


	printf("jacobian:\n");
	for(int i=0; i<anglesCount; i++)
	{
		for(int j=0; j<3; j++)
			printf("%8.4f ", jacobian[j*anglesCount+i]);
		printf("\n");
	}
	
	return jacobian;
}


void effector_target(float target[4])
{
	while(1)
	{
		/* Get current location of end effector */
		float currentLoc[4]; 
		end_effector_loc(currentLoc, angles);
		/* Get a vector pointing to target from current end effector location */
		float deltaTarget[3];
		vec3f_sub_new(deltaTarget, target, currentLoc);
		float distance = vec3f_norm(deltaTarget);

		if(distance < .001)
			break;

		printf("pre: location, target, delta:\n");
		vec3f_print(currentLoc);
		vec3f_print(target);
		vec3f_print(deltaTarget);
		printf("distance: %f\n", distance);
		printf("angles:\n");
		vecNf_print(angles, anglesCount);

		float *jacobian = get_jacobian(2);

		/* Jacobian is in column-major order. Each column represents
		 * the change in (x,y,z) given a change in a specific angle:

		   angle0  angle1  ...
		   -----------------
		   x       x
		   y       y
		   z       z

		   We can "transpose" the jacobian simply by changing the way
		   we index into the array. To transpose it, we simply assume
		   that the array is in row-major order---meaning that a row
		   represents a change in (x,y,z) given a change in a specific
		   angle.

		   Transposed jacobian is (we can assume it is in row-major order to transpose)

		   x y z
		   x y z
		   ...

		*/


		/* Multiply the change in the end effector by the transposed
		 * Jacobian. The result will approximate the change in angle
		 * of each of the joints that we want. */
		float *changeInAngle = malloc(sizeof(float)*anglesCount);
		for(int i=0; i<anglesCount; i++) // for each row in the transposed Jacobian
		{
			// take the dot product of the transposed jacobian row with the target position:
			changeInAngle[i] = vec3f_dot(&(jacobian[i*3]), deltaTarget);
		}

		/* Calculate how these changes in angle would influence end
		   effector based on the original Jacobian */
		float expectedChangeInEffector[3] = { 0,0,0 };
		for(int i=0; i<3; i++)
		{
			// Dot product of the first row of the jacobian with the changeInAngle
			for(int j=0; j<anglesCount; j++)
				expectedChangeInEffector[i] += changeInAngle[j] * jacobian[j*3+i];
		}
		printf("expected change in effector:\n");
		vec3f_print(expectedChangeInEffector);

		/* Calculate a reasonable alpha according to:
		   http://www.math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/iksurvey.pdf
		*/
		float alpha = vec3f_dot(expectedChangeInEffector, deltaTarget) /
			vec3f_dot(expectedChangeInEffector, expectedChangeInEffector);
		printf("alpha: %f\n", alpha);

		/* Apply our angle changes (multiplied by alpha) to the robots angles */
		printf("Change in angles: ");
		for(int i=0; i<anglesCount; i++)
		{
			angles[i] += alpha*changeInAngle[i];
			angles[i] = fmod(angles[i], 360); // Keep angles within 0 to 360
			printf("%f ", changeInAngle[i]);
		}
		printf("\n");

		float newLoc[3];
		end_effector_loc(newLoc, angles);
		float actualChange[3];
		vec3f_sub_new(actualChange, newLoc, currentLoc);
		printf("Actual change in end effector\n");
		vec3f_print(actualChange);
		
		free(changeInAngle);
		free(jacobian);

		// Uncomment to see IK solution change
		// break;
	}

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

		glUniform1i(kuhl_get_uniform("renderStyle"), renderStyle);
		// Copy far plane value into vertex program so we can render depth buffer.
		float f[6]; // left, right, bottom, top, near>0, far>0
		projmat_get_frustum(f, viewport[2], viewport[3]);
		glUniform1f(kuhl_get_uniform("farPlane"), f[5]);
		
		kuhl_errorcheck();
		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   perspective); // value

//		float modelMat[16];
//		get_model_matrix(modelMat);
//		mat4f_stack_mult(stack, modelMat);



		effector_target(target);
		
		float arm1Mat[16],arm2Mat[16];
		get_arm_matrices(arm1Mat, arm2Mat, angles);
		
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, arm1Mat);
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();
		kuhl_geometry_draw(modelgeom); /* Draw the model */
		kuhl_errorcheck();

		mat4f_mult_mat4f_new(modelview, viewMat, arm2Mat);
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
		                   1, // number of 4x4 float matrices
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();
		kuhl_geometry_draw(modelgeom); /* Draw the model */
		kuhl_errorcheck();

		float ealoc[4];
		end_effector_loc(ealoc, arm2Mat);

		
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
	char *modelFilename    = "../models/cube/cube.obj";
	char *modelTexturePath = NULL;
	
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
