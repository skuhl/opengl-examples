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

GLuint triangleVAO = 0; // id for our vertex array object
GLuint quadVAO = 0;

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
	}
	glutPostRedisplay();
}

void display()
{
	dgr_update();

	glClearColor(0,0,0,0); // set clear color to black
	// Clear the screen to black, clear the depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // turn on depth testing
	kuhl_errorcheck();

	glUseProgram(program);



	for(int view=0; view<viewmat_num_viewports(); view++)
	{
		/* Where is the viewport that we are drawing onto and what is its size? */
		int viewport[4];
		viewmat_get_viewport(viewport, view);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Get the view frustum information. */
		float f[6];
		projmat_get_frustum(f, viewport[2], viewport[3]);
	    
		/* Get the projection matrix, update view frustum if necessary. */
		float viewMat[16];
		viewmat_get(viewMat, f, view);

		/* Communicate matricies to OpenGL */
		float perspective[16];
		mat4f_frustum_new(perspective,f[0], f[1], f[2], f[3], f[4], f[5]);
		glUniformMatrix4fv(kuhl_get_uniform(program, "Projection"),
		                   1, // count
		                   0, // transpose
		                   perspective); // value
	    
		/* Change angle for animation. */
		int count = glutGet(GLUT_ELAPSED_TIME) % 10000; // get a counter that repeats every 10 seconds
		float angle = count / 10000.0 * 360;
		dgr_setget("angle", &angle, sizeof(GLfloat));
		float animationMat[16];
		mat4f_rotateAxis_new(animationMat, angle, 0,1,0);

		float scaleMatrix[16];
		mat4f_scale_new(scaleMatrix, 3, 3, 3);
		
		// modelview = lookat * scale * animation
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, scaleMatrix);
		mat4f_mult_mat4f_new(modelview, modelview, animationMat);

		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();
		glUniformMatrix4fv(kuhl_get_uniform(program, "ModelView"),
		                   1, // count
		                   0, // transpose
		                   modelview); // value
		kuhl_errorcheck();


		/* Draw the scene. These examples draw triangles, but OpenGL can
		 * draw other simple shapes too. The images on this page should
		 * help you understand the different options (note: you may see
		 * some non-OpenGL images in the results too!):
		 * https://www.google.com/search?q=opengl+primitive+types&tbm=isch
		 */
		kuhl_errorcheck();
		glBindVertexArray(triangleVAO);
		kuhl_errorcheck();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		kuhl_errorcheck();
	
		glBindVertexArray(quadVAO);
		kuhl_errorcheck();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
	} // finish viewport loop
	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();
    
	/* Display the buffer we just drew (necessary for double buffering). */
	glutSwapBuffers();

	/* Ask GLUT to call display() again. We shouldn't call display()
	 * ourselves recursively because it will not leave time for GLUT
	 * to call other callback functions for when a key is pressed, the
	 * window is resized, etc. */
	glutPostRedisplay();
}

void init_geometryTriangle()
{
	/* Create a vertex array object (VAO) */
	glGenVertexArrays(1, &triangleVAO);
	glBindVertexArray(triangleVAO);

	/* The data that we want to draw */
	GLfloat vertexData[] = {0, 0, 0,
	                        1, 0, 0,
	                        1, 1, 0};
	kuhl_array_buffer(vertexData,
	                  9,  // array contains 9 items
	                  0,  // attribute location
	                  3);  // 3 components

	/* Unbind VAO. */
	glBindVertexArray(0);
}


/* This illustrates how to draw a quad by drawing two triangles. An
 * easier way to draw a quad would be to use glDrawArray(GL_QUADS,
 * ...), but this allows us to draw a quad with only triangles. The
 * purpose of this is to show how you can refer to vertices in a
 * list. This saves space because if the same vertex is used by
 * multiple triangles, you don't have to store the vertex in memory
 * multiple times. */
void init_geometryQuad()
{
	/* Create a vertex array object (VAO) */
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	/* The data that we want to draw */
	GLfloat vertexData[] = {0+1.1, 0, 0,
	                        1+1.1, 0, 0,
	                        1+1.1, 1, 0,
	                        0+1.1, 1, 0 };
	kuhl_array_buffer(vertexData,
	                  3*4,  // array contains 9 items
	                  0,  // attribute location
	                  3);  // 3 components


	/* Copy the indices data into the currently bound buffer. */
	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
	                       0, 2, 3 }; // indices of second triangle.
	kuhl_element_array_buffer(indexData,
	                          6,  // array contains 6 items
	                          0); // attribute location

	/* Unbind VAO. */
	glBindVertexArray(0);
}

int main(int argc, char** argv)
{
	/* set up our GLUT window */
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	/* Ask GLUT to for a double buffered, full color window that
	 * includes a depth buffer */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
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

	/* attribs is a list of attributes in the program. The last item
	 * should be NULL so kuhl_create_program() can determine how many
	 * attributes you passed in. The first attribute in this list will
	 * be stored at location 0, the second at location 1, etc. */
	char *attribs[] = { "in_Position", NULL };
	program = kuhl_create_program("ogl3-triangle.vert", "ogl3-triangle.frag", attribs);
	glUseProgram(program);
	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	glUniform1i(kuhl_get_uniform(program, "red"), 1);
	kuhl_errorcheck();

	init_geometryTriangle();
	init_geometryQuad();

	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	float initPos[3] = {0,0,3};
	float initLook[3] = {0,0,0};
	float initUp[3] = {0,1,0};

	// Initialize DGR
	dgr_init();
	projmat_init();
	viewmat_init(initPos, initLook, initUp);

	/* Tell GLUT to start running the main loop and to call display(),
	 * keyboard(), etc callback methods as needed. */
	glutMainLoop();
	exit(EXIT_SUCCESS);
}
