/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 *
 * Mousemove provides basic mouse functionality so a user can drag the
 * mouse to navigate a 3D scene. Dragging while pressing the left
 * button allows for up/down/left/right translation. Dragging while
 * pressing the middle mouse button (or both at the same time) will
 * translate the camera forward/back. Dragging while pressing the
 * right button will rotate the camera in place. Mousemove does not
 * allow you to roll the camera. In other words, the camera "up"
 * vector always points in the same direction.
 *
 * Using mousemove involves two main steps:
 *
 * 1) Initialize the starting location and speed of mousemove when the
 *    program begins.
 *
 *    mousemove_set( 0, 0,10, // initial position of the camera<br>
 *                   0, 0, 0, // initial point camera is looking at<br>
 *                   0, 1, 0); // up vector<br>
 *    mousemove_speed(0.05, 0.5);
 *
 * 2) Set up the appropriate callbacks:
 *
 *    glfwSetMouseButtonCallback(window, mousemove_glfwMouseButtonCallback);<br>
 *    glfwSetCursorPosCallback(window, mousemove_glfwCursorPosCallback);<br>
 *    glfwSetScrollCallback(window, mousemove_glfwScrollCallback);
 *
 * 3) Get the camera position/orientation whenever you draw a scene.
 *
 *    float pos[3],look[3],up[3];<br>
 *    mousemove_get(pos, look, up);<br>
 *    gluLookAt(pos[0],pos[1],pos[2],<br>
 *              look[0],look[1],look[2],<br>
 *              up[0],up[1],up[2]);
 *
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <GLFW/glfw3.h>


void mousemove_glfwCursorPosCallback(GLFWwindow *window, double x, double y);
void mousemove_glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void mousemove_glfwScrollCallback(GLFWwindow *window, double xoff, double yoff);

/* Don't need to be called directly if you are using the GLUT or GLFW callback functions listed above. */
void mousemove_buttonPress(int down, int button, int x, int y);
int mousemove_movement(int x, int y);


void mousemove_setVec(const float position[3], const float lookAt[3], const float up[3]);
void mousemove_set(float posX, float posY, float posZ,
                   float lookX, float lookY, float lookZ,
                   float upX, float upY, float upZ);
void mousemove_get(float position[3], float lookAt[3], float up[3]);
void mousemove_speed(float translationSpeed, float rotationSpeed);

#ifdef __cplusplus
} // end extern "C"
#endif
