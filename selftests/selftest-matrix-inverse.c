#include <stdlib.h>
#include <stdio.h>
#include "vecmat.h"

/* Invert the matrix. Multiply the original matrix by its
 * inverse. Ensure that the product is the identity matrix. */
void test_matrix_inverse_float(float mat[16])
{
	//mat4f_print(mat);
	float inv[16];
	mat4f_invert_new(inv, mat);
	//mat4f_print(inv);

	float result[16];
	mat4f_mult_mat4f_new(result, mat, inv);
	//mat4f_print(result);
	float identity[16];
	mat4f_identity(identity);

	float diff = 0;
	for(int i=0; i<16; i++)
		diff += fabsf(result[i]-identity[i]);

	if(diff > .001)
		printf("ERROR: %f\n", diff);
}

/* Invert the matrix. Multiply the original matrix by its
 * inverse. Ensure that the product is the identity matrix. */
void test_matrix_inverse_double(double mat[16])
{
	//mat4d_print(mat);
	double inv[16];
	mat4d_invert_new(inv, mat);
	//mat4d_print(inv);

	double result[16];
	mat4d_mult_mat4d_new(result, mat, inv);
	//mat4d_print(result);
	double identity[16];
	mat4d_identity(identity);

	double diff = 0;
	for(int i=0; i<16; i++)
		diff += fabs(result[i]-identity[i]);

	if(diff > .000000001)
		printf("ERROR: %f\n", diff);
}




void test_matrix_inverse(double mat[16])
{
	float matf[16];
	mat4f_from_mat4d(matf, mat);
	test_matrix_inverse_float(matf);

	test_matrix_inverse_double(mat);
}

	

int main(void)
{
	for(int i=0; i<10000; i++)
	{
		double mat[16];
		mat4d_rotateEuler_new(mat,
		                      drand48()*360,
		                      drand48()*360,
		                      drand48()*360, "XYZ");
		double trans[16];
		mat4d_translate_new(trans,
		                    (drand48()-.5)*1000,
		                    (drand48()-.5)*1000,
		                    (drand48()-.5)*1000);
		mat4d_mult_mat4d_new(mat, mat, trans);
		test_matrix_inverse(mat);
	}

	printf("This program will print out ERROR above if an error occurs.\n");
}
