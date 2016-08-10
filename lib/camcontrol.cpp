#include "camcontrol.h"
#include "viewmat.h"
#include "vecmat.h"

camcontrol::camcontrol()
{
	float inPos[3] = { 0,0,0 };
	float inLook[3] = {0,0,-1};
	float inUp[3] = {0,1,0};
	vec3f_copy(pos, inPos);
	vec3f_copy(look, inLook);
	vec3f_copy(up, inUp);
}

camcontrol::camcontrol(const float inPos[3], const float inLook[3], const float inUp[3])
{
	vec3f_copy(pos, inPos);
	vec3f_copy(look, inLook);
	vec3f_copy(up, inUp);
}

/** Gets camera position and a rotation matrix for the camera.
    
    @param outPos The position of the camera.
    
    @param outRot A rotation matrix for the camera.

    @param requestedEye Specifies the eye that we wish to get the
    position/orientation of.
    
    @return The eye that the matrix is actually for. In some cases,
    the requestedEye might NOT match the actual eye. For example, the
    mouse movement manipulator might always return VIEWMAT_EYE_MIDDLE
    regardless of which eye was requested---and the caller must update
    it appropriately. Or, you can use the get() function instead of
    this one which provides the logic to apply the appropriate
    correction.
*/
viewmat_eye camcontrol::get_separate(float outPos[3], float outRot[16], viewmat_eye requestedEye)
{
	mat4f_lookatVec_new(outRot, pos, look, up);
	float zero[4] = { 0,0,0,1 };
	mat4f_setColumn(outRot, zero, 3);
		
	vec3f_copy(outPos, pos);
	return VIEWMAT_EYE_MIDDLE;
}

/** Gets a view matrix.
	    
    @param matrix The requested view matrix.
	    
    @param requestedEye The eye that we are requesting.

    @return The eye that the matrix is actually for. In almost all
    cases the returned value should match the requested eye.
*/
viewmat_eye camcontrol::get(float matrix[16], viewmat_eye requestedEye) {
	float pos[3], rot[16], trans[16];
	viewmat_eye actualEye = this->get_separate(pos, rot, requestedEye);
	mat4f_translate_new(trans, -pos[0],-pos[1],-pos[2]); // negate translation because we are translating camera, not an object
	mat4f_transpose(rot); // transpose because we are rotating camera, not an object
	mat4f_mult_mat4f_new(matrix, rot, trans);

	if(actualEye == VIEWMAT_EYE_MIDDLE &&
	   (requestedEye == VIEWMAT_EYE_LEFT ||
	    requestedEye == VIEWMAT_EYE_RIGHT) )
	{
		/* Update the view matrix based on which eye we are rendering */
		float eyeDist = 0.055f;  // TODO: Make this configurable.
		float eyeShift = eyeDist/2.0f;
		if(requestedEye == VIEWMAT_EYE_LEFT)
			eyeShift = eyeShift * -1;
			
		// Negate eyeShift because the matrix would shift the world, not
		// the eye by default.
		float shiftMatrix[16];
		mat4f_translate_new(shiftMatrix, -eyeShift, 0, 0);
			
		/* Adjust the view matrix by the eye offset */
		mat4f_mult_mat4f_new(matrix, shiftMatrix, matrix);
			
		actualEye = requestedEye;
	}
		
	return actualEye;
}
