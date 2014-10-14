/* This example demonstrates how to draw a HUD cursor and how to use
 * the stencil buffer to determine what piece of geometry the cursor
 * is on. For more information and details, see:
 * http://en.wikibooks.org/wiki/OpenGL_Programming/Object_selection
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

kuhl_geometry cursor;
kuhl_geometry triangle;
kuhl_geometry quad;


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

	/* Whenever any key is pressed, request that display() get
	 * called. */ 
	glutPostRedisplay();
}

/* Called by GLUT whenever the window needs to be redrawn. This
 * function should not be called directly by the programmer. Instead,
 * we can call glutPostRedisplay() to request that GLUT call display()
 * at some point. */
void display()
{
	/* If we are using DGR, send or receive data to keep multiple
	 * processes/computers synchronized. */
	dgr_update();

	glClearColor(0,0,0,0); // set clear color to black
	glClearStencil(0); // set the stencil value to zero
	// Clear the screen to black, clear the depth buffer, clear the stencil buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); // turn on depth testing
	kuhl_errorcheck();

	/* Render the scene once for each viewport. Frequently one
	 * viewport will fill the entire screen. However, this loop will
	 * run twice for HMDs (once for the left eye and once for the
	 * right. */
	for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)
	{
		/* Where is the viewport that we are drawing onto and what is its size? */
		int viewport[4]; // x,y of lower left corner, width, height
		viewmat_get_viewport(viewport, viewportID);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Get the frustum information which will be later used to generate a perspective projection matrix. */
		float f[6]; // left, right, top, bottom, near>0, far>0
		projmat_get_frustum(f, viewport[2], viewport[3]);
	    
		/* Get the view or camera matrix; update the frustum values if needed. */
		float viewMat[16];
		viewmat_get(viewMat, f, viewportID);

		/* Create a 4x4 perspective projection matrix from the frustum values. */
		float perspective[16];
		mat4f_frustum_new(perspective,f[0], f[1], f[2], f[3], f[4], f[5]);
	    
		/* Calculate an angle to rotate the
		 * object. glutGet(GLUT_ELAPSED_TIME) is the number of
		 * milliseconds since glutInit() was called. */
		int count = glutGet(GLUT_ELAPSED_TIME) % 10000; // get a counter that repeats every 10 seconds
		float angle = count / 10000.0 * 360; // rotate 360 degrees every 10 seconds
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
		 * vertex programs immediately above. Use the stencil buffer
		 * to keep track of which object appears on top. */
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, -1);
		kuhl_geometry_draw(&triangle);

		glStencilFunc(GL_ALWAYS, 2, -1);
		kuhl_geometry_draw(&quad);
		glDisable(GL_STENCIL_TEST);

		/* If we have multiple viewports, only draw cursor in the
		 * first viewport. */
		if(viewportID==0)
		{
			/* Draw the cursor in normalized device coordinates. Don't
			 * use any matrices. */
			float identity[16];
			mat4f_identity(identity);
			glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			                   1, 0, identity);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			                   1, 0, identity);

			/* Disable depth testing so the cursor isn't occluded by
			 * anything. */
			glDisable(GL_DEPTH_TEST);
			kuhl_geometry_draw(&cursor);
			glEnable(GL_DEPTH_TEST);

			GLuint stencilVal = 0;
			glReadPixels(viewport[0]+viewport[2]/2, viewport[1]+viewport[3]/2,
			             1,1, // get data for 1x1 area (i.e., a pixel)
			             GL_STENCIL_INDEX, // query the stencil buffer
			             GL_UNSIGNED_INT,
			             &stencilVal);
			if(stencilVal == 1)
				printf("Cursor is on triangle.\n");
			else if(stencilVal == 2)
				printf("Cursor is on quad.\n");
			else
				printf("Cursor isn't on anything.\n");
		}

		glUseProgram(0); // stop using a GLSL program.
		
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

void init_geometryTriangle(GLuint program)
{
	kuhl_geometry_zero(&triangle);
	triangle.program = program;
	triangle.primitive_type = GL_TRIANGLES;

	/* The data that we want to draw */
	GLfloat vertexData[] = {0, 0, 0,
	                        1, 0, 0,
	                        1, 1, 0};
	triangle.vertex_count = 3; // 3 vertices
	triangle.attrib_pos = vertexData;
	triangle.attrib_pos_components = 3; // each vertex has X, Y, Z
	triangle.attrib_pos_name = "in_Position";
	GLfloat colorData[] = { 1,0,0,
	                        0,1,0,
	                        0,0,1 };
	triangle.attrib_color = colorData;
	triangle.attrib_color_components = 3; // each vertex has X, Y, Z
	triangle.attrib_color_name = "in_Color";

	kuhl_geometry_init(&triangle);
}

void init_geometryCursor(GLuint program)
{
	kuhl_geometry_zero(&cursor);
	cursor.program = program;
	cursor.primitive_type = GL_LINES;

	/* The data that we want to draw */
	GLfloat vertexData[] = {-.04, 0, 0,
	                         .04, 0, 0,
	                        0, -.04, 0,
	                        0,  .04, 0 };
	cursor.vertex_count = 4;
	cursor.attrib_pos = vertexData;
	cursor.attrib_pos_components = 3; // each vertex has X, Y, Z
	cursor.attrib_pos_name = "in_Position";
	GLfloat colorData[] = { 1,1,1,
	                        1,1,1,
	                        1,1,1,
	                        1,1,1 };
	cursor.attrib_color = colorData;
	cursor.attrib_color_components = 3; // each vertex has X, Y, Z
	cursor.attrib_color_name = "in_Color";
	
	kuhl_geometry_init(&cursor);
}



/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuad(GLuint program)
{
	kuhl_geometry_zero(&quad);
	quad.program = program;
	quad.primitive_type = GL_TRIANGLES;


	/* The data that we want to draw */
	GLfloat vertexData[] = {0+1.1, 0, 0,
	                        1+1.1, 0, 0,
	                        1+1.1, 1, 0,
	                        0+1.1, 1, 0 };
	quad.vertex_count = 4;  // 4 vertices
	quad.attrib_pos_components = 3; // each vertex has X, Y, Z
	quad.attrib_pos = vertexData;
	quad.attrib_pos_name = "in_Position";

	GLfloat colorData[] = { 1,0,0,
	                        0,1,0,
	                        0,0,1,
	                        0,1,1 };
	quad.attrib_color_components = 3;
	quad.attrib_color_name = "in_Color";
	quad.attrib_color = colorData;

	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
	                       0, 2, 3 }; // indices of second triangle.
	quad.indices = indexData;
	quad.indices_len = 6;
	kuhl_geometry_init(&quad);
}

int main(int argc, char** argv)
{
	/* set up our GLUT window */
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	/* Ask GLUT to for a double buffered, full color window that
	 * includes a depth buffer */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
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

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program("ogl3-triangle-color.vert", "ogl3-triangle-color.frag");
	glUseProgram(program);
	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	glUniform1i(kuhl_get_uniform("red"), 1);
	kuhl_errorcheck();
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	/* Create kuhl_geometry structs for the objects that we want to
	 * draw. */
	init_geometryCursor(program);
	init_geometryTriangle(program);
	init_geometryQuad(program);

	dgr_init();     /* Initialize DGR based on environment variables. */
	projmat_init(); /* Figure out which projection matrix we should use based on environment variables */

	float initCamPos[3]  = {0,0,10}; // location of camera
	float initCamLook[3] = {0,0,0}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);
	
	/* Tell GLUT to start running the main loop and to call display(),
	 * keyboard(), etc callback methods as needed. */
	glutMainLoop();
    /* // An alternative approach:
    while(1)
       glutMainLoopEvent();
    */

	exit(EXIT_SUCCESS);
}
