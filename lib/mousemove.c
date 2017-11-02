/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include "windows-compat.h"
#include "mousemove.h"

#include <math.h>
#include <stdio.h>

#include "kuhl-util.h"
#include "vecmat.h"


#define EPSILON 0.0001

/** Current camera lookat *point*. A lookat vector is created by
 * subtracting the lookat point from the camera position. */
static float cam_lookat[3];
/** Current camera position */
static float cam_position[3]; 
/** Current camera up vector. Since mousemove does not support roll,
 * this value is only changed when a user specifically sets it with
 * mousemove_set() or mousemove_setVec() */
static float cam_up[3] = { 0.0f, 1.0f, 0.0f };


static float settings_rot_scale = 0.5f;  /**< amount to scale rotations  */
static float settings_trans_scale = .01f;/**< amount to scale translations */

/** Currently pressed mouse button. -1=no button pressed, 0=left,
 * 1=middle, 2=right */
static int cur_button = -1; 
static int last_x; /**< Last X coordinate of the mouse cursor */
static int last_y; /**< Last Y coordinate of the mouse cursor */
static float cam_lookat_down[3]; /**< The lookat vector when the mouse button was last pressed down */
static float cam_position_down[3]; /**< The camera position when the mouse button was last pressed down */

/** Internal function to move camera along the look at vector
 * @param dy Amount to translate camera down the lookVec
 * @param lookVec Vector pointing where camera is looking
 */
void mousemove_translate_inout(int dy, const float lookVec[3]){
	// Move the camera and lookat point along the look vector
	for(int i=0; i<3; i++)
	{
		float offset = lookVec[i] * -dy * settings_trans_scale;
		cam_position[i] = cam_position_down[i] - offset;
		cam_lookat[i]   = cam_lookat_down[i]   - offset;
	}
}


/** Set the speed of rotation and translations when mouse movement is used.
 *
 * @param translationSpeed Speed of translation (0.05 is a good value to start with)
 * @param rotationSpeed Speed of rotation (0.5 is a good value to start with)
 */
void mousemove_speed(float translationSpeed, float rotationSpeed)
{
	settings_trans_scale = translationSpeed;
	settings_rot_scale   = rotationSpeed;
}

/** Gets the currently used camera position, look at point, and up vector.
 *
 * @param position To be filled with the viewpoint position.
 * @param lookAt To be filled with a point that the viewer is looking at.
 * @param up To be filled with an up vector.
 */
void mousemove_get(float position[3], float lookAt[3], float up[3])
{
	vec3f_copy(position, cam_position);
	vec3f_copy(lookAt, cam_lookat);
	vec3f_copy(up, cam_up);
}

/** Sets the currently used camera position, look at point, and up
 * vector. This function is typically called at the beginning of a
 * program to initialize the location that the camera should be at.
 *
 * @param position The position to place the camera.
 * @param lookAt The point that the camera is pointing at.
 * @param up The camera's up vector.
 * @see mousemove_set()
 */    
void mousemove_setVec(const float position[3], const float lookAt[3], const float up[3])
{
	vec3f_copy(cam_position, position);
	vec3f_copy(cam_lookat, lookAt);
	vec3f_copy(cam_up, up);
}

/** Sets the currently used camera position, look at point, and up
 * vector. Note that the look variables are not a vector---but are a
 * point that the camera should be pointing at. This function is
 * typically called at the beginning of a program to initialize the
 * location that the camera should be at.
 *
 * @param posX The X position to place the camera.
 * @param posY The Y position to place the camera.
 * @param posZ The Z position to place the camera.
 * @param lookX The X component of a point that the camera is looking at.
 * @param lookY The Y component of a point that the camera is looking at.
 * @param lookZ The Z component of a point that the camera is looking at.
 * @param upX The X component of the camera up vector.
 * @param upY The Y component of the camera up vector.
 * @param upZ The Z component of the camera up vector.
 * @see mousemove_setVec()
 */
void mousemove_set(float posX, float posY, float posZ,
                   float lookX, float lookY, float lookZ,
                   float upX, float upY, float upZ)
{
	vec3f_set(cam_position, posX, posY, posZ);
	vec3f_set(cam_lookat, lookX, lookY, lookZ);
	vec3f_set(cam_up, upX, upY, upZ);
}

/** Creates a rotation matrix and multiplies a point by the rotation
 * matrix in place.
 *
 * @param degrees Number of degrees of rotation in the rotation matrix.
 *
 * @param axis The axis to rotate around.
 *
 * @param point The point that should be rotated and the location to
 * store the result in.
 */
static void mousemove_private_rotate_point(float degrees, float axis[3], float point[3])
{
	if(fabs(degrees) < EPSILON)
		return;

	float m[9];
	mat3f_rotateAxisVec_new(m,degrees,axis);
	mat3f_mult_vec3f_new(point, m, point);
}



/** This function should be called whenever a mouse button is pressed
 * and is automatically called by mousemove_glfwMouseButtonCallback()
 * and mousemove_glutMouseFunc()
 *
 * @param down 1 if the button is being pressed down or 0 if the button is being released.
 * @param leftMidRight 0=left button is pressed, 1=middle button is pressed, 2=right button is pressed, 3=scoll up, 4=scroll down, -1 no button is pressed.
 * @param x The x coordinate of the mouse cursor when the button is pressed (or amount to scroll horizontally if scrolling event).
 * @param y The y coordinate of the mouse cursor when the button is pressed (or amount to scroll vertically if scrolling event).
 */
void mousemove_buttonPress(int down, int leftMidRight, int x, int y)
{
	if(down)
	{
		cur_button = leftMidRight;

		last_x = x;
		last_y = y;

		// Store camera position & lookat when mouse button is pressed
		vec3f_copy(cam_lookat_down, cam_lookat);
		vec3f_copy(cam_position_down, cam_position);
		if(leftMidRight>2)
		{
			float lookAt[3];
			// Calculate a new vector pointing from the camera to the
			// look at point and normalize it.
			vec3f_sub_new(lookAt,cam_lookat_down,cam_position_down);
			if(cur_button == 3) // scroll up (zoom in)
				mousemove_translate_inout(y,lookAt);
			else // cur_button = 4
				mousemove_translate_inout(y,lookAt);
		}
	}
	else
		cur_button = -1;
}

/** This function should be called whenever a mouse cursor motion
 * occurs and is automatically called by
 * mousemove_glfwCursorPosCallback() and mousemove_glutMotionFunc()
 *
 * @param x The x coordinate of the mouse cursor.
 * @param y The y coordinate of the mouse cursor.
 * @return 1 if the scene needs to be redawn, 0 otherwise.
 */
int mousemove_movement(int x, int y)
{
	if(cur_button == -1)
		return 0;

	/* Distance cursor has moved since last press. */
	int dx = x-last_x;
	int dy = y-last_y;
	/* Vectors to store our orthonormal basis. */
	float f[3], r[3], u[3];

	// Calculate a new vector pointing from the camera to the
	// look at point and normalize it.
	vec3f_sub_new(f,cam_lookat_down,cam_position_down);
	vec3f_normalize(f);

	// Get our up vector
	vec3f_copy(u,cam_up);

	// Get a right vector based on the up and lookat vector.
	vec3f_cross_new(r, f, u);

	// If right vector was short, then look vector was pointing in
	// nearly the same direction as the up vector.
	if(vec3f_normSq(r) < EPSILON)
	{
		//printf("mousemove: whoops, pointed camera at up vector.");
		// move the up vector slightly and try again:
		u[0] += 0.05f;
		vec3f_cross_new(r,f,u);
	}
	vec3f_normalize(r);

	// recalculate the up vector from the right vector and up vector
	// to ensure we have a an orthonormal basis.
	vec3f_cross_new(u, r, f);
	vec3f_normalize(u);

	switch(cur_button)
	{
		case 0: // left mouse button - translate left/right or up/down

			/* Translate the camera along the right and up vectors
			 * appropriately depending on the type of mouse movement.  */
			for(int i=0; i<3; i++)
			{
				float offset = r[i]*dx*settings_trans_scale + -u[i]*dy*settings_trans_scale;
				cam_position[i] = cam_position_down[i] - offset;
				cam_lookat[i]   = cam_lookat_down[i]   - offset;
			}
			break;

		case 1: // middle mouse button - translate forward and back
			mousemove_translate_inout(dy, f);
			break;

		case 2: // right mouse button - rotate

			// If the mouse is moved left/right, rotate the facing
			// vector around the up vector.
			mousemove_private_rotate_point(dx*settings_rot_scale, u, f);
			// If the mouse is moved up/down, rotate the facing vector
			// around the right vector.
			mousemove_private_rotate_point(dy*settings_rot_scale, r, f);
			vec3f_normalize(f);

			// Add new facing vector to the position that the camera
			// was at when the button was first pressed to get a new
			// lookat point.
			vec3f_add_new(cam_lookat, cam_position_down, f);
			break;
	}

	return 1;
}



/** A callback function suitable for use with
 * glfwSetMouseButtonCallback(). Typically, you would call
 * glftSetMouseButtonCallback(window, mousemove_glfwMouseButtonCallback) to
 * tell GLFW to call this function whenever a mouse button is
 * pressed. 
 *
 * @see mousemove_buttonPress() mousemove_glutMouseFunc() 
 */
void mousemove_glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	double x,y;
	glfwGetCursorPos(window, &x, &y);

	int leftMidRight = -1;
	switch(button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:   leftMidRight = 0;  break;
		case GLFW_MOUSE_BUTTON_MIDDLE: leftMidRight = 1;  break;
		case GLFW_MOUSE_BUTTON_RIGHT:  leftMidRight = 2;  break;
		default:                       leftMidRight = -1; break;
	}
	mousemove_buttonPress(action==GLFW_PRESS, leftMidRight, (int)x, (int)y);
}


/** A callback function suitable for use with
 *  glfwSetCursorPosCallback(). Typically, you would call
 *  glfwSetCursorPosCallback(window, mousemove_glfwCursorPosCallback) to tell
 *  GLFW to call this function whenever a mouse is moved while a mouse
 *  button is pressed.
 *
 * @param window The GLFW window we are working with.
 * @param x The x coordinate of the mouse cursor.
 * @param y The y coordinate of the mouse cursor.
 *
 * @see mousemove_movement() mousemove_glutMotionFunc()
 */
void mousemove_glfwCursorPosCallback(GLFWwindow *window, double x, double y)
{
	mousemove_movement(x,y);
}

/** A callback function suitable for use with
 * glfwSetScrollCallback(). Typically, you would call
 * glfwSetScrollCallback(window, mousemove_glfwCursorPosCallback) to
 * tell GLFW to call this function whenever a scroll event (mouse
 * scroll wheel, touchpad scroll, etc) occurs.
 *
 * @param window The GLFW window we are working with.
 * @param xoff The change in x (0 if scroll is vertical)
 * @param yoff The change in y (0 if scroll is horizontal)
 */
void mousemove_glfwScrollCallback(GLFWwindow *window, double xoff, double yoff)
{
	// msg(MSG_INFO, "glfw scroll %f %f\n", xoff, yoff);
	if(yoff > 0)
		mousemove_buttonPress(1, 3, 0, (int)(yoff*10));
	else if(yoff < 0)
		mousemove_buttonPress(1, 4, 0, (int)(yoff*10));
}
