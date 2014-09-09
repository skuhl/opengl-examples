/* This sample is based on a sample that is included with ASSIMP. Much
 * of the logic of the program was unchanged. However, texture loading
 * and other miscellaneous changes were made.
 *
 * Changes by: Scott Kuhl
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "kuhl-util.h"
#include "dgr.h"
#include "projmat.h"
#include "viewmat.h"
GLuint program = 0; // id value for the GLSL program

/** Set this variable to 1 to force this program to scale the entire
 * model and translate it so that we can see the entire model. This is
 * a useful setting to use when you are loading a new model that you
 * are unsure about the units and position of the model geometry. */
#define FIT_TO_VIEW_AND_ROTATE 1
/** The location in 3D space that we want the center of the bounding
 * box to be (if FIT_TO_VIEW_AND_ROTATE is set) or the location that
 * we should put the origin of the model */
float placeToPutModel[3] = { 0, 0, 0 };
/** SketchUp produces files that older versions of ASSIMP think 1 unit
 * is 1 inch. However, all of this software assumes that 1 unit is 1
 * meter. So, we need to convert some models from inches to
 * meters. Newer versions of ASSIMP correctly read the same files and
 * give us units in meters. */
#define INCHES_TO_METERS 0

GLuint scene_list = 0; // display list for model
char *modelFilename = NULL;
char *modelTexturePath = NULL;
int renderStyle = 0;


/* Called by GLUT whenever a key is pressed. */
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'q':
		case 'Q':
		case 27: // ASCII code for Escape key
			exit(0);
			break;
		case ' ':
			renderStyle++;
			if(renderStyle > 6)
				renderStyle = 0;
			switch(renderStyle)
			{
				case 0: printf("Render style: Texture\n"); break;
				case 1: printf("Render style: Vertex color\n"); break;
				case 2: printf("Render style: Normals\n"); break;
				case 3: printf("Render style: Texture coordinates\n"); break;
				case 4: printf("Render style: Front (green) and black (red) faces based on winding\n"); break;
				case 5: printf("Render style: Front (green) and black (red) based on normals\n"); break;
				case 6: printf("Render style: Depth (white=far; black=close)\n"); break;
			}
			break;
	}
	glutPostRedisplay();
}


void get_model_matrix(float result[16])
{
	mat4f_identity(result);
	if(FIT_TO_VIEW_AND_ROTATE == 0)
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
	
	/* Change angle for animation. */
	int count = glutGet(GLUT_ELAPSED_TIME) % 10000; // get a counter that repeats every 10 seconds
	float angle = count / 10000.0 * 360;
	dgr_setget("angle", &angle, sizeof(GLfloat));
	// angle = 0; // Don't spin the model

	/* Calculate the width/height/depth of the bounding box and
	 * determine which one of the three is the largest. Then, scale
	 * the scene by 1/(largest value) to ensure that it fits in our
	 * view frustum. */
	float bb_min[3], bb_max[3], bb_center[3];
	kuhl_model_bounding_box(modelFilename, bb_min, bb_max, bb_center);
#define mymax(a,b) (a>b?a:b)
	float tmp;
	tmp = bb_max[0] - bb_min[0];
	tmp = mymax(bb_max[1] - bb_min[1], tmp);
	tmp = mymax(bb_max[2] - bb_min[2], tmp);
	tmp = 1.f / tmp;
#undef mymax
	float rotateAnimate[16], scaleBoundBox[16], moveToOrigin[16], moveToLookPoint[16];
	mat4f_translate_new(moveToOrigin, -bb_center[0], -bb_center[1], -bb_center[2]); // move to origin
//	printf("Scaling by factor %f\n", tmp); 
	mat4f_scale_new(scaleBoundBox, tmp, tmp, tmp); // scale model based on bounding box size
	mat4f_rotateAxis_new(rotateAnimate, angle, 0, 1, 0); // rotate the object
	mat4f_translateVec_new(moveToLookPoint, placeToPutModel);

	mat4f_mult_mat4f_new(result, moveToOrigin, result);
	mat4f_mult_mat4f_new(result, scaleBoundBox, result);
	mat4f_mult_mat4f_new(result, rotateAnimate, result);
	mat4f_mult_mat4f_new(result, moveToLookPoint, result);
}

void display()
{
	dgr_update();

	// Clear the screen to black, clear the depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // turn on depth testing

	kuhl_errorcheck();
	

	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		/* Where is the viewport that we are drawing onto and what is its size? */
		int viewport[4];
		viewmat_get_viewport(viewport, viewportID);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Get the view frustum information. */
		float f[6];
		projmat_get_frustum(f, viewport[2], viewport[3]);
	    
		/* Get the projection matrix, update view frustum if necessary. */
		float viewMat[16];
		viewmat_get(viewMat, f, viewportID);

		glUseProgram(program);
		/* Communicate matricies to OpenGL */
		float perspective[16];
		mat4f_frustum_new(perspective,f[0], f[1], f[2], f[3], f[4], f[5]);
		glUniformMatrix4fv(kuhl_get_uniform(program, "Projection"),
		                   1, // count
		                   0, // transpose
		                   perspective); // value
		float modelMat[16];
		get_model_matrix(modelMat);
		
		// modelview = view * model
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, modelMat);

		glUniformMatrix4fv(kuhl_get_uniform(program, "ModelView"),
		                   1, // count
		                   0, // transpose
		                   modelview); // value
		// Normal matrix = transpose(inverse(modelview))
		float normalMat[16];
		mat4f_copy(normalMat, modelview);
		mat4f_invert(normalMat);
		mat4f_transpose(normalMat);
		glUniformMatrix4fv(kuhl_get_uniform(program, "NormalMat"),
		                   1, // count
		                   0, // transpose
		                   modelview); // value

		glUniform1i(kuhl_get_uniform(program, "renderStyle"), renderStyle);
		// Copy far plane value into vertex program so we can render depth buffer.
		glUniform1f(kuhl_get_uniform(program, "farPlane"), f[5]);
		
		kuhl_errorcheck();

		kuhl_draw_model_file_ogl3(modelFilename, modelTexturePath, program);
		kuhl_errorcheck();
	}
	    

	int time = glutGet(GLUT_ELAPSED_TIME);
	float fps = kuhl_getfps(time);
	if(time % 1000 == 0)
		printf("Frames per second: %0.1f\n", fps);
		
	
	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	glFlush();
	glFinish();
	
	/* Display the buffer we just drew (necessary for double buffering). */
	glutSwapBuffers();
//	exit(1);

	// kuhl_video_record("videoout", 30);
	
	/* Ask GLUT to call display() again. We shouldn't call display()
	 * ourselves recursively because it will not leave time for GLUT
	 * to call other callback functions for when a key is pressed, the
	 * window is resized, etc. */
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
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

	/* set up our GLUT window */
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	/* Ask GLUT to for a double buffered, full color window that
	 * includes a depth buffer */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glEnable(GL_MULTISAMPLE);
	glutInitContextVersion(3,0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // Don't allow deprecated OpenGL calls.
	glutCreateWindow(argv[0]); // set window title to executable name

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

	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
//	glUniform1i(kuhl_get_uniform(program, "red"), 1);
	kuhl_errorcheck();

	program = kuhl_create_program("ogl3-assimp.vert", "ogl3-assimp.frag");
	glUseProgram(program);


	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	float initPos[3] = {0,0,2};
	float initLook[3] = {0,0,0};
	float initUp[3] = {0,1,0};

	// Initialize DGR
	dgr_init();
	projmat_init();
	viewmat_init(initPos, initLook, initUp);

	// Clear the screen while things might be loading
	glClearColor(.2,.2,.2,1);
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();
	
	/* Tell GLUT to start running the main loop and to call display(),
	 * keyboard(), etc callback methods as needed. */
	glutMainLoop();
	exit(EXIT_SUCCESS);
}
