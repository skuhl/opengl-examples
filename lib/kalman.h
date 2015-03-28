#ifndef __kalman_h
#define __kalman_h

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    double p[9];
    double xk_prev[3];
    double qScale;
    double r;
    double h[3];
    double a[9];
	long time_prev;
	int isEnabled;
} kalman_state;

void kalman_initialize(kalman_state * state);
float kalman_estimate(kalman_state * state, float measured);

#ifdef __cplusplus
} // end extern "C"
#endif
#endif // __KALMAN_H__
