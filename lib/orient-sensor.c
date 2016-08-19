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

#include "windows-compat.h"
#include "orient-sensor.h"
#include "kuhl-util.h"
#include "serial.h"
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* uint8_t */
#include <time.h>

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
		msg(MSG_FATAL, "Can't connect to orientation sensor because device file is NULL.");
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
		msg(MSG_FATAL, "Can't connect to orientation sensor because sensor type is not set.");
		exit(EXIT_FAILURE);
	}

	char *typeString = NULL;
	switch(sensorType)
	{
		case ORIENT_SENSOR_BNO055: typeString = "bno055"; break;
		case ORIENT_SENSOR_DSIGHT: typeString = "dsight"; break;
		default:                   typeString = "ERROR?!"; break;
	}
		
	msg(MSG_INFO, "Connecting to sensor '%s' at '%s'\n", typeString, deviceFile);

	/* Initialize state struct */
	OrientSensorState state;
	strncpy(state.deviceFile, deviceFile, 32);
	state.deviceFile[31]='\0';
	state.isWorking = 0;
	state.type = sensorType;
	state.lastDataTime = 0;
	for(int i=0; i<4; i++)
		state.lastData[i] = 0.0;

	/* Create connection, apply proper tty settings */
	if(sensorType == ORIENT_SENSOR_BNO055)
	{
		state.fd = serial_open(deviceFile, 115200, 0, 1, 5);
		// we will find the magic byte at the start of a record in our get() function.
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
	static int calibrationMessage = 100;

	int options = SERIAL_NONE;
	if(state->isWorking == 0)
	{
		/* isWorking will be set to 0 in our init function. If a long
		 * time passed between init() and now, our input buffer may
		 * have overflowed. Here, we discard data in the input buffer
		 * and set the options so we will block when reading from the
		 * input buffer. */
		serial_discard(state->fd);
		options = SERIAL_CONSUME;
	}
	else
	{
		/* If connection is working: Consume any extra records in
		 * input buffer. If there are not enough data in input buffer,
		 * use cached data. */
		options = SERIAL_CONSUME|SERIAL_NONBLOCK;
	}
		

	/* Try to read a record, hope that we started reading at the
	 * beginning of a record */
	char temp[RECORD_SIZE];
	temp[0] = '\0'; // make sure temp doesn't happen to contain magic byte.
	if(serial_read(state->fd, temp, RECORD_SIZE, options) == 0)
	{
		// If SERIAL_NONBLOCK was used *and* if there wasn't enough
		// bytes to read a full record, use cached data. Note:
		// serial_read() can never return 0 except when the
		// SERIAL_NONBLOCK setting is used.

		/* Check how old the cached data is. If it is old, we have
		 * been using the same cached data for too long. */
		if(time(NULL) - state->lastDataTime >= 2)
		{
			msg(MSG_WARNING, "We haven't received a new record from the orientation sensor in the past couple seconds. Is sensor still connected? Trying to reconnect.");
		}
		else
		{
			for(int i=0; i<4; i++)
				quaternion[i] = state->lastData[i];
			// msg(MSG_INFO, "Using cached data for orientation sensor.\n");
			return;
		}
	}


	/* Look for magic bytes at beginning of record */
	int32_t v = 0x42f6e979; // hex for the 123.456 float sent from arduino	
	while(memcmp(temp, &v, 4) != 0)
	{
		/* While we are here, the first bytes of the record didn't
		 * match the magic bytes we were expecting. This can happen if
		 * the sender overwhelmed our buffer or if there was a problem
		 * with the sensor. */
		if(state->isWorking) // if the sensor was previously working, print message
		{
			uint32_t received;
			memcpy(&received, temp, 4);
			msg(MSG_WARNING, "Synchronizing to orientation sensor stream (may block if we can't read from sensor)...");
			msg(MSG_DEBUG,   "Synchronizing because we expected 0x%08x but  received 0x%08x", v, received);
		}
		state->isWorking = 0;
		serial_discard(state->fd); // clear input buffer in case it
								   // got filled up. Therefore, the
								   // find and read calls below WILL
								   // BLOCK until we get new data.

		/* Try to find the magic bytes somewhere in the stream of data. */
		if(serial_find(state->fd, (char*) &v, 4, 1000) == 1)
		{
			/* If we found the magic bytes, copy them into our buffer
			 * and read the rest of the record. */
			if(serial_read(state->fd, temp+4, RECORD_SIZE-4, SERIAL_NONE) == RECORD_SIZE-4)
				memcpy(temp, &v, 4);
		}
		else
		{
			/* If we didn't find the bytes, something more serious may
			 * have went wrong. */
			msg(MSG_ERROR, "Failed to resynchronize to orientation sensor. Trying to reconnect.");
			serial_close(state->fd);
			*state = orient_sensor_init(state->deviceFile, state->type);
			serial_read(state->fd, temp, RECORD_SIZE, SERIAL_CONSUME);
		}

	} // end while magic byte is wrong.

	// If we get here, we successfully synchronized...
	if(state->isWorking == 0)
	{
		msg(MSG_INFO, "Successfully synchronized to orientation sensor.\n");
		state->isWorking = 1;
	}
	state->lastDataTime = time(NULL);
	// msg(MSG_GREEN, "Record OK");

	uint8_t sys, gyro, accel, mag;
	sys    = temp[4*5+0];
	gyro   = temp[4*5+1];
	accel  = temp[4*5+2];
	mag    = temp[4*5+3];
	calibrationMessage--;
	if(calibrationMessage < 0)
	{
		calibrationMessage = 1000;
		
		if(sys == 0)
			msg(MSG_ERROR, "Sensor is uncalibrated.");
		else if (sys == 1)
			msg(MSG_WARNING, "Sensor calibration is poor.");

		if(gyro == 0)
			msg(MSG_WARNING, "Gyro is uncalibrated. Let sensor sit still.");
		else if(gyro == 1)
			msg(MSG_WARNING, "Gyro calibration is poor. Let sensor sit still.");

		if(accel == 0)
			msg(MSG_WARNING, "Accelerometer is uncalibrated. Place sensor on 6 sides of block.");
		else if(accel == 1)
			msg(MSG_WARNING, "Accelerometer calibration is poor. Place sensor on 6 sides of block.");
		
		if(mag == 0)
			msg(MSG_WARNING, "Magnetometer is uncalibrated. Use figure 8 motion.");
		else if(mag == 1)
			msg(MSG_WARNING, "Magnetometer calibration is poor. Use figure 8 motion.");

		if(sys < 2 || gyro < 2 || accel < 2 || mag < 2)
			msg(MSG_BLUE, "Raw orientation sensor calib data: sys=%d gyro=%d accel=%d mag=%d", sys, gyro, accel, mag);
	}
	// msg(MSG_INFO, "sys=%d gyro=%d accel=%d mag=%d", sys, gyro, accel, mag);

	/* Copy data from our buffer into quaternion buffer and into the lastData buffer */
	memcpy(quaternion, temp+4, sizeof(float)*4);
	memcpy(state->lastData, quaternion, sizeof(float)*4);
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
