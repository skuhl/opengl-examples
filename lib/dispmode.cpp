#include <stdlib.h>
#include <GL/glew.h>
#include "dispmode.h"
#include "msg.h"
#include "bufferswap.h"

dispmode::dispmode()
{
}

viewmat_eye dispmode::eye_type(int viewportID)
{
	return VIEWMAT_EYE_UNKNOWN;
}


int dispmode::num_viewports(void)
{
	return 1;
}

void dispmode::get_viewport(int viewportValue[4], int viewportId)
{

}

void dispmode::get_frustum(float result[6], int viewportID)
{

}



/** If we are rendering a scene for the Oculus, we will be rendering
 * into a multisampled FBO that we are not allowed to read from. We
 * can only read after the multisampled FBO is blitted into a normal
 * FBO inside of viewmat_end_frame(). This function will retrieve the
 * binding for the normal framebuffer. The framebuffer will be from
 * the previous frame unless it is called after
 * viewmat_end_frame().
 *
 * In all cases besides Oculus, you can retrieve the current
 * framebuffer the normal way (or use this function).
 *
 * @param viewportID The viewport that we want a framebuffer for.
 * @return An OpenGL framebuffer that we can bind to.
 */
int dispmode::get_framebuffer(int viewportID)
{
	GLint framebuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING,  &framebuffer);
	return framebuffer;
}
	
/* Ideally, dispmode returns a view frustum. This allows us to
 * further adjust the frustum for systems such as those that
 * provide a dynamic frustum (e.g., IVS display wall). However,
 * other systems might not provide a way to access the view
 * frustum and just provides us with a projection matrix (Oculus).
 */
int dispmode::provides_projmat_only()
{
	return 0;
}


void dispmode::get_projmatrix(float projmatrix[16], int viewportID)
{
	float f[6];  // left, right, bottom, top, near>0, far>0
	this->get_frustum(f, viewportID);
	mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
}

void dispmode::begin_frame()
{

}


void dispmode::end_frame()
{
	bufferswap();
}


void dispmode::begin_eye(int viewportID)
{

}
