#include <stdlib.h>
#include "kuhl-util.h"
#include "camcontrol-orientsensor.h"
#include "vecmat.h"
#include "orient-sensor.h"


camcontrolOrientSensor::camcontrolOrientSensor(dispmode *currentDisplayMode, const float initialPos[3])
	:camcontrol(currentDisplayMode)
{
	vec3f_copy(position, initialPos);

	int orientSensorType = ORIENT_SENSOR_NONE;
	const char *typeString = kuhl_config_get("orientsensor.type");
	if(typeString != NULL)
	{
		if(strcasecmp(typeString, "bno055") == 0)
			orientSensorType = ORIENT_SENSOR_BNO055;
		else if(strcasecmp(typeString, "dsight") == 0)
			orientSensorType = ORIENT_SENSOR_DSIGHT;
		else
		{
			msg(MSG_FATAL, "Unknown orientation sensor type: %s\n", typeString);
			exit(EXIT_FAILURE);
		}
	}

	orientsense = orient_sensor_init(kuhl_config_get("orientsensor.tty"),
	                                 orientSensorType);
}


camcontrolOrientSensor::~camcontrolOrientSensor()
{
}

viewmat_eye camcontrolOrientSensor::get_separate(float pos[3], float orient[16], viewmat_eye requestedEye)
{
	vec3f_copy(pos, position); // use default position
	mat4f_identity(orient); // initialize orientation matrix

	// Retrieve quaternion from sensor, convert it into a matrix.
	float quaternion[4];
	orient_sensor_get(&orientsense, quaternion);
	mat4f_rotateQuatVec_new(orient, quaternion);

	// Correct rotation
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
	//vec3f_print(pos);
	//mat4f_print(orient);

	return VIEWMAT_EYE_MIDDLE;
}
