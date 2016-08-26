/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */
#include "windows-compat.h"
#include <string.h>
#include <math.h>

#include "kuhl-nodep.h"
#include "kalman.h"
#include "vecmat.h"



/** Given a fully initialized kalman_state object, and a new
 * measurement, get a filtered data point. The model behind this
 * kalman filter assumes a constant acceleration and also keeps track
 * of the points velocity.
 *
 * This function filters only a single 1D point. You would need to
 * call this function three different times with three different
 * kalman_state variables to filter X, Y, and Z.
 *
 * @param state An kalman_state struct initialized by kalman_initialize()
 *
 * @param measured The newest, unfiltered measurement.
 *
 * @param measured_time The time that 'measured' was recorded in
 * microseconds. If -1, we will use the current time.
 *
 * @return The filtered data.
 *
 */
float kalman_estimate(kalman_state * state, float measured, long measured_time)
{
	if(state->isEnabled == 0)
		return measured;

	if(measured_time == -1)
		measured_time = kuhl_microseconds();
	if(state->time_prev == -1)
		state->time_prev = measured_time-1;
	double dt = (measured_time - state->time_prev)/1000000.0;
	
	/* A is the transition matrix which will move our state ahead by
	 * one timestep. */
	{
		double row1[3] = { 1, dt, .5*dt*dt };
		double row2[3] = { 0, 1, dt };
		double row3[3] = { 0, 0, 1 };
		mat3d_setRow(state->a, row1, 0);
		mat3d_setRow(state->a, row2, 1);
		mat3d_setRow(state->a, row3, 2);
	}
	// mat3d_print(state->a);

	
	/* Q is the process/system noise covariance.

	   From pg 156 of "Fundamentals of Kalman filtering: a practical
	   approach" which provides tables where each state is a
	   derivative of the one above it and all of the noise enters into
	   the bottom-most state.  The resulting matrix can be scaled by a
	   scalar as needed (called the "continuous process-noise spectral
	   density") in the book.
	*/
	double q[9];
	{
		double row1[3] = { pow(dt,5)/20, pow(dt,4)/8, pow(dt,3)/6 };
		double row2[3] = { pow(dt,4)/8, pow(dt,3)/3, pow(dt,2)/2 };
		double row3[3] = { pow(dt,3)/6, pow(dt,2)/2, dt };
		mat3d_setRow(q, row1, 0);
		mat3d_setRow(q, row2, 1);
		mat3d_setRow(q, row3, 2);
	}
	for(int i=0; i<9; i++)
		q[i] = q[i] * state->qScale;
//	printf("%f\n", dt);
//	mat3d_print(q);

	// === PREDICTION ===

	// Project the state ahead
	// kx_minus = A * xk_prev     (no control input!)
	double xk_minus[3];
	mat3d_mult_vec3d_new(xk_minus, state->a, state->xk_prev);
	if(state->predictOnly)
		return xk_minus[0];

	// Project the error covariance ahead.
	// Pminus = A * P * A^T + Q
	double a_transpose[9];
	mat3d_transpose_new(a_transpose, state->a);

	double a_dot_p[9];
	mat3d_mult_mat3d_new(a_dot_p, state->a, state->p);   //  A*P

	double p_minus[9];
	mat3d_mult_mat3d_new(p_minus, a_dot_p, a_transpose); // (A*P)*A^T
	for(int i=0; i<9; i++)                               // add Q
		p_minus[i] = p_minus[i] + q[i];
	
	
	// === MEASUREMENT UPDATE or CORRECTION ===
	// Compute the Kalman gain
	// K = Pminus * transpose(H) * (H * Pminus * transpose(H) + R)^-1
	// 
	// Note: H is a column vector, transposing just changes it between
	// a vertical and horizontal matrix. Multiplying a rowVec * colVec
	// is the same as taking a dot product between them.

	// Pminus * transpose(H)
	double pminus_h[3];
	mat3d_mult_vec3d_new(pminus_h, p_minus, state->h);
	// H * (Pminus*transpose(H)) + R
	double s = vec3d_dot(state->h, pminus_h) + state->r;
	double inv_s = 1/(s);
	double k[3];
	vec3d_scalarMult_new(k, pminus_h, inv_s);
	//vec3d_print(k); // if k=0, relies 100% on predicted value


	// Update the estimate with the measurement. (Weights the current
	// measurement and our prediction to come up with a new value.)
	// 
	// x = x + K * ( obs - H * x )
	double paren = measured - vec3d_dot( state->h, xk_minus);
	double k_dot_paren[3];
	vec3d_scalarMult_new(k_dot_paren, k, paren);
	double xk[3];
	vec3d_add_new(xk, xk_minus, k_dot_paren);

	// Update the error covariance
	// P = P - (K * H) * P
	double k_mult_h[9];
	vec3d_mult_vec3d(k_mult_h, k, state->h); // K*H
	double subtrahend[9];
	mat3d_mult_mat3d_new(subtrahend, k_mult_h, p_minus);  // (K*H)*P
	for(int i=0; i<9; i++)
		state->p[i] = p_minus[i] - subtrahend[i]; // P = P-(K*H)*P

	// vec3d_print(xk); // Print current estimate of pos, velocity, accel
	state->time_prev = measured_time;
	vec3d_copy(state->xk_prev, xk);
	return (float) xk[0];
}

/** Initializes a kalman_state struct.

   @param state A pointer to a kalman_state struct which should be initialized.
   
   @param sigma_meas Standard deviation of the measurement noise.

   @param qScale A value near 0 indicates high confidence in our
   model. A larger value will cause the filter to better track large
   jumps in data. A value of 0 is not recommended.
*/
void kalman_initialize(kalman_state * state, float sigma_meas, float qScale)
{
	memset(state, 0, sizeof(kalman_state));

	state->isEnabled = 1;
	state->predictOnly = 0;
	state->time_prev = -1;

	float sigma_model = 1; // confidence in current state (smaller=more confident)

	/* kalman_estimate() creates a Q matrix appropriate for a model
	   where we are handling position, velocity, and
	   acceleration. However, we can scale this matrix to indicate how
	   confident we are in our model. Setting it to 0 indicates a
	   belief that our model is perfect. Making this value large will
	   cause the filter be willing to accept and follow large jumps in
	   the data.
	
	   In the case of a tracking, the user's movements is noise
	   because they will be moving in complex and unpredictable ways
	   which will not fit our model.  If we are assuming position is
	   changing due to velocity and velocity is changing due to
	   acceleration, and acceleration is fixed, errors in acceleration
	   will occur.
	*/
	state->qScale = qScale;
	
	// Variance of our measurements.  A small number indicates that our
	// measurements are noise-free.
	state->r = sigma_meas * sigma_meas;

	// Confidence in current state (changes as program runs).
	// Smaller numbers = more confidence.
	mat3d_identity(state->p);
	for(int i=0; i<9; i++)
		state->p[i] = state->p[i] * sigma_model;

	// Guess for initial state (position, velocity, acceleration)
	vec3d_set(state->xk_prev, 0, 0, 0);

	// Converts our state into the set of variables we are measuring.
	vec3d_set(state->h, 1,0,0);
}
