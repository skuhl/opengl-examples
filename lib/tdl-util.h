/*
 * This file contains useful methods for reading, writing and creating Tracked Data Log (.tdl) files
 * @author John Thomas
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Moves the cursor to the first data point entry.
 * This MUST be called before any calls to tdl_read.
 * Additionally, this will store the name of the tracked
 * object that the file contatins to the given pointer.
 *
 * @param int fd - a file descripter pointing to the file.
 *		  char** name - a pointer to the char* where the name should be stored.
 *						if this is null, the name will be ignored.
 */
int tdl_prepare(int fd, char** name);

/*
 * Creates a new empty tdl file. This will setup 
 *
 * Proper header: 219  84  68  76  13  10  26  10
 * ASCII  header: INV   T   D   L  \r  \n \032 \n
 * We use this combination of invalid ascii and different returns to make
 * the file not openable with text editors since it is a binary file.
 *
 * @param char* path - the path to the file and the file name. 
 * 				       ".tdl" will be appended to the file name if
 *					   it is not specified in the path.
 *		  char* name - the name of the object that the file contains.
 *					   There is a limit of 32 chars for the object name,
 *					   anything longer will be truncated.
 *
 * @return int - a file descripter pointing to the newly created file.
 */
int tdl_create(const char* path, const char* name);


/*
 * Returns the next tracked point in the file.
 * If the end of the file is reached, the file cursor will be
 * moved back to the beginning of the file.
 *
 * @param int fd - a file descripter pointing to the file.
 *		  float* pos - the array that the position will be stored. Should be length 3.
 *		  float* orient - the array that the orientation will be stored. Should be length 16.
 *
 * @return int - -1 if the file could not be read.
 * 				  0 if the operation was normal.
 *				  1 if the end of the file was reached and the cursor was reset.
 *
 */
int tdl_read(int fd, float pos[3], float orient[9]);

/*
 * Writes the position and orientation properly formated to a file.
 *
 * @param int fd - the file to write to
 * 		  float* pos - the position array 
 * 		  float* orient - the orientation array
 *
 */
void tdl_write(int fd, float pos[3], float orient[9]);

/*
 * Checks the headers of a file to make sure it is a proper tdl file.
 * Note: the position of the cursor in the file MUST be at the CUR_START
 *
 * @param int fd - a file descripter pointing to the file.
 *		  float* pos - the array containing the position data. Should be length 3.
 *		  float* orient - the array containing the orientation data. Should be length 16.
 *
 * @return (C)int - 0 - the file is not valid.
 *					1 - the file is valid.
 *
 *		   (C++)bool - whether or not the file is valid
 */
#ifdef __cplusplus
bool tdl_validate(int fd);
#else
int tdl_validate(int fd);
#endif

#ifdef __cplusplus
} // end extern "C"
#endif
