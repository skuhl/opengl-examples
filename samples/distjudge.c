/* Copyright (c) 2014-2017 Scott Kuhl. All rights reserved.
* License: This code is licensed under a 3-clause BSD license. See
* the file named "LICENSE" for a full copy of the license.
*/

/** @file Distance judgment experiment.
*
* @author Scott Kuhl, Bochao Li
*/

#include "libkuhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <time.h>

//for pre-rendering
kuhl_geometry prerendQuad;  // a quad to draw the prerender texuture on
GLuint prerenderTexID = 0;  // for prerenderTexture
GLuint prerenderFrameBuffer = 0; // prerender frame buffer
GLuint prerendProgram = 0; // prerender shader program
int preRend = 0; // value=0: no prerendering; value=1: rendering the circle peripheral; value=2: rendering square frame; value=3: blurry frame

//used for Bochao frame research
float frame_resize = -0.0098;
#define FOV_sim 0
#define WHITE_FRAME 1  // WHITE_FRAME = 1: white frame;  = 0: black frame; = 2: gray frame
#define DISTANCE_EXP 1

//black frame object
kuhl_geometry BFrame_1;
kuhl_geometry BFrame_2;
kuhl_geometry BFrame_3;
kuhl_geometry BFrame_4;

//target objects
kuhl_geometry *modeltargets[5]; // targets model

								//for experiment control sequence----------------------------------

								//to give a blackscreen
int blank_screen = 1;

int rand_targets = 0;

//Number of trials
int num_trails = 15;
float test_dist = 0.5;

//  x:-1.893
//	y:1.993
//	z:0

//	x:2.057
//	y:-1.216
//	z:0

//calculated vector:
//  x-3.209
//	y:3.95	
//	z:0

//normalized coordinates
//  x:-0.63054823960707860843160422320103
//  y:0.77615006121781255945928223173701
//  z:0

//rotate the place the room model to match the IVS lab
float rotOffset = 126;
float posOffset[3] = { -0.3,0.0f,0.9 };
float yaw_offset = 0;
int practise_flag = 2;
float practise_trails[2] = { 3.5, 5.5 };
//trails
float exp_trails[15] = { 2, 2, 2, 2.5, 3, 3, 3, 3.5, 4, 4, 4, 4.5, 5, 5, 5 };

//flag indicator for drawing a target
int target_flag = 0;

//text file where we record our data
FILE *dis_record;

//target distance for current trail
float target_distance = 0;

//parameters for recording distances
float start_pos[3] = { 0, 0, 0 };
float current_pos[3] = { 0, 0, 0 };

//the direction where the target should lay around
//float targ_direction[3] = { -0.41232313313, 0, -0.8587927615322591 };
float targ_direction[3] = { 0.8406944038796979502028538, 0, 0.5415098515127488464018887 };//normalized vector :0.7500326357557513413592

		  //the position to render the target
float targ_position[3] = { 0, 0, 0 };

//indicate the target type the range is from 0 to 4.
int targ_type = 0;

//store the result for current trail
float walk_dist = 0;

int exp_stage = 0;

int rand_dist_index = 0;
//--------------------------------------------end condtrol sequence



GLuint program = 0; /**< id value for the GLSL program */

int renderStyle = 2;

kuhl_geometry *modelgeom = NULL;
kuhl_geometry *origingeom = NULL;
float bbox[6];

					/** The following variable toggles the display an "origin+axis" marker
					* which draws a small box at the origin and draws lines of length 1
					* on each axis. Depending on which matrices are applied to the
					* marker, the marker will be in object, world, etc coordinates. */


					/** Initial position of the camera. 1.55 is a good approximate
					* eyeheight in meters.*/
const float initCamPos[3] = { 0.0f,0.0f,0.0f };

/** A point that the camera should initially be looking at. If
* fitToView is set, this will also be the position that model will be
* translated to. */
const float initCamLook[3] = { 0.0f,0.0f,-5.0f };

/** A vector indicating which direction is up. */
const float initCamUp[3] = { 0.0f,1.0f,0.0f };


#define GLSL_VERT_FILE "assimp.vert"
#define GLSL_FRAG_FILE "assimp.frag"


//get vrpn curret head position
void get_vrpn_pos_current(float pos[3])
{
#if 0
	float vrpnpos[3], vrpnorient[16];
	vrpn_get("DK2", "vrpnhost", vrpnpos, vrpnorient);

	pos[0] = vrpnpos[0];
	pos[1] = 0;
	pos[2] = vrpnpos[2];
	//printf("vrpnpos is x: %f ----  y:%f ----- z:%f----------\n", vrpnpos[0], vrpnpos[1], vrpnpos[2]);
#endif
	vec3f_set(pos, 0,0,0);
}

/*measure the distance of current trail*/
float get_distance(float start[3], float finish[3]) {

	float distance = 0;

	distance = sqrt(
		pow((finish[0] - start[0]), 2) + pow((finish[1] - start[1]), 2)
		+ pow((finish[2] - start[2]), 2));

	return distance;
}

//get the target position based on the target distance we currently examine
void get_target_position(float pos[3]) {

	pos[0] = start_pos[0] + target_distance * targ_direction[0];
	pos[1] = 0.0;
	pos[2] = start_pos[2] + target_distance * targ_direction[2];
	//pos[0] = start_pos[0] + test_dist * targ_direction[0];
	//pos[1] = 0.0;
	//pos[2] = start_pos[2] + test_dist * targ_direction[2];

}

//record the walked distances in a txt file
void record_trail_distance() {

	//record distance and write it to file
	get_vrpn_pos_current(current_pos);
	float result_dist = get_distance(start_pos, current_pos);
	int dist_percentage = 100 * result_dist / target_distance;
	if (practise_flag <= 0) {
		fprintf(dis_record, "%d      %f     %f      %d percent \n", 16 - num_trails,
			target_distance, result_dist, dist_percentage);
	}

	printf("trail %d at distance: %f , walked distance: %f, recorded\n",
		16 - num_trails, target_distance, result_dist);

	//delete the target distance from the pool by switch the current element with the last element
	if (practise_flag <= 0) {
		exp_trails[rand_dist_index] = exp_trails[num_trails - 1];


		num_trails -= 1;
	}
}

void generate_target() {

	get_target_position(targ_position);
	target_flag = 1;

	targ_type = rand() % 5;
}

//void reset_Oculus_orientation();
//void set_yaw_new_offset(float);

/* Called by GLFW whenever a key is pressed. */
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	switch (key)
	{
	case GLFW_KEY_1:
	{
		posOffset[0] = posOffset[0] + 0.3;
		break;
	}
	case GLFW_KEY_2:
	{
		posOffset[0] = posOffset[0] - 0.3;
		break;
	}
	case GLFW_KEY_3:
	{
		posOffset[2] = posOffset[2] + 0.3;
		break;
	}
	case GLFW_KEY_4:
	{
		posOffset[2] = posOffset[2] - 0.3;
		break;
	}
	case GLFW_KEY_5:
	{
		posOffset[1] = posOffset[1] - 0.3;
		break;
	}
	case GLFW_KEY_6:
	{
		posOffset[2] = posOffset[2] - 0.3;
		break;
	}
	case GLFW_KEY_Q:
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_R:
	{
		// Reload GLSL program from disk
		kuhl_delete_program(program);
		program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);
		/* Apply the program to the model geometry */
		kuhl_geometry_program(modelgeom, program, KG_FULL_LIST);
		break;
	}
	case GLFW_KEY_P:
	{
		rotOffset++;
		break;
	}
	case GLFW_KEY_I:
	{
		rotOffset--;
		break;
	}

	case GLFW_KEY_PERIOD: // Toggle different sections of the GLSL fragment shader
		switch (exp_stage) {
		case 0: //empty room
			exp_stage = exp_stage + 1;
			printf("\n experiment start!!\n\n");
			break;
		case 1: //generate target
			get_vrpn_pos_current(start_pos); //get the current position as a start position for current trail

											 /*printf("the position is x: %f;   y: %f;   z: %f   ;\n",
											 start_pos[0], start_pos[1], start_pos[2]);*/
			if (practise_flag <= 0) {
				rand_dist_index = rand() % num_trails; //randomly select a distance from the pool


				target_distance = exp_trails[rand_dist_index]; //select distance from the distance pool

				generate_target(); // generate the target
			}
			else {
				target_distance = practise_trails[practise_flag-1];
				generate_target();
			}

			//reset_Oculus_orientation();

			blank_screen = 0;
			exp_stage = exp_stage + 1;
			if (practise_flag <= 0) {
				printf("number %d trial: targets generated at distance: %f\n",
					16 - num_trails, target_distance);
			}
			else {
				printf("practise trial - target placed at distance: %f\n",
					target_distance);
			}


			/*printf(
				"the position is x: %f;   y: %f;   z: %f   ;  target type is %d \n",
				targ_position[0], targ_position[1], targ_position[2],
				targ_type);*/
			break;
		case 2: //blank screen, while subject walk to the target
			blank_screen = 1;
			exp_stage = exp_stage + 1;
			printf("Walk to the target.\n");
			break;
		case 3: //record the distance and write it to file
			if (practise_flag <= 0) {
				record_trail_distance();

				if (num_trails == 0) {
					exp_stage = 99;
					fclose(dis_record);
					printf("finished!!!\n");
					printf("finished!!!\n");
					printf("finished!!!\n");
				}
				else {
					exp_stage = 1;
				}
			}
			else { // for practise trails
				record_trail_distance();
				practise_flag = practise_flag - 1;
				exp_stage = 1;
			}
			break;
		}
		break;
	}
}


/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuadPrerender(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog, 4, GL_TRIANGLES);


	/* The data that we want to draw */
	GLfloat vertexPositions[] = { -1, -1, 0,
		1, -1, 0,
		1, 1, 0,
		-1, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions,
		3, // number of components x,y,z
		"in_Position", // GLSL variable
		KG_WARN); // warn if attribute is missing in GLSL program?

	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
		0, 2, 3 }; // indices of second triangle.
	kuhl_geometry_indices(geom, indexData, 6);

	GLfloat texcoordData[] = { 0, 0,
		1, 0,
		1, 1,
		0, 1 };
	kuhl_geometry_attrib(geom, texcoordData, 2, "in_TexCoord", KG_WARN);
}

/** Called by GLUT whenever the window needs to be redrawn. This
* function should not be called directly by the programmer. Instead,
* we can call glutPostRedisplay() to request that GLUT call display()
* at some point. */
void display()
{

	/* Ensure the slaves use the same render style as the master
	* process. */
	dgr_setget("style", &renderStyle, sizeof(int));

	//glEnable(GL_FRAMEBUFFER_SRGB);

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
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // set clear color to black
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
		mat4f_identity(modelMat);
		float modelview[16];

		//rotate the room to match the IVS lab settings
		float offsetRot[16];
		mat4f_identity(offsetRot);
		mat4f_rotateAxis_new(offsetRot, rotOffset, 0, 1, 0);
		mat4f_mult_mat4f_new(modelMat, modelMat, offsetRot);

		//move the room so that initial position match the IVS lab
		float offsetPos[16];
		mat4f_translate_new(offsetPos, posOffset[0], posOffset[1], posOffset[2]);
		mat4f_mult_mat4f_new(modelMat, modelMat, offsetPos);

		mat4f_mult_mat4f_new(modelview, viewMat, modelMat); // modelview = view * model

															/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			modelview); // value

		glUniform1i(kuhl_get_uniform("renderStyle"), renderStyle);

		kuhl_errorcheck();

		GLint drawBufferID;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawBufferID);

		if (preRend) {
			if (prerenderFrameBuffer == 0) {
				prerenderFrameBuffer = kuhl_gen_framebuffer(viewport[2], viewport[3],
					&prerenderTexID,
					NULL);
				kuhl_geometry_texture(&prerendQuad, prerenderTexID, "tex", 1);
			}

			kuhl_errorcheck();
			glBindFramebuffer(GL_FRAMEBUFFER, prerenderFrameBuffer); // draw to the texturebuffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		if (!blank_screen) {

			kuhl_geometry_draw(modelgeom); /* Draw the model */
			kuhl_errorcheck();

			if (DISTANCE_EXP) {
				if (target_flag) {
					//kuhl_geometry *targ_model = modeltargets[targ_type];
					mat4f_translate_new(modelMat, targ_position[0], targ_position[1], targ_position[2]);
					mat4f_mult_mat4f_new(modelview, viewMat, modelMat);

					glUniformMatrix4fv(kuhl_get_uniform("Projection"), 1, 0,
						perspective);
					glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, // number of 4x4 float matrices
						0, // transpose
						modelview); // value
					kuhl_geometry_draw(modeltargets[targ_type]);
					//printf("hello\n");
				}
			}

			//draw the black frame
			if (FOV_sim) {
				//				printf("Print out black frame\n");
				/* Draw black frame */
				/*this is to simulate the FOV of nVis ST60 HMD inside DK2 HMD*/
				glDisable(GL_DEPTH_TEST);
				//glEnable(GL_DEPTH_CLAMP);
				glUniform1i(kuhl_get_uniform("renderStyle"), 1);
				//				glDisable(GL_DEPTH_CLAMP);

				float frame_vertical = 4.0 * frame_resize / 5.0;

				//right edge
				float stretchLabel[16];
				mat4f_scale_new(stretchLabel, 4, 4, 1);
				float transLabel[16];
				mat4f_translate_new(transLabel, 0.025 + frame_resize, -2, -0.03);
				//				mat4f_translate_new(transLabel, 0, -2, -1.0);
				mat4f_mult_mat4f_new(modelview, transLabel, stretchLabel);
				glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, 0,
					modelview);
				float identity[16];
				mat4f_identity(identity);
				glUniformMatrix4fv(kuhl_get_uniform("Projection"), 1, 0,
					perspective);
				kuhl_geometry_draw(&BFrame_1); // draw the right edge

											   //top edge
				mat4f_scale_new(stretchLabel, 4, 4, 1);
				mat4f_translate_new(transLabel, -2, 0.02 + frame_vertical, -0.03);
				mat4f_mult_mat4f_new(modelview, transLabel, stretchLabel);
				glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, 0,
					modelview);
				glUniformMatrix4fv(kuhl_get_uniform("Projection"), 1, 0,
					perspective);
				kuhl_geometry_draw(&BFrame_2);

				//left edge
				mat4f_scale_new(stretchLabel, 4, 4, 1);
				mat4f_translate_new(transLabel, -4.025 - frame_resize, -2, -0.03);
				mat4f_mult_mat4f_new(modelview, transLabel, stretchLabel);
				glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, 0,
					modelview);
				glUniformMatrix4fv(kuhl_get_uniform("Projection"), 1, 0,
					perspective);
				kuhl_geometry_draw(&BFrame_3);

				//bottom edge
				mat4f_scale_new(stretchLabel, 4, 4, 1);
				mat4f_translate_new(transLabel, -2, -4.02 - frame_vertical, -0.03);
				mat4f_mult_mat4f_new(modelview, transLabel, stretchLabel);
				glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, 0,
					modelview);
				glUniformMatrix4fv(kuhl_get_uniform("Projection"), 1, 0,
					perspective);
				kuhl_geometry_draw(&BFrame_4);

				/* Don't use depth testing and make sure we use the texture
				* rendering style */

				//				glUniform1i(kuhl_get_uniform("renderStyle"), 1);
				//				kuhl_geometry_draw(&BFrame_1);
				//				kuhl_geometry_draw(&BFrame_2);
				//				kuhl_geometry_draw(&BFrame_3);
				//				kuhl_geometry_draw(&BFrame_4);
				//				kuhl_geometry_draw(&labelQuad); /* Draw the quad */
				glUniform1i(kuhl_get_uniform("renderStyle"), renderStyle);
				glEnable(GL_DEPTH_TEST);
				//glDisable(GL_DEPTH_CLAMP);

			}


		}

		//glUseProgram(0); // stop using a GLSL program.
		if (preRend) {
			glBindFramebuffer(GL_FRAMEBUFFER, drawBufferID);
			//glUseProgram(0);
			kuhl_errorcheck();
		}
		glUseProgram(0); // stop using a GLSL program.
		if (preRend) {
			//glViewport(0, 0, viewport[2], viewport[3]);
			//viewmat_get(viewMat, perspective, viewportID);
			//viewmat_begin_eye(viewportID);
			kuhl_geometry_draw(&prerendQuad);
		}
		viewmat_end_eye(viewportID);
	} // finish viewport loop

	  /* Update the model for the next frame based on the time. We
	  * convert the time to seconds and then use mod to cause the
	  * animation to repeat. */
	double time = glfwGetTime();
	dgr_setget("time", &time, sizeof(double));
	kuhl_update_model(modelgeom, 0, fmodf((float)time, 10.0f));

	viewmat_end_frame();

	/* Check for errors. If there are errors, consider adding more
	* calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	//kuhl_video_record("videoout", 30);
}


/* Draw a simple quad. */
/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuad(kuhl_geometry *geom, GLuint program) {
	kuhl_geometry_new(geom, program, 4, // number of vertices
		GL_TRIANGLES); // type of thing to draw

					   /* The data that we want to draw */
	GLfloat vertexPositions[] = { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions, 3, // number of components x,y,z
		"in_Position", // GLSL variable
		KG_WARN); // warn if attribute is missing in GLSL program?

	GLfloat texcoord[] = { 0, 0, 1, 0, 1, 1, 0, 1 };
	kuhl_geometry_attrib(geom, texcoord, 2, // number of components x,y,z
		"in_TexCoord", // GLSL variable
		KG_WARN); // warn if attribute is missing in GLSL program?

	GLuint indexData[] = { 0, 1, 2, // first triangle is index 0, 1, and 2 in the list of vertices
		0, 2, 3 }; // indices of second triangle.
	kuhl_geometry_indices(geom, indexData, 6);

	/* The colors of each of the vertices */
	GLfloat colorData_black[] = { 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05 };
	//	if(WHITE_FRAME == 1) {
	GLfloat colorData_white[] = { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 };

	GLfloat colorData_grey[] = { 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02 };
	//	}
	if (WHITE_FRAME == 0) {
		kuhl_geometry_attrib(geom, colorData_black, 3, "in_Color", KG_WARN);
	}
	else if (WHITE_FRAME == 1) {
		kuhl_geometry_attrib(geom, colorData_white, 3, "in_Color", KG_WARN);
		//		printf("the frame is white\n");
	}
	else if (WHITE_FRAME == 2) {
		kuhl_geometry_attrib(geom, colorData_grey, 3, "in_Color", KG_WARN);
	}

	kuhl_errorcheck();
}

int main(int argc, char** argv)
{
	srand(time(NULL));

	/* Initialize GLFW and GLEW */
	kuhl_ogl_init(&argc, argv, 960, 540, 32, 4);


	char *modelFilename = "models/lab-mtu/lab_minification.dae";
	char *modelTexturePath = NULL;


	/* Specify function to call when keys are pressed. */
	glfwSetKeyCallback(kuhl_get_window(), keyboard);
	// glfwSetFramebufferSizeCallback(window, reshape);

	/* Compile and link a GLSL program composed of a vertex shader and
	* a fragment shader. */
	program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);

	//initial prerender shaders
	if (preRend == 1) {
		prerendProgram = kuhl_create_program("distjudge-oval.vert", "distjudge-oval.frag");
	}
	else if (preRend == 2) {
		prerendProgram = kuhl_create_program("distjudge-cube.vert", "distjudge-cube.frag");
	}
	else if (preRend == 3) {
		prerendProgram = kuhl_create_program("distjudge-pixel.vert", "distjudge-pixel.frag");
	}
	else {
		prerendProgram = kuhl_create_program("distjudge-oval.vert", "distjudge-oval.frag");
	}

	init_geometryQuadPrerender(&prerendQuad, prerendProgram);

	dgr_init();     /* Initialize DGR based on environment variables. */

	viewmat_init(initCamPos, initCamLook, initCamUp);

	// Clear the screen while things might be loading
	glClearColor(.2f, .2f, .2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the model from the file
	modelgeom = kuhl_load_model(modelFilename, modelTexturePath, program, bbox);

	// For black frame condition
	init_geometryQuad(&BFrame_1, program);
	init_geometryQuad(&BFrame_2, program);
	init_geometryQuad(&BFrame_3, program);
	init_geometryQuad(&BFrame_4, program);

	// All of the target models.
	// On the Rekhi lab machines, these files are stored in /local/kuhl-public-share/opengl/data/...
	modeltargets[0] = kuhl_load_model("models/targets-bochao/cylinder_green_s.dae", NULL, program, NULL);
	modeltargets[1] = kuhl_load_model("models/targets-bochao/sq_yellow.dae", NULL, program, NULL);
	modeltargets[2] = kuhl_load_model("models/targets-bochao/cross_red.dae", NULL, program, NULL);
	modeltargets[3] = kuhl_load_model("models/targets-bochao/poly_brown.dae", NULL, program, NULL);
	modeltargets[4] = kuhl_load_model("models/targets-bochao/trian_blue_s.dae", NULL, program, NULL);

	// for experiment recording
	if (DISTANCE_EXP) {
		const char *fname = "./results/dist.txt";
		dis_record = fopen(fname, "w");
		if (!dis_record) {
			msg(MSG_FATAL, "Unable to open %s for writing.\n", fname);
			exit(EXIT_FAILURE);
		}
		fprintf(dis_record, "Trail    Target_Distance    Record_Distance    Percentage\n");
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_CLAMP);
	glDepthFunc(GL_LEQUAL); // makes far clamping work.


	while(!glfwWindowShouldClose(kuhl_get_window()))
	{
		display();
		kuhl_errorcheck();

		/* process events (keyboard, mouse, etc) */
		glfwPollEvents();
	}
	exit(EXIT_SUCCESS);
}
