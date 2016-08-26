/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	int isEnabled; /**< If set to 0, disable kalman filter */
	int predictOnly; 
	
	double xk_prev[3]; /**< Filtered position, velocity, and acceleration */
	long time_prev;    /**< Time of previous measurement in milliseconds */

	double p[9];   /**< Estimated error of our current state */
	double qScale; /**< Scaling factor for Q matrix (system error) */
	double r;      /**< Variance of measurement error */
	double h[3];   /**< Measurement matrix (converts measurement into state) */
	double a[9];   /**< Transition matrix (moves state forward one step) */
} kalman_state;

void kalman_initialize(kalman_state * state, float sigma_meas, float qScale);
float kalman_estimate(kalman_state * state, float measured, long measured_time);

#ifdef __cplusplus
} // end extern "C"
#endif
