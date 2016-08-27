/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/**
   This file is similar to kuhl-util.c, however these are functions
 * that do not depend on any other libraries. Therefore, it should be
 * easy to use this file with other projects.
 */

#define __GNU_SOURCE // make sure are allowed to use GNU extensions. Redundant if compiled with -std=gnu99
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h> // isspace()

#ifndef _WIN32   // Linux, Mac
#include <sys/time.h>
#include <unistd.h>
#include <libgen.h> // dirname()
#endif

#if __APPLE__
#include <mach-o/dyld.h>  // for _NSGetExectuablePath()
#endif

#include "kuhl-nodep.h"
#include "windows-compat.h"



/** Don't call this function, call kuhl_malloc() instead. */
void *kuhl_mallocFileLine(size_t size, const char *file, int line)
{
	if(size == 0)
	{
		fprintf(stderr, "!!!!! malloc Warning !!!!! - Size 0 passed to malloc at %s:%d\n", file, line);
		return NULL;
	}
	void *ret = malloc(size);
	if(ret == NULL)
		fprintf(stderr, "!!!!! malloc Error !!!!! - Failed to allocate %du bytes at %s:%d\n", (int) size, file, line);
	return ret;
}



/** Checks if a file is readable or not by attempting to fopen() the
    file for reading. IMPORTANT: Calling fopen() on a directory is
    undefined---but on Linux fopen() typically behaves as if it were
    successful. Therefore, on Linux, this function will also find
    directories.

 @param filename The file to check
 @return Returns 1 if we can read the file, 0 otherwise.
*/
int kuhl_can_read_file(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if(f == NULL)
		return 0;
	else
	{
		fclose(f);
		return 1;
	}
}

/** Replaces all instances of '\' in a string with '/'. This function
    is useful if you have a relative file path created on a Windows
    machine---and you want to try to open that file on Linux.

    @param input The string to change '\' to '/'
    
    @return A newly allocated string containing a copy of input with
    '\' changed to '/'. The returned string should be free()'d by the
    caller.
*/
static char* kuhl_fix_path(const char* input)
{
	if(input == NULL)
		return NULL;
	
	char *output = strdup(input);
	if(output == NULL) // insufficient memory
	{
		perror("strdup");
		exit(EXIT_FAILURE);
	}
	char *tmp = output;

	while(*tmp != '\0')
	{
		if(*tmp == '\\')
			*tmp = '/';
		tmp++;
	}
	return output;
}

/** Creates a path out of strings a and b (with a '/' inserted between
 * them). Then, checks to see if the path is readable with
 * kuhl_can_read_file().

    @param a The first part of the path.
    @param b The second part of the path.
    @return The new path (if it is readable) or NULL.
*/
static char* kuhl_path_concat_read(const char* a, const char* b)
{
	if(a == NULL || b == NULL)
		return NULL;

	int combinedLen = strlen(a)+strlen(b)+32; // ensure plenty of space
	char *combined = kuhl_malloc(sizeof(char)*combinedLen);
	int ret = snprintf(combined, combinedLen, "%s/%s", a, b);
	if(ret < 0 || ret >= combinedLen)
	{
		fprintf(stderr, "snprintf() error");
		exit(EXIT_FAILURE);
	}

	if(kuhl_can_read_file(combined))
		return combined;
	else
	{
		free(combined);
		return NULL;
	}
}

/** Given a filename, tries to find that file by:
   1) Looking for the file using the given path.

   2) Change '\' characters to '/' in case the provided path uses
   Windows-style path separators.

   3) Search for file relative to directory of executable (on Linux)
   (also, try changing the path separators.)

   4) Search for file using a list of hard-coded directories (also,
   try changing the path separators).

   @param filename The name of the file the caller wants to open.
   
   @return A path to the file that may be different than the path
   provided in the filename parameter. The returned string should be
   free()'d. If the file was not found, a copy of the original
   filename is returned.
*/
char* kuhl_find_file(const char *filename)
{
	if(kuhl_can_read_file(filename))
		return strdup(filename);

	char *pathSepChange = kuhl_fix_path(filename);
	if(kuhl_can_read_file(pathSepChange))
		return pathSepChange;

	char commonDirs[32][256];
	int commonDirsLen = 0;
#ifdef _WIN32
	strncpy(commonDirs[commonDirsLen++], "../../samples", 255); // On windows, binaries get put into extra subdirectory
#else
	strncpy(commonDirs[commonDirsLen++], "../samples", 255); // Find fragment programs in samples directory
#endif
	strncpy(commonDirs[commonDirsLen++], "/home/kuhl/public-ogl/data", 255); // CCSR
	strncpy(commonDirs[commonDirsLen++], "/local/kuhl-public-share/opengl/data", 255); // Rekhi
	strncpy(commonDirs[commonDirsLen++], "/Users/kuhl/public-ogl/data", 255); // OS X laptop
	strncpy(commonDirs[commonDirsLen++], "/research/kuhl/public-ogl/data", 255); // IVS

#if defined(__linux__) || defined(__APPLE__)
	#ifdef __linux__
	/* If we can't open the filename directly, then try opening it
	   with the full path based on the path to the
	   executable. This allows us to more easily run programs from
	   outside of the same directory that the executable that the
	   executable resides without having to specify an absolute
	   path to our shader programs. */
	char exe[1024];
	ssize_t len = readlink("/proc/self/exe", exe, 1023);
	exe[len]='\0';
	#endif
	#ifdef __APPLE__
	char exe[1024];
	uint32_t size = 1024;
	_NSGetExecutablePath(exe, &size); // Todo: if returns non-zero, it didn't fit
    #endif
	
	/* For every relative path in commonDirs, add an additional
	 * directory which has the path to the executable prepended to
	 * it. */
	char *dir = dirname(exe);
	int origLen = commonDirsLen;
	strncpy(commonDirs[commonDirsLen++], dir, 255); // Rekhi
	for(int i=0; i<origLen; i++)
		if(commonDirs[i][0] != '/')
			snprintf(commonDirs[commonDirsLen++], 256, "%s/%s", dir, commonDirs[i]);
#endif

	/*
	printf("File search paths besides the current directory:\n");
	for(int i=0; i<commonDirsLen; i++)
		printf("%s\n", commonDirs[i]);
	*/
		
	
	char *newPath = NULL;
	for(int i=0; i<commonDirsLen; i++)
	{
		newPath = kuhl_path_concat_read(commonDirs[i], filename);
		if(newPath)
		{
			free(pathSepChange);
			return newPath;
		}
		/* Try converting path separators too */
		newPath = kuhl_path_concat_read(commonDirs[i], pathSepChange);
		if(newPath)
		{
			free(pathSepChange);
			return newPath;
		}
	}
	
	free(pathSepChange);
	return strdup(filename);
}



/** Reads a text file.
 *
 * @param filename The file that we want to read in.
 *
 * @return An array of characters for the file. This array should be
 * free()'d when the caller is finished with it. Exits if an error
 * occurs.
 */
char* kuhl_text_read(const char *filename)
{
	int chunkSize    = 1024;        /* read in chunkSize bytes at a time */
	int contentSpace = chunkSize;   /* space in 'content' array */

	/* We add one more character to create room to store a '\0' at the
	 * end. */
	char *content = (char*) kuhl_malloc(sizeof(char)*(contentSpace+1));

	/* Pointer to where next chunk should be stored */
	char *contentLoc = content;

	char *newFilename = kuhl_find_file(filename);
	FILE *fp = fopen(newFilename,"rt");
	free(newFilename);
	int readChars;

	if(fp == NULL)
	{
		fprintf(stderr, "ERROR: Can't open %s.\n", filename);
		exit(EXIT_FAILURE);
	}

	do
	{
		readChars = fread(contentLoc, sizeof(char), chunkSize, fp);
		contentLoc[readChars] = '\0';
		contentSpace += chunkSize;
		content = (char*) realloc(content, sizeof(char)*(contentSpace+1));
		contentLoc = content + contentSpace - chunkSize;

	} while( readChars == chunkSize );

	/* We should now be at end of file. If not, there was an error. */
	if(feof(fp) == 0)
	{
		fprintf(stderr, "ERROR: Can't read %s\n", filename);
		exit(EXIT_FAILURE);
	}

	fclose(fp);
	return content;
}




/** When called per frame, sleeps for a short period of time to limit
 * the frames per second. There are two potential uses for this: (1)
 * When FPS are far higher than the monitor refresh rate and CPU load
 * are high, this can reduce both of them to a more reasonable
 * value. (2) Allows you to test to see how your program might run if
 * it were running on hardware with a lower frame rate.
 *
 * kuhl_limitfps() does not reduce tearing. Use glfwSwapInterval() to
 * change how tearing is handled.
 *
 * @param fps Requested frames per second that we should not exceed.
 *
 * @see kuhl_getfps()
 */
void kuhl_limitfps(int fps)
{
#ifndef _WIN32   // TODO, implement for windows!

	/** The time at which kuhl_limitfps() was last called. */
	static struct timeval limitfps_last = { .tv_sec = 0,.tv_usec = 0 };

	struct timeval tv;
	gettimeofday(&tv, NULL);

	if(limitfps_last.tv_usec == 0 && limitfps_last.tv_sec == 0)
	{
		limitfps_last.tv_usec = tv.tv_usec;
		limitfps_last.tv_sec  = tv.tv_sec;
		return;
	}
	
	// How many microseconds have elapsed since previous call to limitfps() was completed? */
	suseconds_t elapsed_micro = (tv.tv_sec - limitfps_last.tv_sec)*1000000L + (tv.tv_usec - limitfps_last.tv_usec);
	// printf("elapsed_micro %ld\n", elapsed_micro);
	
	// How many microseconds should elapse per frame?
	float microspf = 1.0/fps * 1000000;
	// printf("microsec per frame %f\n", microspf);

	// How many microseconds do we have to kill? 
	suseconds_t microsec_sleep = (suseconds_t)microspf - elapsed_micro;
	if(microsec_sleep > 0)
	{
		/* Just calling usleep() works OK on some machines, but on
		   others we sleep for too long. usleep() only guarantees that
		   we will sleep at least as long as we request. It seemed to
		   be a particular problem when we are trying to limit to
		   100fps and on a macOS laptop. */
		//usleep(microsec_sleep);

		long first = kuhl_microseconds();
		int loop = 1;
		while(loop)
		{
			// How much more do we need to sleep?
			long remain = microsec_sleep - (kuhl_microseconds() - first);

			// If we have little time remaining, just return.
			if(remain <= 2)
				loop = 0;
			else
				usleep(remain / 2); // sleep half of the remaining time.
		}
	}
	else
	{
		// printf("Sleep not needed! We are %ld behind.\n", microsec_sleep);
	}

	gettimeofday(&limitfps_last, NULL);
#endif
}

/** Returns the current time in microseconds. 1 second = 1,000,000 microseconds. 1 millisecond = 1000 microseconds */
long kuhl_microseconds(void)
{
#if _WIN32
	LARGE_INTEGER freq, start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	return (long) ((start.QuadPart * 1000000) / freq.QuadPart);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long us = (tv.tv_sec * 1000000L) + tv.tv_usec;
	return us;
#endif
}

/** Returns the number of milliseconds since the first time this
 * function was called. */
static long kuhl_microseconds_start_time = -1;
long kuhl_microseconds_start(void)
{
	if(kuhl_microseconds_start_time == -1)
		kuhl_microseconds_start_time = kuhl_microseconds();
	return kuhl_microseconds() - kuhl_microseconds_start_time;
}


/** Returns the current time in milliseconds. 1 second = 1000
 * milliseconds */
long kuhl_milliseconds(void)
{
	return kuhl_microseconds() / 1000L;
}

/** Returns the number of milliseconds since the first time this
 * function was called. */
static long kuhl_milliseconds_start_time = -1;
long kuhl_milliseconds_start(void)
{
	if(kuhl_milliseconds_start_time == -1)
		kuhl_milliseconds_start_time = kuhl_milliseconds();
	return kuhl_milliseconds() - kuhl_milliseconds_start_time;
}


/** Initialize a kuhl_fps_state struct which can be used to easily
 * calculate fps.
 *
 * @param state An empty kuhl_fps_state object to be initialized. */
void kuhl_getfps_init(kuhl_fps_state *state)
{
	state->frame = 0;
	state->timebase = kuhl_milliseconds();
	state->fps = -1;
}


/** When called every frame, estimates the frames per second.
 *
 * @param state This is a state structure that is initialized by
 * kuhl_getfps_init().
 *
 * @return An estimate of the frames per second (updated every second).
 *
 * @see kuhl_limitfps()
 */
float kuhl_getfps(kuhl_fps_state *state)
{
	state->frame++;
	
	long now = kuhl_milliseconds();
	// printf("Frame at %ld\n", now);
	
	// If a second has elapsed since our last estimation
	if(now - state->timebase >= 1000)
	{
		// printf("Frames from %ld to %ld: %d\n", state->timebase, now, state->frame);
		
		// Calculate frames per second
		// Frames * 1000 ms/s / (elapsed ms) = frames/second
		state->fps = state->frame*1000.0/(now - state->timebase);
		// Update the time that our estimation occurred
		state->timebase = now;
		// Reset our frame counter.
		state->frame = 0;
	}

	return state->fps;
}



/** Generates a random integer between min and max inclusive. This
 * uses floating point to avoid possible issues with using rand()
 * along with modulo. This approach isn't completely bias free since
 * doubles don't have infinite precision and we aren't guaranteed
 * perfectly uniform distribution after multiplying the value returned
 * by drand48(). But, this is good enough for most purposes.
 *
 * @param min Smallest random number that we want.
 * @param max Largest random number that we want.
 */
int kuhl_randomInt(int min, int max)
{
	static int kuhl_random_init_done = 0; /*< Have we called srand48() yet? */

	if(kuhl_random_init_done == 0)
	{
		// http://stackoverflow.com/questions/8056371
		srand48((getpid()*2654435761U)^time(NULL));
		kuhl_random_init_done = 1;
	}
	
	int possibleVals = max-min+1;
	double fl = drand48(); // [0, 1.0)
	fl = fl*possibleVals; // [0, possibleVals)
	fl = fl+min;          // [min, min+possibleVals)
	                      // [min, min+(max-min+1))
	                      // [min, max+1)
	return (int) floor(fl); // Using floor() makes negative values work.
}

/** Shuffles an array of items randomly.

   @param array Array of items
   @param n Number of items in the array.
   @param size Size of each item.
*/
void kuhl_shuffle(void *array, int n, int size)
{
	char *arr = (char*) array; // Use a char array which we know uses 1 byte pointer arithmetic
	char *tmp = kuhl_malloc(size); // avoid use of VLA

	// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
	for(int i=n-1; i>=1; i--)
	{
		int j = kuhl_randomInt(0, i); // index to swap

		// Swap the values
		memcpy(tmp,        arr+j*size, size);
		memcpy(arr+j*size, arr+i*size, size);
		memcpy(arr+i*size, tmp,        size);
	}

	free(tmp);
}

/** Removes any whitespace characters at the beginning or end of the string in place.

    @param str The null-terminated string to trim in place.
    @return The same pointer as the str parameter.
*/
char* kuhl_trim_whitespace(char *str)
{
	/* Find first non-whitespace character in string (or the null terminator) */
	char *firstNonSpace = str;
	while(isspace(*firstNonSpace) && *firstNonSpace != 0)
		firstNonSpace++;

	/* If it was an empty string */
	if(*firstNonSpace == 0)
	{
		*str = 0;
		return str;
	}

	/* Find the last character in the string */
	char *lastNonSpace = str + strlen(str) - 1;
	while(isspace(*lastNonSpace))
		lastNonSpace--;

	*(lastNonSpace+1) = 0; // set byte after the last nonspace character to null.
	
	// If the string is two non-whitespace characters,
	// lastNonSpace-firstNonSpace will be 1. But, we want to copy both
	// of the characters plus the null terminator. */
	memmove(str, firstNonSpace, lastNonSpace-firstNonSpace+2);
	return str;
}


/**
   Generate random numbers following Gaussian distribution. The
   generated numbers will have a mean of 0 and a standard deviation of
   1.

   @return A random sample from a Gaussian distribution.

   Source: http://c-faq.com/lib/gaussian.html
   Discussed in Knuth and described by Marsaglia
 */
double kuhl_gauss(void)
{
	static double V2;
	static double S;
	static int phase = 0;
	double X;

	if(phase == 0) {
		static double V1;
		do {
			double U1 = (double)rand() / RAND_MAX;
			double U2 = (double)rand() / RAND_MAX;

			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
			} while(S >= 1 || S == 0);

		X = V1 * sqrt(-2 * log(S) / S);
	} else
		X = V2 * sqrt(-2 * log(S) / S);

	phase = 1 - phase;

	return X;
}
