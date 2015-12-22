/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file Prints information about the OpenGL context
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

#include "msg.h"

int main(int argc, char** argv)
{
	/* set up our GLUT window */
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	/* Ask GLUT to for a double buffered, full color window that
	 * includes a depth buffer */
#ifdef FREEGLUT
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE); // Windows NVIDIA seems to require GLUT_DOUBLE
	// Don't specify a specific context.
	// If we did, it can affect the information that gets printed out.
	//glutInitContextVersion(3,2);
	//glutInitContextProfile(GLUT_CORE_PROFILE);
#else
	glutInitDisplayMode(GLUT_RGB);
#endif
	glutCreateWindow(argv[0]); // set window title to executable name

	/* Initialize GLEW */
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if(glewError != GLEW_OK)
	{
		fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(glewError));
		exit(EXIT_FAILURE);
	}
	
	GLint numExten = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExten);
	msg(MSG_INFO, "%d extensions supported\n", numExten);
	for(GLuint i=0; i<(GLuint)numExten; i++)
		msg(MSG_INFO, "%4d: %s", i, glGetStringi(GL_EXTENSIONS, i));

	msg(MSG_INFO, "OpenGL version: %s", glGetString(GL_VERSION));
	msg(MSG_INFO, "  GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	msg(MSG_INFO, "       Version: %s", glGetString(GL_VENDOR));
	msg(MSG_INFO, "      Renderer: %s", glGetString(GL_RENDERER));
	
	exit(EXIT_SUCCESS);
}
