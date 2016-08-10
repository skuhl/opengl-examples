#include <stdlib.h>
#include <GL/glew.h>
#include "dispmode.h"
#include "vecmat.h"
#include "msg.h"
#include "bufferswap.h"

dispmode::dispmode()
{
}

/** Translates a viewportID into a specific eye. For HMD applications, viewportID=0
 * is typically the left eye, it does not necessarily need to be.
 *
 * @param viewportID The viewport ID that we wish to know the eye that it corresponds to.
 *
 * @return The eye that the viewport corresponds to.
 */
viewmat_eye dispmode::eye_type(int viewportID)
{
	return VIEWMAT_EYE_MIDDLE;
}

void dispmode::get_eyeoffset(float offset[3], viewmat_eye eye)
{
	vec3f_set(offset, 0,0,0);
}


/** A wrapper around get_eyeoffset(offset, viewmat_eye eye) which works by converting the viewportID into the eye type.
 *
 * @param offset The eye offset to be filled in.
 * @param viewportID The viewport of the eye that we are requesting.
 */
void dispmode::get_eyeoffset(float offset[3], int viewportID)
{
	this->get_eyeoffset(offset, eye_type(viewportID));
}



/** Returns the number of viewports. For most desktop applications, it
 * will return 1. For stereoscopic rendering (Oculus, anaglyph, etc),
 * this function may return 2.
 *
 * @return The number of viewports that this display mode provides.
 */
int dispmode::num_viewports(void)
{
	return 1;
}

/** Retrives a viewport for the specified viewport ID.

    @param viewportValue Specifies the viewport. The first two values
    are the lower left corner of the viewport rectangle in pixels. The
    last two values specify the width and the height of the viewport.

    @param viewportID The ID for the viewport that we are requesting a
    viewport for.
*/
void dispmode::get_viewport(int viewportValue[4], int viewportId)
{

}

/** Retrieves a view frustum for the specified viewport ID.

    @param result The resulting view frustum (left, right, bottom, top, near far).

    @param viewportID The ID for the viewport that we are requesting a frustum for.
*/
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
	
/** Ideally, dispmode can provide either a frustum (get_frustum()) or
 * a projection matrix (get_projmatrix()). By providing access to the
 * frustum values, it allows us later adjust the frustum values later
 * for applications which require a dynamic frustum (e.g., IVS display
 * wall). However, other systems might not provide a way to access the
 * view frustum and just provides us with a projection matrix
 * (Oculus).
 *
 * @return Returns 1 if this dispmode object can only provide a
 * projection matrix and is unable to provide a frustum with
 * get_frustum.
 */
int dispmode::provides_projmat_only()
{
	return 0;
}

/** Get a projection matrix for a given viewport. By default, this
    function calls get_frustum() and then converts top/bottom/left/etc
    values into a projection matrix. Some display modes may not provide
    access to a frustum and only provide a matrix.

    @param projmatrix The 4x4 projection matrix to be filled in by this function.

    @param viewportID Specifies the ID of the viewport that the caller
    wants a projection matrix for.
*/
void dispmode::get_projmatrix(float projmatrix[16], int viewportID)
{
	float f[6];  // left, right, bottom, top, near>0, far>0
	this->get_frustum(f, viewportID);
	mat4f_frustum_new(projmatrix, f[0], f[1], f[2], f[3], f[4], f[5]);
}

/** To be called prior to drawing a frame. May be necessary for some
 * rendering modes. */
void dispmode::begin_frame()
{

}

/** To be called when done rendering a frame. This function swaps the
 * buffers. In some rendering modes, it may also do additional work. */
void dispmode::end_frame()
{
	bufferswap();
}

/** To be called prior to drawing in a viewport. May be necessary for
 * some rendering modes. */
void dispmode::begin_eye(int viewportID)
{

}

/** To be called when done drawing to a viewport. May be necessary for
 * some rendering modes. */
void dispmode::end_eye(int viewportID)
{

}
