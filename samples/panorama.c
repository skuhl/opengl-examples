/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file This program demonstrates how to load cylindrical and
 * cubemap panorama photos (either in mono or stereo modes).
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
static kuhl_geometry quad;
static kuhl_geometry cylinder;

/* cylindrical panorama textures */
static GLuint texIdLeft  = 0;
static GLuint texIdRight = 0;

/* cubemap textures */
static GLuint cubemapLeftTex[6];
static GLuint cubemapRightTex[6];



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
		case GLFW_KEY_S: // swap the left & right images
		{
			GLuint tmp;
			tmp = texIdLeft;
			texIdLeft = texIdRight;
			texIdRight = tmp;
			break;
		}
	}
}

void setupCubemap(GLuint texId[6], kuhl_geometry q, const float origModelView[16])
{
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),1,0,origModelView);
	kuhl_geometry_texture(&q, texId[0], "tex", KG_WARN);
	kuhl_geometry_draw(&q); // negative Z (front)
	
	float rotation[16];
	float modelview[16];
	mat4f_rotateEuler_new(rotation, 0,180,0, "XYZ");
	mat4f_mult_mat4f_new(modelview, origModelView, rotation);
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),1,0,modelview);
	kuhl_geometry_texture(&q, texId[1], "tex", KG_WARN);
	kuhl_geometry_draw(&q); // positive Z (back)

	mat4f_rotateEuler_new(rotation, 0,90,0, "XYZ");
	mat4f_mult_mat4f_new(modelview, origModelView, rotation);
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),1,0,modelview);
	kuhl_geometry_texture(&q, texId[2], "tex", KG_WARN);
	kuhl_geometry_draw(&q); // negative X (left)

	mat4f_rotateEuler_new(rotation, 0,-90,0, "XYZ");
	mat4f_mult_mat4f_new(modelview, origModelView, rotation);
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),1,0,modelview);
	kuhl_geometry_texture(&q, texId[3], "tex", KG_WARN);
	kuhl_geometry_draw(&q); // positive X (right)

	mat4f_rotateEuler_new(rotation, -90,0,0, "XYZ");
	mat4f_mult_mat4f_new(modelview, origModelView, rotation);
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),1,0,modelview);
	kuhl_geometry_texture(&q, texId[4], "tex", KG_WARN);
	kuhl_geometry_draw(&q); // negative Y (down)
	
	mat4f_rotateEuler_new(rotation, 90,0,0, "XYZ");
	mat4f_mult_mat4f_new(modelview, origModelView, rotation);
	glUniformMatrix4fv(kuhl_get_uniform("ModelView"),1,0,modelview);
	kuhl_geometry_texture(&q, texId[5], "tex", KG_WARN);
	kuhl_geometry_draw(&q); // positive Y (up)
}


/** Draws the 3D scene. */
void display()
{
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
		float scaleMat[16];
		mat4f_scale_new(scaleMat, 20, 20, 20);

		// Modelview = (viewMatrix * scaleMat) * rotationMatrix
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, scaleMat);

		/* Tell OpenGL which GLSL program the subsequent
		 * glUniformMatrix4fv() calls are for. */
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

		if(texIdLeft != 0 && texIdRight != 0) // cylinder
		{
			/* Draw the cylinder with the appropriate texture */
			if(eye == VIEWMAT_EYE_RIGHT)
				kuhl_geometry_texture(&cylinder, texIdRight, "tex", KG_WARN);
			else
				kuhl_geometry_texture(&cylinder, texIdLeft, "tex", KG_WARN);
			kuhl_geometry_draw(&cylinder);
		}
		else // cubemap
		{
			if(eye == VIEWMAT_EYE_RIGHT)
				setupCubemap(cubemapRightTex, quad, modelview);
			else
				setupCubemap(cubemapLeftTex, quad, modelview);
		}
		viewmat_end_eye(viewportID);
	} // finish viewport loop
	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	 * calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();


	static int counter = 0;
	counter++;
	if(counter % 10 == 0)
		msg(MSG_INFO, "fps: %6.2f\n", bufferswap_fps());
}


/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuad(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog,
	                  4, // number of vertices
	                  GL_TRIANGLES); // type of thing to draw

	/* The data that we want to draw */
	GLfloat vertexPositions[] = {-.5, -.5, -.5,
	                              .5, -.5, -.5,
	                              .5,  .5, -.5,
	                             -.5,  .5, -.5 };
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


/** Creates a cylinder geometry object. */
void init_geometryCylinder(kuhl_geometry *cyl, GLuint prog)
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

	kuhl_geometry_new(cyl, prog, verticesIndex/3, GL_TRIANGLES);
	kuhl_geometry_attrib(cyl, vertices, 3, "in_Position", 1);
	kuhl_geometry_attrib(cyl, normals, 3, "in_Normal", 1);
	kuhl_geometry_attrib(cyl, texcoords, 2, "in_TexCoord", 1);
	kuhl_geometry_indices(cyl, indices, indicesIndex);

	/*
	  printf("v: %d\n", verticesIndex);
	  printf("n: %d\n", normalsIndex);
	  printf("t: %d\n", texcoordsIndex);
	  printf("i: %d\n", indicesIndex);
	*/
}


int main(int argc, char** argv)
{
	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 512, 512, 32, 4);

	
	if(argc != 2 && argc != 3 && argc != 7 && argc != 13)
	{
		printf("Usage for cylindrical panoramas:\n");
		printf("   %s panoImage.jpg\n", argv[0]);
		printf("   %s left.jpg right.jpg\n", argv[0]);
		printf("\n");
		printf("Usage for cubemap panoramas:\n");
		printf("   %s front.jpg back.jpg left.jpg right.jpg down.jpg up.jpg\n", argv[0]);
		printf("   %s Lfront.jpg Lback.jpg Lleft.jpg Lright.jpg Ldown.jpg Lup.jpg Rfront.jpg Rback.jpg Rleft.jpg Rright.jpg Rdown.jpg Rup.jpg\n", argv[0]);
		printf("\n");
		printf("Tip: Works best if horizon is at center of the panorama.\n");
		exit(EXIT_FAILURE);
	}
	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	 * a fragment shader. */
	program = kuhl_create_program("texture.vert", "texture.frag");
	glUseProgram(program);
	kuhl_errorcheck();

	init_geometryQuad(&quad, program);
	init_geometryCylinder(&cylinder, program);

	if(argc == 2)
	{
		msg(MSG_INFO, "Cylinder mono image: %s\n", argv[1]);
		if(kuhl_read_texture_file(argv[1], &texIdLeft) < 0)
			exit(EXIT_FAILURE);
		texIdRight = texIdLeft;
	}
	if(argc == 3)
	{
		msg(MSG_INFO, "Cylinder left  image: %s\n", argv[1]);
		if(kuhl_read_texture_file(argv[1], &texIdLeft) < 0)
			exit(EXIT_FAILURE);
		msg(MSG_INFO, "Cylinder right image: %s\n", argv[2]);
		if(kuhl_read_texture_file(argv[2], &texIdRight) < 0)
			exit(EXIT_FAILURE);
	}

	char *cubemapNames[] = { "front", "back", "left", "right", "down", "up" };	
	if(argc == 7)
	{
		for(int i=0; i<6; i++)
		{
			msg(MSG_INFO, "Cubemap image (%-5s): %s\n", cubemapNames[i], argv[i+1]);
			if(kuhl_read_texture_file(argv[i+1], &(cubemapLeftTex[i])) < 0)
				exit(EXIT_FAILURE);
			cubemapRightTex[i]= cubemapLeftTex[i];
			texIdLeft =0;
			texIdRight=0;
		}
	}
	if(argc == 13)
	{
		for(int i=0; i<6; i++)
		{
			msg(MSG_INFO, "Cubemap image (left,  %-5s): %s\n", cubemapNames[i], argv[i+6+1]);
			if(kuhl_read_texture_file(argv[i+1], &(cubemapLeftTex[i])) < 0)
				exit(EXIT_FAILURE);
			msg(MSG_INFO, "Cubemap image (right, %-5s)\n", cubemapNames[i], argv[i+6+1]);
			if(kuhl_read_texture_file(argv[i+6+1], &(cubemapRightTex[i])) < 0)
				exit(EXIT_FAILURE);
			texIdLeft =0;
			texIdRight=0;
		}
	}
	
	
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	dgr_init();     /* Initialize DGR based on environment variables. */

	float initCamPos[3]  = {0,0,0}; // location of camera
	float initCamLook[3] = {0,0,-1}; // a point the camera is facing at
	float initCamUp[3]   = {0,1,0}; // a vector indicating which direction is up
	viewmat_init(initCamPos, initCamLook, initCamUp);
	
	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}

	exit(EXIT_SUCCESS);
}
