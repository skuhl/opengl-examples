#include <stdlib.h>
#include <stdio.h>
#include "vecmat.h"
#include "kuhl-util.h"


// Convert Euler angle to a matrix. Then, convert the matrix back to
// an Euler angle. The input and output Euler angles should match.
void test_angle_float(float a1, float a2, float a3, const char *order)
{
	float input[3] = { a1,a2,a3 };
	float input_mat[9];
	mat3f_rotateEuler_new(input_mat, input[0], input[1], input[2], order);

	float output[3];
	eulerf_from_mat3f(output, input_mat, order);
	float output_mat[9];
	mat3f_rotateEuler_new(output_mat, output[0], output[1], output[2], order);

	float diff[3];
	vec3f_sub_new(diff, output, input);

	if(vec3f_norm(diff) > .0001)
	{
		printf("order: %s (float)\n", order);
		printf("input:  ");
		vec3f_print(input);
		printf("output: ");
		vec3f_print(output);
		printf("diff:   ");
		vec3f_print(diff);

		float diff = 0;
		for(int i=0; i<9; i++)
			diff += fabsf(output_mat[i]-input_mat[i]);
		if(diff > .0001)
			printf("ERROR\n");
		else
			printf("Output Euler angles are different than the input, but are different representations of the same rotation.\n");
		
		printf("\n");
	}
	else
	{
#if 0
		printf("OK: ");
		vec3f_print(input);
#endif
	}
}



// Convert Euler angle to a matrix. Then, convert the matrix back to
// an Euler angle. The input and output Euler angles should match.
void test_angle_double(double a1, double a2, double a3, const char *order)
{
	double input[3] = { a1,a2,a3 };
	double input_mat[9];
	mat3d_rotateEuler_new(input_mat, input[0], input[1], input[2], order);
	
	double output[3];
	eulerd_from_mat3d(output, input_mat, order);
	double output_mat[9];
	mat3d_rotateEuler_new(output_mat, output[0], output[1], output[2], order);

	
	double diff[3];
	vec3d_sub_new(diff, output, input);

	if(vec3d_norm(diff) > .0001)
	{
		printf("order: %s (double)\n", order);
		printf("input:  ");
		vec3d_print(input);
		printf("output: ");
		vec3d_print(output);
		printf("diff:   ");
		vec3d_print(diff);

		double diff = 0;
		for(int i=0; i<9; i++)
			diff += fabs(output_mat[i]-input_mat[i]);
		if(diff > .0001)
			printf("ERROR\n");
		else
			printf("Output Euler angles are different than the input, but are different representations of the same rotation.\n");
		
		printf("\n");
	}
	else
	{
#if 0
		printf("OK: ");
		vec3d_print(input);
#endif
	}
}


void test_angle(float a1, float a2, float a3, const char *order)
{
//	test_angle_float(a1, a2, a3, order);
	test_angle_double(a1, a2, a3, order);
}


int main(void)
{
	char *orders[12] = { "XYZ", "XZY",
	                     "YXZ", "YZX",
	                     "ZXY", "ZYX",
	                     "XYX", "XZX",
	                     "YXY", "YZY",
	                     "ZXZ", "ZYZ" };

	for(int i=0; i<6; i++) // first & last axis are different - tait bryan angles
	{
		for(int j=0; j<10000; j++)
			test_angle(drand48()*360-180, drand48()*180-90, drand48()*360-180, orders[i]);

		// In many cases, we will get different output angles than
		// input angles when our angles are near the edges of the
		// normal ranges. However, the code should still produce
		// rotations that are equivalent (even though the Euler angles
		// might look different at first glance).
		test_angle( 180, 0, -180, orders[i]); // largest, middle, smallest
		test_angle(-180, 0,  180, orders[i]); // smallest, middle, largest
		test_angle(-180, 0, -180, orders[i]); // smallest, middle, smallest
		test_angle( 180, 0,  180, orders[i]); // largest, middle, largest

		// Numbers out of typical range
		test_angle(1000, 2000, 3000, orders[i]);
		
		// near gimbal lock
		test_angle(1, -90+.0001, 2, orders[i]);
		test_angle(1, 90-.0001, 2, orders[i]);

		// at gimbal lock, expect an equivalent rotation for input or
		// output, but the numbers might be different:
		test_angle(1, -90, 2, orders[i]);
		test_angle(1,  90, 2, orders[i]);
	}
	for(int i=6; i<12; i++)
	{
		for(int j=0; j<10000; j++)
			test_angle(drand48()*360-180, drand48()*180, drand48()*360-180, orders[i]);

		test_angle( 180, 90, -180, orders[i]); // largest, middle, smallest
		test_angle(-180, 90,  180, orders[i]); // smallest, middle, largest
		test_angle(-180, 90, -180, orders[i]); // smallest, middle, smallest
		test_angle( 180, 90,  180, orders[i]); // largest, middle, largest
		test_angle( 12, 90,  13, orders[i]); 

		// Numbers out of typical range
		test_angle(1000, 2000, 3000, orders[i]);
		
		// near gimbal lock
		test_angle(1, nextafterf(0,1), 2, orders[i]);
		test_angle(1, 180-.0001, 2, orders[i]);

		// at gimbal lock, expect an equivalent rotation for input or
		// output, but the numbers might be different:
		test_angle(1,    0, 2, orders[i]);
		test_angle(1,  180, 2, orders[i]);

	}

	printf("Rerun and grep the output for the string ERROR to determine if any real errors occurred\n");
}


	
