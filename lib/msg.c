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

static FILE *f = NULL;


static void msg_timestamp(char *buf, int len)
{
	struct timeval tv;
	if(gettimeofday(&tv, NULL) < 0)
	{
		msg(FAIL, "gettimeofday: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct tm *now = localtime(&(tv.tv_sec));
	char buf1[1024];
	strftime(buf1, 1024, "%Y%m%d-%H%M%S", now);
	snprintf(buf, len, "%s.%06ld", buf1, tv.tv_usec);
}

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
		case FAIL:
			snprintf(buf, len, "[FAIL ]");
			break;
		default:
			snprintf(buf, len, "[?????]");
	}
}

/** Returns 1 if this type of message should be printed to the console. */
static int msg_show_type(msg_type type)
{
	switch(type)
	{
		case DEBUG:   return 0;
		case INFO:    return 1;
		case WARNING: return 1;
		case ERROR:   return 1;
		case FAIL:    return 1;
		default:      return 1;
	}
}

static void msg_start_color(msg_type type, FILE *stream)
{
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
		case FAIL:
			fprintf(stream, "\x1B[31m"); // red text
			fprintf(stream, "\x1B[1m");  // bold
			break;
		default:
			break;
	}

}

static void msg_end_color(msg_type type, FILE *stream)
{
	if(stream == NULL || isatty(fileno(stream)) == 0)
		return;

	fprintf(stream, "\x1B[0m");
}



/** Initializes the logging system, creates the log file if needed.
 */
static void msg_init(void)
{
	// Set to 1 to overwrite existing log file, 0 to append.
	const int append = 0;
	const char *logfile = "log.txt";
	
	if(f != NULL)
		return;

	if(append)
	{
		f = fopen(logfile, "a");
		fprintf(f, "============================================================\n");
		fprintf(f, "=== Program started ========================================\n");
		fprintf(f, "============================================================\n");
		msg(INFO, "Messages are being appended to %s\n", logfile);
	}
	else
		f = fopen(logfile, "w"); // overwrite

	fprintf(f, "[TYPE ] YYYYMMDD-HHMMSS.uSEC   file:line function() message\n");
	fprintf(f, "-----------------------------------------------------------\n");
	msg(INFO, "Messages are being written to %s\n", logfile);
}


void msg_details(msg_type type, const char *fileName, int lineNum, const char *funcName, const char *msg, ...)
{
	msg_init();
	
	/* Construct a string for the user's message */
	char msgbuf[1024];
	va_list args;
	va_start(args, msg);
	vsnprintf(msgbuf, 1024, msg, args);
	va_end(args);

	/* info to prepend to message printed to console */
	char typestr[1024];
	msg_type_string(type, typestr, 1024);

	/* Determine the stream that we are going to print out to: stdout,
	 * stderr, or don't print to console */
	FILE *stream = stdout;
	if(type == ERROR || type == FAIL)
		stream = stderr;
	if(msg_show_type(type) == 0)
		stream = NULL;

	if(stream)
	{
		msg_start_color(type, stream);
		fprintf(stream, "%s %s", typestr, msgbuf);
		msg_end_color(type, stream);
	}

	/* info to prepend to message printed to log file */
	char timestamp[1024];
	msg_timestamp(timestamp, 1024);
	if(fileName)
	{
		char *fileNameCopy = strdup(fileName);
		char *shortFileName = basename(fileNameCopy);
		fprintf(f, "%s %s %s:%d %s() %s", typestr, timestamp, shortFileName, lineNum, funcName, msgbuf);
		free(fileNameCopy);
	}
	else
	{
		fprintf(f, "%s %s %s", typestr, timestamp, msgbuf);

	}

	/* Add a newline to the end of the message if one was absent */
	int msgbuflen = strlen(msgbuf);
	if(msgbuf[msgbuflen-1] != '\n')
	{
		if(stream)
			fprintf(stream, "\n");
		fprintf(f, "\n");
	}

	/* Ensure messages are written to the file or console. */
	fflush(stream);
	fflush(f);
}

void msg_assimp_callback(const char* msg, char *usr)
{
	msg_details(DEBUG, NULL, 0, NULL, "ASSIMP: %s %s", usr, msg);
}




