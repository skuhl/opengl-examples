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
#include <GLFW/glfw3.h>

#include "msg.h"

/* Will print messages when glfw errors occur. */
void glfw_error(int error, const char* description)
{
	msg(MSG_ERROR, "GLFW error: %s\n", description);
}


int main(int argc, char** argv)
{
	glfwSetErrorCallback(glfw_error);
	if(!glfwInit()) // initialize glfw
	{
		msg(MSG_FATAL, "Failed to initialize GLFW.\n");
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_VISIBLE, GL_FALSE); // don't show window
	GLFWwindow *window = glfwCreateWindow(512, 512, argv[0], NULL, NULL);
	if(!window)
	{
		msg(MSG_FATAL, "Failed to create a GLFW window.\n");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

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

	/* texture size */
	GLint maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	msg(MSG_INFO, "Maximum texture size estimate: %dx%d\n", maxTextureSize, maxTextureSize);
	
	
	exit(EXIT_SUCCESS);
}
