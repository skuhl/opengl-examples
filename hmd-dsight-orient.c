/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
* @author Evan Hauck
*/

#include "hmd-dsight-orient.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

// write "safe", a method that always succeeds (failure is handled by exit())
void writeSafe(const int fd, const unsigned char* buf, size_t numBytes)
{
	while (numBytes > 0)
	{
		ssize_t result = write(fd, buf, numBytes);
		if (result == -1)
		{
			if (errno == EAGAIN)
			{
				continue;
			}
			fprintf(stderr, "writeSafe failed, errno = %d\n", errno);
			exit(EXIT_FAILURE);
		}
		buf += result;
		numBytes -= (size_t)result;
	}
}

// read "safe", same as writeSafe but for reading
void readSafe(int fd, unsigned char* buf, size_t numBytes)
{
	while (numBytes > 0)
	{
		ssize_t result = read(fd, buf, numBytes);
		if (result == -1)
		{
			if (errno == EAGAIN)
			{
				continue;
			}
			fprintf(stderr, "readSafe failed, errno = %d\n", errno);
			exit(1);
		}
		buf += result;
		numBytes -= (size_t)result;
	}
}

void swapEndianessFloat(float* data, const int n)
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

HmdControlState initHmdControl(const char* deviceFile)
{
	HmdControlState result;
	result.fd = open(deviceFile, O_RDWR | O_NOCTTY);
	if (result.fd == -1)
	{
		fprintf(stderr, "Could not open %s for HMD rotation sensor driver\n", deviceFile);
		exit(EXIT_FAILURE);
	}
	return result;
}

// http://stackoverflow.com/questions/2100331
#define IS_BIG_ENDIAN (!*(unsigned char *)&(unsigned short){1})

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
