/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 *
 * This file is a wrapper around the cfg_parse.c/cfg_parse.h code
 * (which is in the public domain and published at
 * http://cfg-parse.sourceforge.net/
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


void kuhl_config_filename(const char *filename);
const char* kuhl_config_get(const char *key);
int kuhl_config_isset(const char *key);
int kuhl_config_boolean(const char *key, int returnWhenMissing, int returnInvalidValue);
float kuhl_config_float(const char *key, float returnWhenMissing, float returnInvalidValue);
int kuhl_config_int(const char *key, int returnWhenMissing, int returnInvalidValue);
	
#ifdef __cplusplus
}
#endif
