/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
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


#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

#include "msg.h"
#include "serial.h"

/**
   Reliably write bytes to a file descriptor. Exits on failure.

   @param fd File descriptor to write to.
   @param buf Buffer containing bytes to write.
   @param numBytes Number of bytes in the buffer that should be written.
 */
void serial_write(const int fd, const char* buf, size_t numBytes)
{
	while (numBytes > 0)
	{
		ssize_t result = write(fd, buf, numBytes);
		if(result < 0)
		{
			msg(FATAL, "write error %s", strerror(errno));
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
   
   @param options If SERIAL_CONSUME bit is set and there are many
   bytes avaialable to read, discard some multiple of numBytes and
   return the last set of numBytes. If SERIAL_CACHED is set, the
   return value will indicate if data was read or not.

   @return Number of bytes read (which should always match
   numBytes). 0 can only be returned if SERIAL_NONBLOCK is set.
 */
int serial_read(int fd, char* buf, size_t numBytes, int options)
{
	char *ptr = buf;
	size_t totalBytesRead = 0;

	// Determine how many bytes there are to read.
	size_t bytesAvailable = 0;
	ioctl(fd, FIONREAD, &bytesAvailable);

	/* If SERIAL_CONSUME is set and if there are more than numBytes*2
	   bytes available, repeatedly read numBytes. This will eventually
	   lead to having numBytse or slightly more than numBytes
	   available for us to actually read. */
	if(options & SERIAL_CONSUME)
	{
		while(bytesAvailable >= numBytes*2)
		{
			int r = read(fd, ptr, numBytes);
			if(r == 0)
				msg(ERROR, "There was nothing to read!\n");
			else if(r < 0)
			{
				msg(FATAL, "read: %s", strerror(errno));
				exit(EXIT_FAILURE);
			}
			else
				bytesAvailable -= r;
		}
	}

	/* If SERIAL_NONBLOCK is set, and if there are not enough bytes
	 * available to read, return 0 so the caller can instead return a
	 * cached value. */
	if(bytesAvailable < numBytes &&
	   options & SERIAL_NONBLOCK)
	{
		return 0;
	}

	// If we read data slowly, and don't consume all data, the value here will grow.
	// msg(BLUE, "Bytes available to read: %d\n", bytesAvailable);

	/* Actually read the data */
	while(totalBytesRead < numBytes)
	{
		ssize_t bytesRead = read(fd, ptr, numBytes-totalBytesRead);
		if(bytesRead == 0)
			msg(ERROR, "There was nothing to read!\n");
		else if(bytesRead < 0)
		{
			msg(FATAL, "read: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
		else
		{
			// read() either read all or some of the bytes we wanted to read.
			// msg(GREEN, "Read %d bytes\n", bytesRead);
			ptr += bytesRead;
			totalBytesRead += bytesRead;
		}
	}

#if 0
	// Print the bytes we read
	printf("Read a total of %d bytes:\n", totalBytesRead);
	for(size_t i=0; i<numBytes; i++)
		printf("%d ", buf[i]);
	printf("\n");
#endif

	/* If we didn't read the full numBytes, we would have exited. */
	return numBytes;
}


/** Applies settings to a serial connection (sets baud rate, parity, etc).

    @param fd The file descriptor corresponding to an open serial connection.
    @param speed The baud rate to be applied to the connection.
    @param parity 0=no parity; 1=odd parity; 2=even parity
    @param vmin 0 = nonblocking; if >1, block until we have received at least vmin bytes
    @param vtime If blocking, tenths of a second we should block until we give up.

    This code is partially based on:
    https://stackoverflow.com/questions/6947413
*/
static void serial_settings(int fd, int speed, int parity, int vmin, int vtime)
{
	/* get current serial port settings */
	struct termios toptions;
	memset(&toptions, 0, sizeof(struct termios));
	if(tcgetattr(fd, &toptions) == -1)
		msg(ERROR, "tcgetattr error: %s\n", strerror(errno));

	int baud = 0;
	switch(speed)
	{
		case 100: baud = B110; break;
		case 300: baud = B300; break;
		case 600: baud = B600; break;
		case 1200: baud = B1200; break;
		case 2400: baud = B2400; break;
		case 4800: baud = B4800; break;
		case 9600: baud = B9600; break;
			//case 14400: baud = B14400; break;
		case 19200: baud = B19200; break;
			//case 28800: baud = B28800; break;
		case 38400: baud = B38400; break;
			// case 56000: baud = B56000; break;
		case 57600: baud = B57600; break;
		case 115200: baud = B115200; break;
		// other rates: 
		//case 128000: baud = B128000; break;
			// case 153600: baud = B153600; break;
			//case 256000: baud = B256000; break;
		case 460800: baud = B460800; break;
		case 921600: baud = B921600; break;
		default:
			msg(FATAL, "Invalid baud rate specified: %d\n", speed);
			exit(EXIT_FAILURE);
	}

	/* set baud rate both directions */
	cfsetispeed(&toptions, baud);
	cfsetospeed(&toptions, baud);

	// http://playground.arduino.cc/Interfacing/LinuxTTY
	// Says the following settings are necessary to interface with arduino:
	// stty -F /dev/ttyUSB0 cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
	toptions.c_cflag = (toptions.c_cflag & ~CSIZE) | CS8; // 8 bit	
	//toptions.c_iflag &= ~IGNBRK;  // disable break processing
	toptions.c_lflag = 0;         // no signaling chars, no echo, no canonical processing
	toptions.c_lflag = NOFLSH;
	toptions.c_oflag = 0;         // no remapping, no delays
	toptions.c_cc[VMIN] = vmin;   
	toptions.c_cc[VTIME] = vtime; // If blocking, max time to block in tenths of a second (5 = .5 seconds).

	toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	toptions.c_cflag |= (CLOCAL | CREAD);// ignore modem controls, enable reading
	toptions.c_cflag &= ~(PARENB | PARODD); // shut off parity
	if(parity == 1)
		toptions.c_cflag |= PARENB|PARODD; // enable odd parity
	else if(parity == 2)
		toptions.c_cflag |= PARENB; // enable even parity

	toptions.c_cflag &= ~CSTOPB;  // one stop bit
	toptions.c_cflag &= ~CRTSCTS; // disable hardware flow control

	// Apply our new settings
	if(tcsetattr(fd, TCSANOW, &toptions) == -1)
		msg(ERROR, "tcgetattr error: %s\n", strerror(errno));
}


/** Discards any bytes that are received but not read and written but
    not transmitted. */
void serial_discard(int fd)
{
	tcflush(fd, TCIOFLUSH);
}

/** Close a serial connection.

 @param fd The file descriptor to close.
*/
void serial_close(int fd)
{
	if(close(fd) == -1)
		msg(ERROR, "close: %s\n", strerror(errno));
}

/** Open a serial connection and applies settings to the connection.

    @param deviceFile The serial device to open (often /dev/ttyUSB0 or /dev/ttyACM0)
    @param speed The baud rate to be applied to the connection.
    @param parity 0=no parity; 1=odd parity; 2=even parity
    @param vmin 0 = nonblocking; if >1, block until we have received at least vmin bytes
    @param vtime If blocking, tenths of a second we should block until we give up.
*/
int serial_open(const char *deviceFile, int speed, int parity, int vmin, int vtime)
{
	msg(DEBUG, "Opening serial connection to %s at %d baud\n", deviceFile, speed);
	int fd = 0;
#ifndef __MINGW32__
	fd = open(deviceFile, O_RDWR | O_NOCTTY);
#else
	fd = open(deviceFile, O_RDWR);
#endif
	if(fd == -1)
	{
		msg(FATAL, "Could not open serial connection to '%s'\n", deviceFile);
		exit(EXIT_FAILURE);
	}

#ifndef __MINGW32__
	serial_settings(fd, speed, parity, vmin, vtime);
#endif

	msg(DEBUG, "Serial connection to '%s' is open on fd=%d.\n", deviceFile, fd);
	return fd;
}
