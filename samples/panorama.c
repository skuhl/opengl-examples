#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include "kuhl-util.h"
#include "vecmat.h"
#include "dgr.h"
#include "projmat.h"
#include "viewmat.h"
GLuint program = 0; // id value for the GLSL program

kuhl_geometry triangle;
kuhl_geometry cylinder;

GLuint texIdLeft  = 0;
GLuint texIdRight = 0;


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
		case 's': // swap the left & right images
		{
			GLuint tmp;
			tmp = texIdLeft;
			texIdLeft = texIdRight;
			texIdRight = tmp;
			break;
		}
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
		   items before closer items. This program always draws the
		   geometry in the same order.). */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		/* Get the view or camera matrix; update the frustum values if needed. */
		float viewMat[16], perspective[16];
		viewmat_eye eye = viewmat_get(viewMat, perspective, viewportID);

		/* Always put camera at origin (no translation, no
		 * interpupillary distance). */
		float noTranslation[4] = {0,0,0,1};
		mat4f_setColumn(viewMat, noTranslation, 3); // last column

		/* Create a scale matrix. */
		float scaleMatrix[16];
		mat4f_scale_new(scaleMatrix, 30, 30, 30);

		// Modelview = (viewMatrix * scaleMatrix) * rotationMatrix
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, scaleMatrix);

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

		/* Draw the cylinder with the appropriate texture */
		if(eye == VIEWMAT_EYE_RIGHT)
			kuhl_geometry_texture(&cylinder, texIdRight, "tex", KG_WARN);
		else
			kuhl_geometry_texture(&cylinder, texIdLeft, "tex", KG_WARN);
		kuhl_geometry_draw(&cylinder);

	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	/* Ask GLUT to call display() again. We shouldn't call display()
	 * ourselves recursively because it will not leave time for GLUT
	 * to call other callback functions for when a key is pressed, the
	 * window is resized, etc. */
	glutPostRedisplay();
}


/** Creates a cylinder geometry object. */
void init_geometryCylinder(kuhl_geometry *cylinder, GLuint program)
{
	int numSides=50;
	
	GLfloat vertices[1024];
	GLfloat normals[1024];
	GLuint indices[1024];
	GLfloat texcoords[1024];
	int verticesIndex = 0;
	int normalsIndex = 0;
	int indicesIndex = 0;
	int texcoordsIndex = 0;

	// Bottom middle
	vertices[verticesIndex++] = 0;
	vertices[verticesIndex++] = -.5;
	vertices[verticesIndex++] = -0;
	normals[normalsIndex++] = 0;
	normals[normalsIndex++] = -1;
	normals[normalsIndex++] = 0;
	texcoords[texcoordsIndex++] = 0;
	texcoords[texcoordsIndex++] = 0;
	

	// For each vertex around bottom perimeter
	for(int i=0; i<numSides; i++)
	{
		vertices[verticesIndex++] = .5*sin(i*2*M_PI/numSides);
		vertices[verticesIndex++] = -.5;
		vertices[verticesIndex++] = .5*cos(i*2*M_PI/numSides);
		normals[normalsIndex++] = 0;
		normals[normalsIndex++] = -1;
		normals[normalsIndex++] = 0;
		texcoords[texcoordsIndex++] = 0;
		texcoords[texcoordsIndex++] = 0;
	}

	// For each face/triangle on bottom, what are the 3 vertices?
	for(int i=0; i<numSides; i++)
	{
		indices[indicesIndex++] = 0;    // center point
		indices[indicesIndex++] = i+1;
		if(i+2 >= numSides+1)
			indices[indicesIndex++] = 1;
		else
			indices[indicesIndex++] = i+2;
	}


	int verticesIndexTopStart = verticesIndex;
	// Bottom middle
	vertices[verticesIndex++] = 0;
	vertices[verticesIndex++] = .5;
	vertices[verticesIndex++] = -0;
	normals[normalsIndex++] = 0;
	normals[normalsIndex++] = 1;
	normals[normalsIndex++] = 0;
	texcoords[texcoordsIndex++] = 1;
	texcoords[texcoordsIndex++] = 1;


	// For each vertex around bottom perimeter
	for(int i=0; i<numSides; i++)
	{
		vertices[verticesIndex++] = .5*sin(i*2*M_PI/numSides);
		vertices[verticesIndex++] = .5;
		vertices[verticesIndex++] = .5*cos(i*2*M_PI/numSides);
		normals[normalsIndex++] = 0;
		normals[normalsIndex++] = 1;
		normals[normalsIndex++] = 0;
		texcoords[texcoordsIndex++] = 1;
		texcoords[texcoordsIndex++] = 1;
	}

	// For each face/triangle on bottom
	for(int i=0; i<numSides; i++)
	{
		indices[indicesIndex++] = verticesIndexTopStart/3;    // center point
		indices[indicesIndex++] = verticesIndexTopStart/3+i+1;
		if(i+2 >= numSides+1)
			indices[indicesIndex++] = verticesIndexTopStart/3+1;
		else
			indices[indicesIndex++] = verticesIndexTopStart/3+i+2;
	}

	// For each vertex on faces on the sides of the cylinder
	for(int i=0; i<numSides; i++)
	{
		vertices[verticesIndex++] = .5*sin(i*2*M_PI/numSides);
		vertices[verticesIndex++] = .5;
		vertices[verticesIndex++] = .5*cos(i*2*M_PI/numSides);

		vertices[verticesIndex++] = .5*sin(i*2*M_PI/numSides);
		vertices[verticesIndex++] = -.5;
		vertices[verticesIndex++] = .5*cos(i*2*M_PI/numSides);

		vertices[verticesIndex++] = .5*sin((i+1)*2*M_PI/numSides);
		vertices[verticesIndex++] = -.5;
		vertices[verticesIndex++] = .5*cos((i+1)*2*M_PI/numSides);

		vertices[verticesIndex++] = .5*sin((i+1)*2*M_PI/numSides);
		vertices[verticesIndex++] = .5;
		vertices[verticesIndex++] = .5*cos((i+1)*2*M_PI/numSides);

		for(int j=0; j<4; j++)
		{
			float pt1[] = { vertices[verticesIndex-3*4+0],
			                vertices[verticesIndex-3*4+1],
			                vertices[verticesIndex-3*4+2] };
			float pt2[] = { vertices[verticesIndex-3*4+3],
			                vertices[verticesIndex-3*4+4],
			                vertices[verticesIndex-3*4+5] };
			float pt3[] = { vertices[verticesIndex-3*4+6],
			                vertices[verticesIndex-3*4+7],
			                vertices[verticesIndex-3*4+8] };
			float v1[3],v2[3],n[3];
			vec3f_sub_new(v1, pt2,pt1);
			vec3f_sub_new(v2, pt3,pt1);
			vec3f_cross_new(n, v1, v2);

			normals[normalsIndex++] = n[0];
			normals[normalsIndex++] = n[1];
			normals[normalsIndex++] = n[2];

			if(j < 2)
				texcoords[texcoordsIndex++] = (numSides-i)/(float)numSides;
			else
				texcoords[texcoordsIndex++] = (numSides-(i+1))/(float)numSides;

			if(j == 1 || j == 2)
				texcoords[texcoordsIndex++] = 0; // bottom
			else
				texcoords[texcoordsIndex++] = 1; // top
		}

		indices[indicesIndex++] = verticesIndex/3-4;
		indices[indicesIndex++] = verticesIndex/3-3;
		indices[indicesIndex++] = verticesIndex/3-2;

		indices[indicesIndex++] = verticesIndex/3-4;
		indices[indicesIndex++] = verticesIndex/3-2;
		indices[indicesIndex++] = verticesIndex/3-1;
	}

	kuhl_geometry_new(cylinder, program, verticesIndex/3, GL_TRIANGLES);
	kuhl_geometry_attrib(cylinder, vertices, 3, "in_Position", 1);
	kuhl_geometry_attrib(cylinder, normals, 3, "in_Normal", 1);
	kuhl_geometry_attrib(cylinder, texcoords, 2, "in_TexCoord", 1);
	kuhl_geometry_indices(cylinder, indices, indicesIndex);

	/*
	  printf("v: %d\n", verticesIndex);
	  printf("n: %d\n", normalsIndex);
	  printf("t: %d\n", texcoordsIndex);
	  printf("i: %d\n", indicesIndex);
	*/
}


int main(int argc, char** argv)
{
	if(argc < 2 || argc > 3)
	{
		printf("Usage: %s panoImage.jpg\n", argv[0]);
		printf(" - or -\n");
		printf("Usage: %s left.jpg right.jpg\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	/* set up our GLUT window */
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	/* Ask GLUT to for a double buffered, full color window that
	 * includes a depth buffer */
#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitContextVersion(3,2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
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
	program = kuhl_create_program("texture.vert", "texture.frag");
	glUseProgram(program);
	kuhl_errorcheck();

	init_geometryCylinder(&cylinder, program);

	msg(INFO, "Left  image: %s\n", argv[1]);
	kuhl_read_texture_file(argv[1], &texIdLeft);
	if(argc == 3)
	{
		msg(INFO, "Right image: %s\n", argv[2]);
		kuhl_read_texture_file(argv[2], &texIdRight);
	}
	
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	dgr_init();     /* Initialize DGR based on environment variables. */
	projmat_init(); /* Figure out which projection matrix we should use based on environment variables */

	float initCamPos[3]  = {0,0,0}; // location of camera
	float initCamLook[3] = {0,0,-1}; // a point the camera is facing at
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
