/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/**
   @file

   msg.c provides a basic logging mechanism.

   All messages are written to a log file with detailed information
   such as type (debug, info, error, etc), time, file+line in source
   code where msg() was called, the function which called msg(), and
   the message itself.

   All non-debugging messages are also printed to stdout or, in the
   case of significant error messages, stderr. The messages printed to
   the console are also highlighted to attract attention to the most
   significant messages.
   
    @author Scott Kuhl
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <libgen.h> /* basename() */
#include <sys/time.h> // gettimeofday()
#include <unistd.h> // isatty()
#endif
#include <time.h> // localtime()
#include <string.h>
#include <errno.h>

#include "msg.h"
#include "kuhl-config.h"
#include "kuhl-nodep.h"
#include "windows-compat.h"

static FILE *f = NULL;  /**< The file stream for our log file */
static char *logfile = NULL; /**< The filename of the log file. */

/** Writes a timestamp string to a pre-allocated char array.

    @param buf A buffer of len bytes where the timestamp should be stored.
    @param len The length of the buffer.
*/
static void msg_timestamp(char *buf, int len)
{
#if 1
	// time relative to start time
	static int needsInit = 1;
	static long starttime;
	if(needsInit)
	{
		starttime = kuhl_microseconds();
		needsInit = 0;
	}

	long nowtime = kuhl_microseconds();
	long difftime = nowtime-starttime;
	double timestamp = difftime / 1000000.0;
	snprintf(buf, len, "%11.6f", timestamp);
#else
	// Absolute time
	struct tm *now = localtime(&(tv.tv_sec));
	char buf1[1024];
	strftime(buf1, 1024, "%H%M%S", now);
	snprintf(buf, len, "%s.%06ld", buf1, tv.tv_usec);
#endif
}

/** Given a message type, creates a string describing that message type.

    @param type The message type.
    @param buf A buffer to store a string which describes the message type.
    @param len The length of the buffer.
*/
static void msg_type_string(msg_type type, char *buf, int len)
{
	switch(type)
	{
		case MSG_DEBUG:
			snprintf(buf, len, "[DEBUG]");
			break;
		case MSG_INFO:
			snprintf(buf, len, "[INFO ]");
			break;
		case MSG_WARNING:
			snprintf(buf, len, "[WARN ]");
			break;
		case MSG_ERROR:
			snprintf(buf, len, "[ERROR]");
			break;
		case MSG_FATAL:
			snprintf(buf, len, "[FATAL]");
			break;
		case MSG_BOLD:
			snprintf(buf, len, "[BOLD ]");
			break;
		case MSG_GREEN:
			snprintf(buf, len, "[GREEN]");
			break;
		case MSG_BLUE:
			snprintf(buf, len, "[BLUE ]");
			break;
		case MSG_CYAN:
			snprintf(buf, len, "[CYAN ]");
			break;
		case MSG_PURPLE:
			snprintf(buf, len, "[PURPL]");
			break;
		default:
			snprintf(buf, len, "[?????]");
	}
}

/** Returns 1 if this type of message should be printed to the
 * console. */
static int msg_show_type(msg_type type)
{
	switch(type)
	{
		case MSG_DEBUG:   return 0;
		case MSG_INFO:    return 1;
		case MSG_WARNING: return 1;
		case MSG_ERROR:   return 1;
		case MSG_FATAL:   return 1;
		default:          return 1;
	}
}

/** Writes bytes to a file stream to enable colors, bold, etc for special messages.

    @param type The message type.
    
    @param stream A stream that the bytes should be written to.
 */
static void msg_start_color(msg_type type, FILE *stream)
{
#ifdef _WIN32
	return;
#else
	/* Don't do anything if the stream is invalid or if the stream is
	 * not associated with a tty (i.e., we only use colors if writing
	 * to stdout or stderr, not when writing to a file. */
	if(stream == NULL || isatty(fileno(stream)) == 0)
		return;

	switch(type)
	{
		case MSG_DEBUG:
			break;
		case MSG_INFO:
			break;
		case MSG_WARNING:
			fprintf(stream, "\x1B[33m"); // yellow text
			break;
		case MSG_ERROR:
			fprintf(stream, "\x1B[31m"); // red text
			break;
		case MSG_FATAL:
			fprintf(stream, "\x1B[31m"); // red text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		case MSG_GREEN:
			fprintf(stream, "\x1B[32m"); // green text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		case MSG_BLUE:
			fprintf(stream, "\x1B[34m"); // blue text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		case MSG_CYAN:
			fprintf(stream, "\x1B[36m"); // cyan text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		case MSG_PURPLE:
			fprintf(stream, "\x1B[35m"); // magenta text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		default:
			break;
	}
#endif
}

/** Writes bytes to a stream to reset the colors back to the default. */
static void msg_end_color(msg_type type, FILE *stream)
{
#ifdef _WIN32
	return;
#else
	if(stream == NULL || isatty(fileno(stream)) == 0)
		return;

	fprintf(stream, "\x1B[0m");
#endif
}



#if 0
/** Prints a backtrace to the stream. The backtrace will list the
    names of the functions and the addresses. Addresses can be
    converted to file:line information by running something similar to
    '/usr/bin/addr2line 0x123456 -e triangle'

    C++ function names may be mangled.

    This is a GNU extension. Depending on how this code is compiled,
    it may produce warnings.
*/
void msg_backtrace(FILE *stream)
{
#ifdef __GLIBC__

#define BACKTRACE_SIZE 100
	void *array[BACKTRACE_SIZE];
	size_t size    = backtrace(array, BACKTRACE_SIZE);
	char **strings = backtrace_symbols(array, BACKTRACE_SIZE);
	fprintf(stream, "Backtrace:\n");
	for(size_t i=0; i<size; i++)
		fprintf(stream, "   %s\n", strings[i]);
	free(strings);
#undef BACKTRACE_SIZE

#else // __GLIBC__
	fprintf(stream, "msg_backtrace() requires glibc.");
#endif
}
#endif



/** Initializes the logging system, creates the log file if
 * needed. Also, writes a message informing the user about the
 * location of the log file. The time printed in the log file will be
 * relative to the time that this first message was printed.
 */
static void msg_init(void)
{
	/* If we have already successfully initialized the msg() system, f
	 * (the file descriptor for the log file) will be initialized. */
	if(f != NULL)
		return;

	// Set to 1 to overwrite existing log file, 0 to append.
	const int append = kuhl_config_boolean("log.append", 0,0);

	// Check if log file name is specified in an environment variable
	const char* config_logfile = kuhl_config_get("log.filename");
	if(config_logfile != NULL && strlen(config_logfile) > 0)
		logfile = strdup(config_logfile);
	else
		logfile = strdup("log.txt"); // default log file name

	/* When the first message gets printed, we will also probably call
	 * kuhl_config_get() for the first time. kuhl_config_get() will
	 * then try to print a message about the newly loaded config
	 * file---calling msg() again. This second call to msg() will work
	 * because kuhl_config_get() doesn't print a message the second
	 * time that it is called. However, the first call to msg() can
	 * get to this point only to find that the log file is already
	 * open (i.e., kuhl_config_get() indirectly opened the file and
	 * completed the initialization. */
	if(f != NULL)
		return;
	
	f = fopen(logfile, append ? "a" : "w");
	if(f == NULL)
	{
		fprintf(stderr, "Unable to %s to log file %s\n", append ? "append" : "write", logfile);
		exit(EXIT_FAILURE);
	}
		
	if(append)
	{
		// outputs of multiple runs.
		fprintf(f, "============================================================\n");
		fprintf(f, "=== Program started ========================================\n");
		fprintf(f, "============================================================\n");
	}

	// Header for log file
	fprintf(f, "[TYPE ]    seconds     filename:line message\n");
	fprintf(f, "------------------------------------------\n");

	// Write message so user knows the log file is being created.
	if(append)
		msg(MSG_INFO, "Messages are being appended to '%s'\n", logfile);
	else
		msg(MSG_INFO, "Messages are being written to '%s'\n", logfile);
}

/** Writes a message to the log file.
    @param type The type of message to log
    @param fileName The filename where this function was called from.
    @param lineNum The line number in the file where this function was called from.
    @param funcName The name of the function which called this function.
    @param msg The message to log
*/
void msg_details(msg_type type, const char *fileName, int lineNum, const char *funcName, const char *msg, ...)
{
	msg_init();
	
	/* Construct a string for the user's message */
	char msgbuf[1024];
	va_list args;
	va_start(args, msg);
	vsnprintf(msgbuf, 1024, msg, args);
	va_end(args);

	/* Remove any newlines at the end of the message. */
	int msgbufidx = strlen(msgbuf)-1;
	while(msgbuf[msgbufidx] == '\n')
	{
		msgbuf[msgbufidx] = '\0';
		msgbufidx--;
	}
	
	/* info to prepend to message printed to console */
	char typestr[1024];
	msg_type_string(type, typestr, 1024);

	/* Determine the stream that we are going to print out to: stdout,
	 * stderr, or don't print to console */
	FILE *stream = stdout;
	if(type == MSG_ERROR || type == MSG_FATAL)
		stream = stderr;
	if(msg_show_type(type) == 0)
		stream = NULL;

	char timestamp[1024];
	msg_timestamp(timestamp, 1024);
	char *fileNameCopy = strdup(fileName);
	char *shortFileName = fileNameCopy;
#ifndef _WIN32
	shortFileName = basename(fileNameCopy);
#endif

	/* Print the message to stderr or stdout */
	if(stream)
	{
		// If using a non-standard logfile name, prepend the name to
		// the message. This makes it easier to distinguish between
		// which process is creating which message if there are
		// multiple programs running at once.
		char prepend[1024];
		if(strcmp(logfile, "log.txt") == 0)
			prepend[0] = '\0';
		else
			snprintf(prepend, 1024, "(%s) ", logfile);
		
		msg_start_color(type, stream);
		fprintf(stream, "%s %s%s\n", typestr, prepend, msgbuf);
		/* Print additional details to console for significant errors */
		if(type == MSG_FATAL || type == MSG_ERROR)
		{
			fprintf(stream, "%s %sOccurred at %s:%d in the function %s()\n",
			        typestr, prepend, shortFileName, lineNum, funcName);

			// if(type == FATAL)
			//   msg_backtrace(stream);
		}
		msg_end_color(type, stream);
	}

	// Not using funcName to try to keep log shorter.
	fprintf(f, "%s%s %12s:%-4d %s\n", typestr, timestamp, shortFileName, lineNum, msgbuf);
	free(fileNameCopy);

	/* Ensure messages are written to the file or console. */
	fflush(stream);
	fflush(f);
}

/** ASSIMP can be configured to call a callback function every time it
    needs to print a message. This function allows us to feed assimp's
    messages through our logging system.
*/
void msg_assimp_callback(const char* msg, char *usr)
{
	msg_details(MSG_DEBUG, "ASSIMP", 0, "", "%s", msg);
}

