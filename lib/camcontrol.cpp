#include "camcontrol.h"
#include "viewmat.h"
#include "vecmat.h"

camcontrol::camcontrol(dispmode *currentDisplayMode)
{
	displaymode = currentDisplayMode;
	vec3f_set(pos, 0,0,0);
	vec3f_set(look, 0,0,-1);
	vec3f_set(up, 0,1,0);
}

camcontrol::camcontrol(dispmode *currentDisplayMode, const float inPos[3], const float inLook[3], const float inUp[3])
{
	displaymode = currentDisplayMode;
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

	// Translation will be in outPos, not in the rotation matrix.
	float zero[4] = { 0,0,0,1 };
	mat4f_setColumn(outRot, zero, 3);

	// Invert matrix because the rotation matrix will be inverted
	// again later.
	mat4f_invert(outRot);
		
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

	/* Get the eye's position and orientation */
	float pos[3], rot[16];
	viewmat_eye actualEye = this->get_separate(pos, rot, requestedEye);

	/* Create a translation matrix based on the eye position. Note
	 * that the eye position is negated because we are translating the
	 * camera (or, equivalently, translating the world)---not an
	 * object. */
	float trans[16];
	mat4f_translate_new(trans, -pos[0],-pos[1],-pos[2]);

	/* We invert the rotation matrix because we are rotating the camera, not an object. */
	mat4f_transpose(rot);

	/* Combine into a single view matrix. */
	mat4f_mult_mat4f_new(matrix, rot, trans);

	/* Determine if the view matrix needs to be updated to get it to
	 * be appropriate for the requested eye. */
	if(actualEye == VIEWMAT_EYE_MIDDLE &&
	   (requestedEye == VIEWMAT_EYE_LEFT ||
	    requestedEye == VIEWMAT_EYE_RIGHT) )
	{
		/* Update the view matrix based on which eye we are rendering */
		float eyeOffset[3];
		displaymode->get_eyeoffset(eyeOffset, requestedEye);
		//vec3f_print(eyeOffset);
		
		/* Create a new matrix which we will use to translate the
		   existing view matrix. We negate eyeOffset because the
		   matrix would shift the world, not the eye by default. */
		float shiftMatrix[16];
		mat4f_translate_new(shiftMatrix, -eyeOffset[0], -eyeOffset[1], -eyeOffset[2]);
			
		/* Adjust the view matrix by the eye offset */
		mat4f_mult_mat4f_new(matrix, shiftMatrix, matrix);
			
		actualEye = requestedEye;
	}
		
	return actualEye;
}
