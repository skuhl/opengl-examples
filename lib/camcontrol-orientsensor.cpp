#include <stdlib.h>
#include "kuhl-util.h"
#include "camcontrol-orientsensor.h"
#include "vecmat.h"
#include "orient-sensor.h"


camcontrolOrientSensor::camcontrolOrientSensor(const float initialPos[3])
{
	vec3f_copy(position, initialPos);
	orientsense = orient_sensor_init(NULL, ORIENT_SENSOR_NONE);
}


camcontrolOrientSensor::~camcontrolOrientSensor()
{
}

viewmat_eye camcontrolOrientSensor::get_separate(float pos[3], float orient[16], viewmat_eye requestedEye)
{
	vec3f_copy(pos, position);


	float quaternion[4];
	orient_sensor_get(&orientsense, quaternion);
	
	float cyclopsViewMatrix[16];
	mat4f_rotateQuatVec_new(cyclopsViewMatrix, quaternion);
	
	// Fix rotation for a hard-wired orientation sensor.
	float adjustLeft[16] = { 0, 1, 0, 0,
	                         0, 0, 1, 0,
	                         1, 0, 0, 0,
	                         0, 0, 0, 1 };
	mat4f_transpose(adjustLeft); // transpose to column-major order
	
	float adjustRight[16] = { 0, 0, -1, 0,
	                          -1, 0,  0, 0,
	                          0, 1,  0, 0,
	                          0, 0,  0, 1 };
	mat4f_transpose(adjustRight);
	
	mat4f_mult_mat4f_new(orient, adjustLeft, orient);
	mat4f_mult_mat4f_new(orient, orient, adjustRight);

	//mat4f_invert(cyclopsViewMatrix);
	
	return VIEWMAT_EYE_MIDDLE;
}
