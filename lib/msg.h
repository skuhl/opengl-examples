/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */


#ifndef __MSG_H__
#define __MSG_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	DEBUG,   /*< Messages that are occasionally useful when troubleshooting */
	INFO,    /*< Messages are useful and/or short */
	WARNING, /*< Messages related to things the user should consider fixing */
	ERROR,   /*< Messages that a user will probably want to fix */
	FAIL     /*< Complete failure, message prior to exit */
} msg_type;

void msg_details(msg_type type, const char *fileName, int lineNum, const char *funcName, const char *msg, ...);
void msg_assimp_callback(const char* msg, char *usr);

/** Print an error message to stderr with file and line number
 * information. */
#define msg(TYPE, M, ...) msg_details(TYPE, __FILE__, __LINE__, __func__, M, ##__VA_ARGS__)


#ifdef __cplusplus
} // end extern "C"
#endif
#endif // end __MSG_H__
