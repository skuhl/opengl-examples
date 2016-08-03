#include <GL/glew.h>
#include "bufferswap.h"
#include "msg.h"
#include "viewmat.h"
#include "vecmat.h"
#include "dispmode-anaglyph.h"

dispmodeAnaglyph::dispmodeAnaglyph()
{
}

viewmat_eye dispmodeAnaglyph::eye_type(int viewportID)
{
	if(viewportID == 0)
		return VIEWMAT_EYE_LEFT;
	else if(viewportID == 1)
		return VIEWMAT_EYE_RIGHT;
	else
		return VIEWMAT_EYE_UNKNOWN;
}

int dispmodeAnaglyph::num_viewports(void)
{
	return 2;
}

void dispmodeAnaglyph::get_viewport(int viewportValue[4], int viewportID)
{
	/* Each viewport has half of the screen */
	int windowWidth, windowHeight;
	viewmat_window_size(&windowWidth, &windowHeight);

	/* Our anaglyph rendering uses parallel cameras. Changing the
	 * offset will change the distance at which objects are perceived
	 * to be at the depth of your screen. For example, a star that is
	 * infinitely far away form the parallel cameras would project
	 * onto the same pixel in each camera. However, if we displayed
	 * those two images without an offset, your eyes would have to
	 * verge to the depth of the screen to fuse the star---causing
	 * convergence cues to indicate that the star is at the depth of
	 * the screen. Offsetting the image horizontally by the
	 * inter-pupil eye distances can resolve this problem. Offsetting
	 * too should be avoided because it causes divergence.
	 *
	 * Anaglyph images may not look good because some light will get
	 * to the incorrect eye due to imperfect filters. Also, if you
	 * move the camera very close to an object, may be difficult to
	 * fuse the object. (Objects close to your eyes in the real world
	 * are hard to fuse too!).
	 *
	 * The offset parameter below is in pixels. Depending on the size
	 * the pixels on your screen, you may need to adjust this value.
	 */
	int offset = 20;

	
	if(viewportID == 0)
	{
		viewportValue[0] = -offset/2;   // x,y coords
		viewportValue[1] = 0;
		viewportValue[2] = windowWidth; // width, height
		viewportValue[3] = windowHeight;
	}
	else if(viewportID == 1)
	{
		viewportValue[0] = offset/2;   // x,y coords
		viewportValue[1] = 0;
		viewportValue[2] = windowWidth; // width, height
		viewportValue[3] = windowHeight;
	}
	else
	{
		msg(MSG_ERROR, "Invalid viewportID=%d requested in mode", viewportID);
	}

}

void dispmodeAnaglyph::get_frustum(float result[6], int viewportID)
{
	int viewport[4];
	this->get_viewport(viewport, viewportID);
	int viewportW = viewport[2];
	int viewportH = viewport[3];

	float aspect = viewportW/(float)viewportH;
	float nearPlane = 0.1f;
	float farPlane = 200.0f;
	float vfov = 65.0f;
	float fovyRad = vfov * M_PI/180.0f;
	float height = nearPlane * tanf(fovyRad/2.0f);
	float width = height * aspect;
	result[0] = -width;
	result[1] = width;
	result[2] = -height;
	result[3] = height;
	result[4] = nearPlane;
	result[5] = farPlane;
}


void dispmodeAnaglyph::end_frame()
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	dispmode::end_frame(); // call our parents implementation
}


void dispmodeAnaglyph::begin_eye(int viewportID)
{
	if(viewportID == 0)
		glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
	else if(viewportID == 1)
		glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_FALSE);
	else
		msg(MSG_ERROR, "Invalid viewportID=%d requested in mode", viewportID);

	dispmode::begin_eye(viewportID);
}
