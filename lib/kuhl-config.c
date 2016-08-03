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

static struct cfg_struct *cfg = NULL;
static char *cfg_filename = NULL;  /*< Filename that holds the configuration */

/** Set the configuration file to be used. If another configuration
    file is already loaded, it will be unloaded and the new file will
    later be loaded when a key is requested.

    @param filename The name of the configuration file to be
    loaded. If NULL, then any existing configuration file will be
    unloaded.
 */
void kuhl_config_filename(const char *filename)
{
	// If the user sets the filename to the file we are already using.
	if(cfg_filename != NULL && filename != NULL && strcmp(cfg_filename, filename) == 0)
		return;

	// Unload any settings we have already loaded.
	if(cfg != NULL)
	{
		cfg_free(cfg);
		cfg = NULL;
	}

	if(cfg_filename != NULL)
		msg(MSG_WARNING, "We have already loaded config file '%s' but we are now switching to file '%s'. This can happen when the program requests a configuration value and then kuhl_config_filename is called.", cfg_filename, filename);

	if(cfg_filename)
		free(cfg_filename);
	cfg_filename = strdup(filename);
}


/** Gets the value for a given key in the config file.

    @param key The key to look up in the config file.

    @return The value of the key as a string. If the key is missing,
    or if the key is present but its value is the empty string, return
    NULL.
*/
const char* kuhl_config_get(const char *key)
{
	if(cfg == NULL)
	{
		int using_defaultFile = 0;
		if(cfg_filename == NULL)
		{
			cfg_filename = "settings.ini";
			using_defaultFile = 1;
		}
		cfg = cfg_init();
		char *filename = kuhl_find_file(cfg_filename);
		if(cfg_load(cfg, filename, 1) == EXIT_FAILURE)
		{
			if(using_defaultFile)
				msg(MSG_INFO, "Failed to read default config file: %s\n", filename);
			else
				msg(MSG_ERROR, "Failed to read user-specified config file: %s\n", filename);
		}
		else
			msg(MSG_DEBUG, "Using settings file at: %s\n", filename);
		free(filename);

		while(cfg_get(cfg, "include") != NULL)
		{
			const char *include = cfg_get(cfg, "include");
			filename = kuhl_find_file(include);
			cfg_delete(cfg, "include");
			if(kuhl_can_read_file(filename))
			{
				msg(MSG_DEBUG, "Config file '%s' included '%s'.", cfg_filename, filename);
				cfg_load(cfg, filename, 0); // don't overwrite
			}
			else
			{
				msg(MSG_ERROR, "Config file '%s' included '%s', but it doesn't exist or isn't readable.", cfg_filename, filename);
			}
			free(filename);
		}
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
 * missing. Returns returnInvalidValue if the key is set to a
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
