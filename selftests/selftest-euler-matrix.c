#include <stdlib.h>
#include <stdio.h>
#include "vecmat.h"

/* Given a rotation matrix, convert to Euler angles, then convert
 * Euler angles back to a matrix. The resulting matrix should match
 * the original matrix. */
void test_euler_matrix_float(float mat[9])
{
	float angles[3];
	eulerf_from_mat3f(angles, mat, "XYZ");

	float result[9];
	mat3f_rotateEuler_new(result, angles[0], angles[1], angles[2], "XYZ");

	float diff = 0;
	for(int i=0; i<9; i++)
		diff += fabsf(mat[i]-result[i]);

	if(diff > .00001)
		printf("ERROR: %0.20f\n", diff);
}


void test_euler_matrix_double(double mat[9])
{
	double angles[3];
	eulerd_from_mat3d(angles, mat, "XYZ");

	double result[9];
	mat3d_rotateEuler_new(result, angles[0], angles[1], angles[2], "XYZ");

	double diff = 0;
	for(int i=0; i<9; i++)
		diff += fabs(mat[i]-result[i]);

	if(diff > .00000001)
		printf("ERROR: %0.20f\n", diff);
}



void test_euler_matrix(double mat[9])
{
	float matf[9];
	mat3f_from_mat3d(matf, mat);
	test_euler_matrix_float(matf);

	test_euler_matrix_double(mat);
}

	

int main(void)
{
	for(int i=0; i<10000; i++)
	{
		double mat[9];
		mat3d_rotateEuler_new(mat,
		                      drand48()*360,
		                      drand48()*360,
		                      drand48()*360, "XYZ");
		test_euler_matrix(mat);
	}

	printf("This program will print out ERROR above if an error occurs.\n");
}
