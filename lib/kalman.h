#ifndef __kalman_h
#define __kalman_h



struct kalman_state{
    double p[9];
    double xk_prev[3];
    double qScale;
    double r;
    double h[3];
    double a[9];
};

void kalman_initialize(struct kalman_state * state);
float kalman_estimate(struct kalman_state * state,
                      float measured, double time_difference);
void kalman_enable(int state);

#endif
