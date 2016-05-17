/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/**
   @file

    Provides an easy-to-use interface to set up and do input/output
    with a serial connection.

    @author Scott Kuhl
    @author Evan Hauck (contributed original versions of read/write functions)
*/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/** This enum is used by some serial related functions */
enum
{
	SERIAL_NONE      = 0,  /**< No options */
	SERIAL_CONSUME   = 1,  /**< When reading, consume extra data */
	SERIAL_NONBLOCK  = 2   /**< Indicate if we would block. */
};

int serial_find(int fd, char *bytes, int len, int maxbytes);
void serial_discard(int fd);
void serial_write(const int fd, const char* buf, size_t numBytes);
int serial_read(int fd, char* buf, size_t numBytes, int options);
int serial_open(const char *deviceFile, int speed, int parity, int vmin, int vtime);
void serial_close(int fd);

#ifdef __cplusplus
} // end extern "C"
#endif
