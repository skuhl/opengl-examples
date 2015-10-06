/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 *
 * This file provides a way to interact with the YEI orientation
 * sensor that use used by the Sensics dSight HMD.
 *
 * @author Evan Hauck
 * @author Scott Kuhl
 */

#include "orient-sensor.h"
#include "kuhl-util.h"
#include "serial.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


/** Opens a connection to the orientation sensor.

    @param deviceFile The serial device to communicate with. For example, /dev/ttyACM0
*/
OrientSensorState orient_sensor_init(const char* deviceFileIn, int sensorType)
{
	const char *deviceFile;
	if(deviceFileIn != NULL)
		deviceFile = deviceFileIn;
	else
		deviceFile = getenv("ORIENT_SENSOR_TTY");

	if(deviceFile == NULL)
	{
		msg(FATAL, "Can't connect to orientation sensor because device file is NULL.");
		exit(EXIT_FAILURE);
	}

	if(sensorType == ORIENT_SENSOR_NONE)
	{
		const char *type = getenv("ORIENT_SENSOR_TYPE");
		if(strcasecmp(type, "bno005") == 0)
			sensorType = ORIENT_SENSOR_BNO055;
		else if(strcasecmp(type, "dsight") == 0)
			sensorType = ORIENT_SENSOR_DSIGHT;
	}

	if(sensorType == ORIENT_SENSOR_NONE)
	{
		msg(FATAL, "Can't connect to orientation sensor because sensor tyepe is not set.");
		exit(EXIT_FAILURE);
	}

	char *typeString = NULL;
	switch(sensorType)
	{
		case ORIENT_SENSOR_BNO055: typeString = "bno055"; break;
		case ORIENT_SENSOR_DSIGHT: typeString = "dsight"; break;
		default:                   typeString = "ERROR?!"; break;
	}
		
	msg(INFO, "Connecting to sensor '%s' at '%s'\n", typeString, deviceFile);

	/* Initialize state struct */
	OrientSensorState state;
	strncpy(state.deviceFile, deviceFile, 32);
	state.deviceFile[31]='\0';
	state.isWorking = 0;
	state.type = sensorType;
	for(int i=0; i<4; i++)
		state.lastData[i] = 0.0;

	/* Create connection, apply proper tty settings */
	if(sensorType == ORIENT_SENSOR_BNO055)
	{
		state.fd = serial_open(deviceFile, 115200, 0, 1, 5);
		// This sleep makes initial connection more reliable
		usleep(100000); // 1/10th second
		serial_discard(state.fd);
	}
	else
		state.fd = serial_open(deviceFile, 115200, 0, 1, 5);
	
	return state;
}



static void orient_sensor_get_dsight(OrientSensorState *state, float quaternion[4])
{

}

static void orient_sensor_get_bno055(OrientSensorState *state, float quaternion[4])
{
// 1 sanity check float, 4 floats for quat, 4 more bytes for calibration data
#define RECORD_SIZE 4+4*4+4
	char temp[RECORD_SIZE];
	static int calibrationMessage = 100;
	
	
	int options = SERIAL_CONSUME;
	if(state->isWorking == 1)
		options = SERIAL_CONSUME|SERIAL_NONBLOCK;

	if(serial_read(state->fd, temp, RECORD_SIZE, options) == 0)
	{
		// We can only get here if SERIAL_NONBLOCK is used *and* if
		// there wasn't enough bytes ready for us to read.
		for(int i=0; i<4; i++)
			quaternion[i] = state->lastData[i];
		// msg(INFO, "Cached data\n");
		return;
	}
	else
	{
		// msg(INFO, "New data\n");
	}
	
	float first = 0;
	memcpy(&first, temp, sizeof(float));
	while(fabs(first-123.456) > .0001)
	{
		msg(WARNING, "Received unexpected data (expected %f, received %f); reconnecting...\n", 123.456, first);
		serial_close(state->fd);
		sleep(1);
		state->isWorking = 0;
		*state = orient_sensor_init(state->deviceFile, state->type);
		serial_read(state->fd, temp, RECORD_SIZE, SERIAL_CONSUME);
	}

	uint8_t sys, gyro, accel, mag;
	sys    = temp[4*5+0];
	gyro   = temp[4*5+1];
	accel  = temp[4*5+2];
	mag    = temp[4*5+3];
	calibrationMessage--;
	if(calibrationMessage < 0)
	{
		if(sys == 0)
			msg(ERROR, "Sensor is uncalibrated.");
		else if (sys == 1)
			msg(WARNING, "Sensor calibration is poor.");

		if(gyro == 0)
			msg(ERROR, "Gyro is uncalibrated. Let sensor sit still.");
		else if(gyro == 1)
			msg(WARNING, "Gyro calibration is poor. Let sensor sit still.");

		if(accel == 0)
			msg(ERROR, "Accelerometer is uncalibrated. Place sensor on 6 sides of block.");
		else if(accel == 1)
			msg(WARNING, "Accelerometer calibration is poor. Place sensor on 6 sides of block.");
		
		if(mag == 0)
			msg(ERROR, "Magnetometer is uncalibrated. Use figure 8 motion.");
		else if(mag == 1)
			msg(WARNING, "Magnetometer calibration is poor. Use figure 8 motion.");

		calibrationMessage = 1000;
	}
	// msg(INFO, "sys=%d gyro=%d accel=%d mag=%d", sys, gyro, accel, mag);
	
	
	// If we get here, everything seems to be working.
	state->isWorking = 1;

	for(int i=0; i<4; i++)
	{
		memcpy(quaternion+i, temp+(i+1)*4, sizeof(float));
		state->lastData[i] = quaternion[i];
	}
}




/** Retrieve the latest orientation from the sensor.

    @param state A OrientSensorState struct created by orient_sensor_init()
    @param quaternion The quaternion retrieved from the sensor.
*/
void orient_sensor_get(OrientSensorState *state, float quaternion[4])
{
	switch(state->type)
	{
		case ORIENT_SENSOR_BNO055:
			orient_sensor_get_bno055(state, quaternion);
			break;
		case ORIENT_SENSOR_DSIGHT:
			orient_sensor_get_dsight(state, quaternion);
	}
}
