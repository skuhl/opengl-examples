#include "windows-compat.h"
#include <GLFW/glfw3.h>
#include "kuhl-util.h"
#include "mousemove.h"
#include "camcontrol-mouse.h"
#include "vecmat.h"

camcontrolMouse::camcontrolMouse(dispmode *currentDisplayMode, const float pos[3], const float look[3], const float up[3])
	:camcontrol(currentDisplayMode)
{
	GLFWwindow *window = kuhl_get_window();
	glfwSetMouseButtonCallback(window, mousemove_glfwMouseButtonCallback);
	glfwSetCursorPosCallback(window, mousemove_glfwCursorPosCallback);
	glfwSetScrollCallback(window, mousemove_glfwScrollCallback);

	//glutMotionFunc(mousemove_glutMotionFunc);
	//glutMouseFunc(mousemove_glutMouseFunc);
	mousemove_set(pos[0],pos[1],pos[2],
	              look[0],look[1],look[2],
	              up[0],up[1],up[2]);
	mousemove_speed(0.05f, 0.5f);
}

viewmat_eye camcontrolMouse::get_separate(float pos[3], float rot[16], viewmat_eye requestedEye)
{
	float look[3], up[3];
	mousemove_get(pos, look, up);
	mat4f_lookatVec_new(rot, pos, look, up);

	// Translation will be in outPos, not in the rotation matrix.
	float zero[4] = { 0,0,0,1 };
	mat4f_setColumn(rot, zero, 3);

	// Invert matrix because the rotation matrix will be inverted
	// again later.
	mat4f_invert(rot);
	
	return VIEWMAT_EYE_MIDDLE;
}
