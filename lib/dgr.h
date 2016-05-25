/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/**
   @file

    DGR provides a framework for a master process to share data with slave processes via UDP packets on a network.

    @author Scott Kuhl
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void dgr_init(void);
void dgr_update(int send, int receive);
void dgr_setget(const char *name, void* buffer, int bufferSize);
void dgr_print_list(void);
int dgr_is_master(void);
int dgr_is_enabled(void);
	
#ifdef __cplusplus
} // end extern "C"
#endif
