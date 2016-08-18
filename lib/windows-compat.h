#pragma once
/* This file includes implementations of functions that are
   available on Linux/Apple/Unix machines but not available on Windows. 
 */

// When compiling on windows, add suseconds_t and the rand48 functions.
#if defined __MINGW32__ || defined _WIN32

// Disable warnings about unsafe functions. For example, numerous warnings
// about how strcpy() should be replaced with strcpy_s()
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

	double drand48();
	void srand48(long seed);
	void usleep(DWORD waitTime);

#ifdef __cplusplus
}
#endif


#endif  // end if windows
