/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 *
 * This code makes it easy to read settings that are stored in an ini
 * file.
 *
 * This file is a wrapper around the cfg_parse.c/cfg_parse.h code
 * (which is in the public domain and published at
 * http://cfg-parse.sourceforge.net/ ).
 *
 */


#include <string.h>
#include <stdlib.h>
#include "kuhl-util.h"
#include "cfg_parse.h"

/** Gets the value for a given key in the config file.

    @param key The key to look up in the config file.

    @return The value of the key as a string. If the key is missing,
    or if the key is present but its value is the empty string, return
    NULL.
*/
const char* kuhl_config_get(const char *key)
{
	static struct cfg_struct *cfg = NULL;
	if(cfg == NULL)
	{
		cfg = cfg_init();
		char *filename = kuhl_find_file("settings.ini");
		if(cfg_load(cfg, filename) == EXIT_FAILURE)
			msg(MSG_INFO, "Failed to read settings.ini\n");
		else
			msg(MSG_DEBUG, "Using settings file at: %s\n", filename);
	}

	const char *value = cfg_get(cfg, key);
	if(value != NULL && strlen(value) == 0)
		value = NULL;
	return value;
}

/** Checks if a key is set to a value in the config file.

    @return Returns 1 if the key is present and set to a non-empty
    string. Returns 0 if the key is missing or if it is set to an
    empty string. */
int kuhl_config_isset(const char *key)
{
	return (kuhl_config_get(key) == NULL);
}

/** Returns 1 if the key is set to true in the config file. Returns 0
 * if it is set to false. Returns returnWhenMissing if the key is
 * missing. Retruns returnInvalidValue if the key is set to a
 * non-boolean value. */
int kuhl_config_boolean(const char *key, int returnWhenMissing, int returnInvalidValue)
{
	const char *value = kuhl_config_get(key);
	if(value == NULL)
		return returnWhenMissing;
	else if(strcasecmp(value, "true") == 0 ||
	   strcasecmp(value, "yes") == 0 ||
	   strcasecmp(value, "y") == 0 ||
	   strcasecmp(value, "t") == 0 ||
	   strcmp(value, "1") == 0)
		return 1;
	else if(strcasecmp(value, "false") == 0 ||
	        strcasecmp(value, "no") == 0 ||
	        strcasecmp(value, "n") == 0 ||
	        strcasecmp(value, "f") == 0 ||
	        strcmp(value, "0") == 0)
		return 0;
	else
		return returnInvalidValue;
}

/** Reads a floating point number from the config file. Returns returnWhenMissing if the key is missing. Returns returnInvalidValue if they key is set to a non-floating point number. */
float kuhl_config_float(const char *key, float returnWhenMissing, float returnInvalidValue)
{
	const char *value = kuhl_config_get(key);
	if(value == NULL)
		return returnWhenMissing;

	float f;
	if(sscanf(value, "%f", &f) == 1)
		return f;
	else
		return returnInvalidValue;
}

/** Reads a floating point number from the config file. Returns returnWhenMissing if the key is missing. Returns returnInvalidValue if they key is set to a non-integer. */
int kuhl_config_int(const char *key, int returnWhenMissing, int returnInvalidValue)
{
	const char *value = kuhl_config_get(key);
	if(value == NULL)
		return returnWhenMissing;

	int i;
	if(sscanf(value, "%d", &i) == 1)
		return i;
	else
		return returnInvalidValue;
}
