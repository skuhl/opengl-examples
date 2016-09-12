#include "windows-compat.h"
#include "kalman.h"
#include "vecmat.h"


static float sensorfuse_getYaw(const float matrix[16])
{
	float euler[3];
	eulerf_from_mat4f(euler, matrix, "XZY");
	return euler[2];
}


/** Given orientation from a sensor that has drive in yaw and another
 * orientation from a less-smooth but non-drifting source, combine the
 * data into an orientation that is both smooth and matches the yaw of
 * the non-drifting source.
 */
void sensorfuse(float corrected[16], const float drifting[16], const float stable[16])
{
	float yawDrift = sensorfuse_getYaw(drifting);
	float yawStable = sensorfuse_getYaw(stable);
	
	static float yawDriftPrev = 0.0f;
	static float yawStablePrev = 0.0f;

	/* Adding 360 degrees to yaw results in no change. If we use the
	   raw yaw values, this also means that if yaw is 359 and
	   increasing, it will jump down to 0. Here, we detect if yaw
	   changed significantly from the previous frame and add/subtract
	   360 to get it so that there aren't discontinuities in our yaw.
	*/
	while(yawStable < yawStablePrev-270.0f)
		yawStable += 360.0f;
	while(yawStable > yawStablePrev+270.0f)
		yawStable -= 360.0f;
	while(yawDrift < yawDriftPrev-270.0f)
		yawDrift += 360.0f;
	while(yawDrift > yawDriftPrev+270.0f)
		yawDrift -= 360.0f;

	// The difference in yaw
	float offsetAngle = yawDrift - yawStable;

	/* If the stable tracking system (i.e., Vicon) returns the exact
	 * same value twice in a row, it is likely caused by the system
	 * losing the orientation of the object. In this case, we will
	 * effectively ignore the stable data. */
	int ignoreStable = 0;
	if(yawStable == yawStablePrev)
		ignoreStable = 1;

	/* Update our record of the previous values */
	yawDriftPrev = yawDrift;
	yawStablePrev = yawStable;
	
	static int count = 0;
	// If we received a good record, increment
	if (ignoreStable == 0)
		count++;
	
	static kalman_state kalman;
	static float offsetAngleFiltered = 0; // filtered version of the offset angle

	if(count == 0 || count == 1)
		kalman_initialize(&kalman, 20.0f, .000000001f);
	if(count < 30)
		offsetAngleFiltered = offsetAngle;
	else if(count == 30)
	{
		msg(MSG_GREEN, "Sensor fusion is now active.");
		// reinitialize after sensors have settled down
		offsetAngleFiltered = offsetAngle;
		kalman_initialize(&kalman, 20.0f, .000000001f);
		kalman.xk_prev[0] = offsetAngle;
	}

	// Since angle wraps around every 360 degrees, try to use an
	// equivalent angle that is close to our filtered value.
	while(offsetAngle < offsetAngleFiltered - 270.0f)
		offsetAngle += 360.0f;
	while(offsetAngle > offsetAngleFiltered + 270.0f)
		offsetAngle -= 360.0f;

	if(ignoreStable == 1)
		kalman.predictOnly = 1;
	else
		kalman.predictOnly = 0;
	//offsetAngleFiltered = kalman_estimate(&kalman, offsetAngle, count*16000);
	offsetAngleFiltered = kalman_estimate(&kalman, offsetAngle, -1);

	// Filtered Oculus data.
	float correction[16];
	mat4f_rotateAxis_new(correction, -offsetAngleFiltered, 0,1,0);
	mat4f_mult_mat4f_new(corrected, correction, drifting);



#if 0
	// Debugging

	// Note that the vicon data may be repeated more than expected because we are calling sensorfuse once per eye instead of once per frame.
	msg(MSG_INFO, "sensorfuse: count: %d; viconLost %d", count, ignoreStable);
	msg(MSG_INFO, "sensorfuse: filtered offset: %f", offsetAngleFiltered);
	msg(MSG_INFO, "sensorfuse: current offset: %f", offsetAngle);
	float yawCorrected = sensorfuse_getYaw(corrected);
	msg(MSG_INFO, "sensorfuse: current error: %f", yawCorrected-yawStable);
#endif
}
