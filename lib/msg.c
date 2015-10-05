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
#include <libgen.h>
#include <sys/time.h> // gettimeofday()
#include <time.h> // localtime()
#include <string.h>
#include <unistd.h> // isatty()
#include <errno.h>

#include "msg.h"

static FILE *f = NULL;  /*< The file stream for our log file */
static char *logfile = NULL;

/** Writes a timestamp string to a pre-allocated char array.

    @param buf A buffer of len bytes where the timestamp should be stored.
    @param len The length of the buffer.
*/
static void msg_timestamp(char *buf, int len)
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) < 0)
	{
		msg(FATAL, "gettimeofday: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

#if 0
	// Absolute time
	struct tm *now = localtime(&(tv.tv_sec));
	char buf1[1024];
	strftime(buf1, 1024, "%H%M%S", now);
	snprintf(buf, len, "%s.%06ld", buf1, tv.tv_usec);
#else
	// Relative to start time.
	double time = tv.tv_sec + tv.tv_usec / 1000000.0;
	static double firstTime = -1;
	if(firstTime < 0)
		firstTime = time;
	snprintf(buf, len, "%11.6f", time-firstTime);
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
		case DEBUG:
			snprintf(buf, len, "[DEBUG]");
			break;
		case INFO:
			snprintf(buf, len, "[INFO ]");
			break;
		case WARNING:
			snprintf(buf, len, "[WARN ]");
			break;
		case ERROR:
			snprintf(buf, len, "[ERROR]");
			break;
		case FATAL:
			snprintf(buf, len, "[FATAL]");
			break;
		case BOLD:
			snprintf(buf, len, "[BOLD ]");
			break;
		case GREEN:
			snprintf(buf, len, "[GREEN]");
			break;
		case BLUE:
			snprintf(buf, len, "[BLUE ]");
			break;
		case CYAN:
			snprintf(buf, len, "[CYAN ]");
			break;
		case PURPLE:
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
		case DEBUG:   return 0;
		case INFO:    return 1;
		case WARNING: return 1;
		case ERROR:   return 1;
		case FATAL:   return 1;
		default:      return 1;
	}
}

/** Writes bytes to a file stream to enable colors, bold, etc for special messages.

    @param type The message type.
    
    @param stream A stream that the bytes should be written to.
 */
static void msg_start_color(msg_type type, FILE *stream)
{
	/* Don't do anything if the stream is invalid or if the stream is
	 * not associated with a tty (i.e., we only use colors if writing
	 * to stdout or stderr, not when writing to a file. */
	if(stream == NULL || isatty(fileno(stream)) == 0)
		return;

	switch(type)
	{
		case DEBUG:
			break;
		case INFO:
			break;
		case WARNING:
			fprintf(stream, "\x1B[33m"); // yellow text
			break;
		case ERROR:
			fprintf(stream, "\x1B[31m"); // red text
			break;
		case FATAL:
			fprintf(stream, "\x1B[31m"); // red text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		case GREEN:
			fprintf(stream, "\x1B[32m"); // green text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		case BLUE:
			fprintf(stream, "\x1B[34m"); // blue text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		case CYAN:
			fprintf(stream, "\x1B[36m"); // cyan text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		case PURPLE:
			fprintf(stream, "\x1B[35m"); // magenta text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		default:
			break;
	}

}

/** Writes bytes to a stream to reset the colors back to he default. */
static void msg_end_color(msg_type type, FILE *stream)
{
	if(stream == NULL || isatty(fileno(stream)) == 0)
		return;

	fprintf(stream, "\x1B[0m");
}



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
	const int append = 0;

	// Check if log file name is specified in an environment variable
	const char* envvar_logfile = getenv("MSG_LOGFILE");
	if(envvar_logfile != NULL && strlen(envvar_logfile) > 0)
		logfile = strdup(envvar_logfile);
	else
		logfile = strdup("log.txt"); // default log file name

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
		msg(INFO, "Messages are being appended to '%s'\n", logfile);
	else
		msg(INFO, "Messages are being written to '%s'\n", logfile);

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
	if(type == ERROR || type == FATAL)
		stream = stderr;
	if(msg_show_type(type) == 0)
		stream = NULL;

	char timestamp[1024];
	msg_timestamp(timestamp, 1024);
	char *fileNameCopy = strdup(fileName);
	char *shortFileName = basename(fileNameCopy);
	
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
		/* Print additional details to console for significant errors */
		if(type == FATAL || type == ERROR)
		{
			fprintf(stream, "%s %s%s\n", typestr, prepend, msgbuf);
			fprintf(stream, "%s %sOccurred at %s:%d in the function %s()\n",
			        typestr, prepend, shortFileName, lineNum, funcName);
		}
		else
			fprintf(stream, "%s %s%s\n", typestr, prepend, msgbuf);
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
	msg_details(DEBUG, "ASSIMP", 0, "", "%s", msg);
}




