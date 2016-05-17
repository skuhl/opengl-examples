/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
* @author Evan Hauck
*/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/** This enum is used by some serial related functions */
enum
{
	ORIENT_SENSOR_NONE      = 0,  /**< No options */
	ORIENT_SENSOR_BNO055    = 1,
	ORIENT_SENSOR_DSIGHT    = 2
};

typedef struct
{
	int fd;
	char deviceFile[32]; /**< Name of the serial device (/dev/ttyUSB0) */
	float lastData[4]; /**< The last piece of data we received. Useful if we want to use cached data when there isn't new data to read. */
	int lastDataTime; /**< What time did we receive the data in lastData? */
	int isWorking; /**< Set to 1 when we have successfully received data */
	int type;
} OrientSensorState;

	
OrientSensorState orient_sensor_init(const char* deviceFile, int sensorType);
void orient_sensor_get(OrientSensorState *state, float quaternion[4]);

	
#ifdef __cplusplus
} // end extern "C"
#endif
