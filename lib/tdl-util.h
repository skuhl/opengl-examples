/*
 * This file contains useful methods for reading, writing and creating Tracked Data Log (.tdl) files
 * @author John Thomas
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

int tdl_prepare(FILE *f, char** name);
FILE* tdl_create(const char* path, const char* name);
int tdl_read(FILE *f, float pos[3], float orient[9]);
void tdl_write(FILE *f, float pos[3], float orient[9]);
#ifdef __cplusplus
bool tdl_validate(FILE *f);
#else
int tdl_validate(FILE *f);
#endif

#ifdef __cplusplus
} // end extern "C"
#endif
