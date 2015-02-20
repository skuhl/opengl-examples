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
 */

#include "hmd-dsight-orient.h"
#include "kuhl-util.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/**
   Reliably write bytes to a file descriptor. Exits on failure.

   @param fd File descriptor to write to.
   @param buf Buffer containing bytes to write.
   @param numBytes Number of bytes in the buffer that should be written.
 */
static void writeSafe(const int fd, const unsigned char* buf, size_t numBytes)
{
	while (numBytes > 0)
	{
		ssize_t result = write(fd, buf, numBytes);
		if(result < 0)
		{
			kuhl_errmsg("write:");
			perror("");
			exit(EXIT_FAILURE);
		}
		// write() wrote none, some, or all of the bytes we wanted to write.
		buf += result;
		numBytes -= (size_t)result;
	}
}

/**
   Reliably read bytes from a file descriptor. Exits on failure.

   @param fd File descriptor to read from.
   @param buf Buffer containing bytes to read.
   @param numBytes Number of bytes in the buffer that be read.
 */
static void readSafe(int fd, unsigned char* buf, size_t numBytes)
{
	while (numBytes > 0)
	{
		ssize_t result = read(fd, buf, numBytes);
		if(result == 0)
		{
			kuhl_errmsg("readSafe reached end of file(?!)\n");
			exit(EXIT_FAILURE);
		}
		else if(result < 0)
		{
			kuhl_errmsg("read:");
			perror("");
			exit(EXIT_FAILURE);
		}
		// read() either read all or some of the bytes we wanted to read.
		buf += result;
		numBytes -= (size_t)result;
	}
}

/**
   Swap the endian of an array of floats.
   @param data A pointer to an array of floats.
   @param n The number of floats in the array.
*/
static void swapEndianessFloat(float* data, const int n)
{
	unsigned char* charData = (unsigned char*)data;
	int i;
	for (i = 0; i < n; i++)
	{
		unsigned char tmp;
		tmp = charData[i * 4 + 0];
		charData[i * 4 + 0] = charData[i * 4 + 3];
		charData[i * 4 + 3] = tmp;
		tmp = charData[i * 4 + 1];
		charData[i * 4 + 1] = charData[i * 4 + 2];
		charData[i * 4 + 2] = tmp;
	}
}

/** Opens a connection to the orientation sensor in the dSight HMD.

    @param deviceFile The serial device to communicate with. For example, /dev/ttyACM0
*/
HmdControlState initHmdControl(const char* deviceFile)
{
	HmdControlState result;
	result.fd = open(deviceFile, O_RDWR | O_NOCTTY);
	if (result.fd == -1)
	{
		kuhl_errmsg("Could not open %s for HMD rotation sensor driver\n", deviceFile);
		exit(EXIT_FAILURE);
	}
	return result;
}


// http://stackoverflow.com/questions/2100331
#define IS_BIG_ENDIAN (!*(unsigned char *)&(unsigned short){1})


/** Retrieve the latest orientation from the dSight HMD.

    @param state A HmdControlState struct created by initHmdControl()
    @param The resulting quaternion.
*/
void updateHmdControl(HmdControlState *state, float quaternion[4])
{
	const unsigned char writeData[3] = { 0xf7, 0x00, 0x00 };
	writeSafe(state->fd, writeData, 3);

	readSafe(state->fd, (unsigned char*)quaternion, 4 * sizeof(float));
	if (!IS_BIG_ENDIAN)
	{
		// HMD returns float in big-endian order
		swapEndianessFloat(quaternion, 4);
	}
	// TODO: Not sure if quaternion itself is in right order, either.
}
