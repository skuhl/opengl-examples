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
GLuint texName1 = 0;
GLuint texName2 = 0;


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

	glClearColor(1,1,1,0); // set clear color to white
	// Clear the screen to black, clear the depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // turn on depth testing
	kuhl_errorcheck();

	/* Turn on blending (note, if you are using transparent textures,
	   the transparency may not look correctly unless you draw further
	   items before closer items. This program always draws the
	   geometry in the same order.). */
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	
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

	/* Set up the texture. Make sure that our texture is set up in texture unit 0. */
	glActiveTexture(GL_TEXTURE0); /* Turn on and use texture unit 0 */
	kuhl_errorcheck();
	glBindTexture(GL_TEXTURE_2D, texName1); /* Use our previously loaded texture */
	kuhl_errorcheck();
	glDrawArrays(GL_TRIANGLES, 0, 3); // DRAW!
	kuhl_errorcheck();

	/* Add a new translation to the modelview matrix. */
	float translate[16];
	mat4f_translate_new(translate, 1.2, 0, 0);
	mat4f_mult_mat4f_new(modelview, modelview, translate);
	glUniformMatrix4fv(kuhl_get_uniform(program, "ModelView"),
	                   1, // count
	                   0, // transpose
	                   modelview); // value
	glBindTexture(GL_TEXTURE_2D, texName2); 
	glDrawArrays(GL_TRIANGLES, 0, 3); // DRAW again---but translated this time.
	
	// unbind:
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
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

	/* Set up a buffer object (BO) which is a place to store the data on the graphics card. */
	GLuint vbuffer;
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	kuhl_errorcheck();
	
	/* The data that we want to draw */
	GLfloat vertexData[] = {0, 0, 0,
	                        1, 0, 0,
	                        1, 1, 0};
	/* Copy the our data into the buffer object that is currently bound. */
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	kuhl_errorcheck();

	/* Tell OpenGL some information about the data that is in the
	 * buffer. Among other things, we need to tell OpenGL which
	 * attribute number (i.e., variable) the data should correspond to
	 * in the vertex program. */
	glEnableVertexAttribArray(0); // turn on attribute location 0
	glVertexAttribPointer(
		0, // attribute location in glsl program
		3, // number of elements (x,y,z)
		GL_FLOAT, // type of each element
		GL_FALSE, // should OpenGL normalize values?
		0,        // no extra data between each position
		0 );      // offset of first element
	kuhl_errorcheck();

	/* Set up a buffer object (BO) which is a place to store data (in this case, texture coordinates) on the graphics card. */
	GLuint tcbuffer;
	glGenBuffers(1, &tcbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tcbuffer);
	kuhl_errorcheck();
	
	/* The data that we want to draw */
	GLfloat texcoordData[] = {0, 0,
	                          1, 0,
	                          1, 1 };
	/* Copy the our data into the buffer object that is currently bound. */
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoordData), texcoordData, GL_STATIC_DRAW);
	kuhl_errorcheck();

	/* Tell OpenGL some information about the data that is in the
	 * buffer. Among other things, we need to tell OpenGL which
	 * attribute number (i.e., variable) the data should correspond to
	 * in the vertex program. */
	glEnableVertexAttribArray(1); // turn on attribute location 0
	glVertexAttribPointer(
		1, // attribute location in glsl program
		2, // number of elements (u,v)
		GL_FLOAT, // type of each element
		GL_FALSE, // should OpenGL normalize values?
		0,        // no extra data between each position
		0 );      // offset of first element
	kuhl_errorcheck();

	/* Load the texture. It will be bound to texName */
	kuhl_read_texture_file("images/blue.png", &texName1);
	kuhl_read_texture_file("images/rainbow-alpha.png", &texName2);
	kuhl_errorcheck();
	/* We will make sure that our texture is in texture unit 0 inside
	 * of display(). Here, we tell OpenGL that the texture that we
	 * refer to in our GLSL program (named "tex") is going to be in
	 * texture unit 0. */
	glUniform1i(kuhl_get_uniform(program, "tex"), 0);
	kuhl_errorcheck();

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
	char *attribs[] = { "in_Position", "in_TexCoord", NULL };
	program = kuhl_create_program("ogl3-texture.vert", "ogl3-texture.frag", attribs);
	glUseProgram(program);
	kuhl_errorcheck();

	init_geometryTriangle();
	
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
