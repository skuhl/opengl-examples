/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// __func__ may not be available on Windows, use __FUNCTION__ instead.
// See: https://stackoverflow.com/questions/2281970/
#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

	
typedef enum {
	/* Types of messages used internally by libkuhl, can be used by applications that use libkuhl too: */
	MSG_DEBUG,   /*< Messages that are occasionally useful when troubleshooting (written to logfile) */
	MSG_INFO,    /*< Messages are useful and/or short (written to stdout + logfile) */
	MSG_WARNING, /*< Messages related to things the user should consider fixing (yellow, written to stdout + logfile) */
	MSG_ERROR,   /*< Messages that a user will probably want to fix (red; written to stderr + logfile) */
	MSG_FATAL,   /*< Complete failure, message prior to exit (bold, red; written to stderr + logfile)*/

	/* Other types of messages intended for developers to use however they want: */
	MSG_BOLD,   /*< (bold, written to stdout + logfile) */
	MSG_GREEN,  /*< (bold, green, written to stdout + logfile) */
	MSG_BLUE,   /*< (bold, blue, written to stdout + logfile) */
	MSG_CYAN,   /*< (bold, cyan, written to stdout + logfile) */
	MSG_PURPLE  /*< (bold, magenta, written to stdout + logfile) */
} msg_type;

void msg_details(msg_type type, const char *fileName, int lineNum, const char *funcName, const char *msg, ...);
void msg_assimp_callback(const char* msg, char *usr);

/** Prints the message and saves information to a logfile. C99
 * requires that __VA_ARGS__ corresponds to at least one parameter
 * (not zero parameters). */
#define msg(TYPE, ...) msg_details(TYPE, __FILE__, __LINE__, __func__, __VA_ARGS__)


#ifdef __cplusplus
} // end extern "C"
#endif

