/* Copyright (c) 2016 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file

    bufferswap.c handles swapping of the front and back
    buffers. Although this is typically very simple, this file makes
    several useful improvements.

    * It implements "latency reduction". Typically, the rendering loop
      will be: (1) render graphics, (2) wait until vsync to swap
      buffers (if this feature is enabled), (3) swap buffers. The
      problem with this approach is that if the graphics are rendered
      quickly, there can be many milliseconds between when the
      graphics were rendered and when they are displayed---introducing
      latency. If we use "latency reduction", we will instead do (1)
      sleep just long enough so that we can render the graphics right
      before the vsync, (2) render graphics, (3) wait until vsync to
      swap buffers (hopefully not long!), (4) swap buffers.

    * It allows you to change the "Swap interval". Historically, you
      could only say "wait for vsync" to swap buffers (then your FPS
      is typically limited to 60fps or the refresh rate of your
      monitor, and if you take just over 1/60th of a second to render
      a frame, your FPS will probably drop to 30fps---but there is no
      "tearing" artifacts). Or, you could indicate that you OpenGL
      should never wait for vsync (allowing FPS to be unbounded---but
      there are "tearing" artifacts). The "swap interval" setting lets
      you say that OpenGL should wait for vsync if the frame rate is
      above the monitor refresh rate and to accept tearing artifacts
      if the FPS drops below the monitor refresh rate.

    * When DGR (distributed multihost rendering is used), we want to
      ensure that slaves receive data right before we try to render it
      and that the master node sends data as soon as
      possible. Therefore, bufferswap() calls dgr_update() to
      send/receive appropriately.

    * Monitors FPS and allows the user to retrieve the current FPS.
    
    @author Scott Kuhl
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void bufferswap(void);
float bufferswap_fps(void);

#ifdef __cplusplus
} // end extern "C"
#endif



	
