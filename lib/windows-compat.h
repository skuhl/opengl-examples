#pragma once
/* This file includes implementations of functions that are
   available on Linux/Apple/Unix machines but not available on Windows. 
 */

// When compiling on windows, add suseconds_t and the rand48 functions.
#if defined __MINGW32__ || defined _WIN32
#include <Windows.h>
double drand48();
void srand48(long seed);
void usleep(DWORD waitTime);
#endif
