#include <string.h>
#include <math.h>

#include "kalman.h"
#include "vecmat.h"


static int kalmanIsEnabled = 0;
void kalman_enable(int isEnabled)
{
	kalmanIsEnabled = isEnabled;
}

// make a prediction based on a new measurement
float kalman_estimate(struct kalman_state * state,
                      float measured,
                      double time_difference)
{
	if(kalmanIsEnabled == 0)
		return measured;

	double dt = time_difference;
	{
		double row1[3] = { 1, dt, .5*dt*dt };
		double row2[3] = { 0, 1, dt };
		double row3[3] = { 0, 0, 1 };
		mat3d_setColumn(state->a, row1, 0);
		mat3d_setColumn(state->a, row2, 1);
		mat3d_setColumn(state->a, row3, 2);
	}

	double q[9];
	{
		double row1[3] = { pow(dt,5)/20, pow(dt,4)/8, pow(dt,3)/6 };
		double row2[3] = { pow(dt,4)/8, pow(dt,3)/3, pow(dt,2)/2 };
		double row3[3] = { pow(dt,3)/6, pow(dt,2)/2, dt };
		mat3d_setColumn(q, row1, 0);
		mat3d_setColumn(q, row2, 1);
		mat3d_setColumn(q, row3, 2);
	}
	for(int i=0; i<9; i++)
		q[i] = q[i] * state->qScale;

	// === PREDICTION ===

	// Project the state ahead
	// kx_minus = A * xk_prev     (no control input!)
	double xk_minus[3];
	mat3d_mult_vec3d_new(xk_minus, state->a, state->xk_prev);


	// Project the error covariance ahead.
	// Pminus = A * P * A^T + Q
	double a_transpose[9];
	mat3d_transpose_new(a_transpose, state->a);

	double a_dot_p[9];
	mat3d_mult_mat3d_new(a_dot_p, state->a, state->p);   //  A*P

	double p_minus[9];
	mat3d_mult_mat3d_new(p_minus, a_dot_p, a_transpose); // (A*P)*A^-1
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

	vec3d_copy(state->xk_prev, xk);
	return xk[0];
}

// The variables in the structure could be modified after they are
// initialized by the following function.
void kalman_initialize (struct kalman_state * state)
{
	memset(state, 0, sizeof(struct kalman_state));

	float sigma_model = 1; // confidence in current state (smaller=more confident)
	float sigma_meas = .1; // stddev of measurement noise

	// Variance of our measurements.  A small number indicates that our
	// measurements are noise-free.
	state->r = sigma_meas * sigma_meas;

	// Confidence in current state (changes as program runs).
	// Smaller numbers = more confidence.
	mat3d_identity(state->p);
	for(int i=0; i<9; i++)
		state->p[i] = state->p[i] * sigma_model;

	// qScale is a scalar to multiply the Q matrix by.  A smaller number
	// means that the filter is more likely to believe changes which
	// deviate from the predicted value.
	state->qScale = .1;

	// Guess for initial state
	state->xk_prev[0] = 1; // position
	state->xk_prev[1] = 0; // velocity
	state->xk_prev[2] = 0; // acceleration

	// Converts our state into the set of variables we are measuring.
	state->h[0] = 1;
}
