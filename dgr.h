/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/**
   @file

    DGR provides a framework for a master process to share data with slave processes via UDP packets on a network.

    @author Scott Kuhl
 */

void dgr_init();
void dgr_update();
void dgr_setget(const char *name, void* buffer, int bufferSize);
void dgr_print_list();
int dgr_is_master();
int dgr_is_enabled();
