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
		deviceFile = strdup(getenv("ORIENT_SENSOR_TTY"));

	if(deviceFile == NULL)
	{
		msg(FATAL, "Can't connect to orientation sensor because device file is NULL.");
		exit(EXIT_FAILURE);
	}

	if(sensorType == ORIENT_SENSOR_NONE)
	{
		const char *type = getenv("ORIENT_SENSOR_TYPE");
		if(strcasecmp(type, "bno055") == 0)
			sensorType = ORIENT_SENSOR_BNO055;
		else if(strcasecmp(type, "dsight") == 0)
			sensorType = ORIENT_SENSOR_DSIGHT;
	}

	if(sensorType == ORIENT_SENSOR_NONE)
	{
		msg(FATAL, "Can't connect to orientation sensor because sensor type is not set.");
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
		serial_discard(state.fd);
		msg(DEBUG, "Waiting for the start of a record from the orientation sensor...");
		int32_t v = 0x42f6e979; // hex for the 123.456 float sent from arduino
		if(serial_find(state.fd, (char*) &v, 4, 10000) == 0)
		{
			msg(FATAL, "Failed to find start of a record from sensor.");
			exit(EXIT_FAILURE);
		}
		// read the rest of the bytes so we finish at the start of a new record.
		char buf[4*4+4];
		serial_read(state.fd, buf, 4*4+4, SERIAL_NONE);
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
#if 0
		/* Print a few more values out after the one that didn't match what we expected */
		for(int i=0; i<20; i++)
		{
			float val=0;
			serial_read(state->fd, (char*)&val, sizeof(float), SERIAL_NONE);
			printf("%f %x\n", val, *(unsigned int*)&val);
		}
#endif
		
		msg(WARNING, "Received unexpected data (expected %f, received %f); reconnecting...\n", 123.456, first);
		serial_discard(state->fd);
		serial_close(state->fd);
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
			msg(WARNING, "Gyro is uncalibrated. Let sensor sit still.");
		else if(gyro == 1)
			msg(WARNING, "Gyro calibration is poor. Let sensor sit still.");

		if(accel == 0)
			msg(WARNING, "Accelerometer is uncalibrated. Place sensor on 6 sides of block.");
		else if(accel == 1)
			msg(WARNING, "Accelerometer calibration is poor. Place sensor on 6 sides of block.");
		
		if(mag == 0)
			msg(WARNING, "Magnetometer is uncalibrated. Use figure 8 motion.");
		else if(mag == 1)
			msg(WARNING, "Magnetometer calibration is poor. Use figure 8 motion.");

		if(sys < 2 || gyro < 2 || accel < 2 || mag < 2)
			msg(BLUE, "Raw orientation sensor calib data: sys=%d gyro=%d accel=%d mag=%d", sys, gyro, accel, mag);

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
