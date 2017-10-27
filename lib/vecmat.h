/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 
General tips for learning the vector and matrix functions provided by this file:

   Matrices are assumed to be column-major (like most OpenGL manuals do).

   This API implements both float and double values for OpenGL. For example, a function that operates on a "float vec[3];" will be named vec3f_something(). A 4x4 matrix would be declared as "float mat[16];" and the functions that operate on the matrix are named mat4f_something(). Matrices are implemented as 1D arrays.

   Many of the functions have the inline keyword to encourage the compiler to inline the code. Note that inline functions have the code in the header file and something that looks like a prototype line in the C file (the reverse of normal!)

   If a calculation returns a single value, the corresponding function in this file will return that value. If a calculation returns a matrix or a vector, the destination is the first argument. For example, to take the cross product of two vectors with 3 float elements (written mathematically: C = A cross B) you could use vec3f_cross_new(C, A, B);

   Some functions end with "_new" to make it clear that the first argument is not part of the calculation and is simply the place where the result of the calculation is stored. For example, mat4f_invert_new(destMatrix, sourceMatrix) will invert sourceMatrix and store it in destMatrix. However, mat4f_invert(matrix) will invert matrix in place.

   Some functions include the "Vec" in their names such as: mat4f_translate_new() and mat4f_translateVec_new(). Both of these functions do the same thing but the first version takes a list of numbers as a parameter and the second one takes an array of numbers.
 
*/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#include <string.h> /* needed for memcpy() */

#include "list.h"
#include "msg.h"


#ifndef M_PI
/** M_PI is an approximation of pi. This variable is often available
 * in C without defining it yourself, but it isn't guaranteed to exist
 * according to the C standard. For more information, see:
 * http://c-faq.com/fp/mpi.html */
#define M_PI 3.14159265358979323846
#endif

/* https://stackoverflow.com/questions/32851292/vs2015-cannot-fathom-static-inline-functions */
#if defined _MSC_VER && defined __EDG__ && !defined __cplusplus
#define inline
#endif

/** Set the values in a 3-component float vector */
static inline void vec3f_set(float  v[3], float  a, float  b, float  c)
{ v[0]=a; v[1]=b; v[2]=c; }
/** Set the values in a 3-component double vector */
static inline void   vec3d_set(double v[3], double a, double b, double c)
{ v[0]=a; v[1]=b; v[2]=c; }
/** Set the values in a 4-component float vector */
static inline void   vec4f_set(float  v[4], float  a, float  b, float  c, float  d)
{ v[0]=a; v[1]=b; v[2]=c; v[3]=d; }
/** Set the values in a 4-component double vector */
static inline void   vec4d_set(double v[4], double a, double b, double c, double d)
{ v[0]=a; v[1]=b; v[2]=c; v[3]=d; }

/** Copy the contents of one float vector in to another vector.
 * @param result An array to copy values into.
 * @param a An array to copy values out of.
 * @param n The length of the array.
 */
static inline void vecNf_copy(float result[ ], const float a[ ], const int n)
{
	memcpy(result, a, n*sizeof(float));
}
/** Copy the contents of one double vector in to another vector.
 * @param result An array to copy values into.
 * @param a An array to copy values out of.
 * @param n The length of the array.
 */
static inline void vecNd_copy(double result[ ], const double a[ ], const int n)
{
	memcpy(result, a, n*sizeof(double));
}
/** Copy the contents 3-component float vector into another vector.
 * @param result An array to copy values into.
 * @param a An array to copy values out of.
 */
static inline void vec3f_copy(float result[3], const float a[3])
{ vecNf_copy(result, a, 3); }
/** Copy the contents 3-component double vector into another vector.
 * @param result An array to copy values into.
 * @param a An array to copy values out of.
 */
static inline void vec3d_copy(double result[3], const double a[3])
{ vecNd_copy(result, a, 3); }
/** Copy the contents 4-component float vector into another vector.
 * @param result An array to copy values into.
 * @param a An array to copy values out of.
 */
static inline void vec4f_copy(float result[4], const float a[4])
{ vecNf_copy(result, a, 4); }
/** Copy the contents 4-component double vector into another vector.
 * @param result An array to copy values into.
 * @param a An array to copy values out of.
 */
static inline void vec4d_copy(double result[4], const double a[4])
{ vecNd_copy(result, a, 4); }


/** Print an N-component float vector into a string. String is null
    terminated. It does NOT end with a space or a newline.

    @param dest A buffer to store the string in. It must be allocated
    to contain at least destSize bytes.

    @param destSize The amount of space available in dest.

    @param v A vector to print to the string.

    @param n The number of components in the vector.
*/
static inline void vecNf_print_to_string(char *dest, const int destSize,
                                         const float v[ ], const int n)
{
	int used = 0;
	used += snprintf(dest+used, destSize-used, "vec%df(", n);
	for(int i=0; i<n; i++)
	{
		// Use a temporary variable since printf() parameters are not const.
		float tmp = v[i];
		used += snprintf(dest+used, destSize-used, "%10.3f ",tmp);
	}
	used += snprintf(dest+used, destSize-used, ")");
}

/** Print an N-component double vector into a string. String is null
    terminated. It does NOT end with a space or a newline.

    @param dest A buffer to store the string in. It must be allocated
    to contain at least destSize bytes.

    @param destSize The amount of space available in dest.

    @param v A vector to print to the string.

    @param n The number of components in the vector.
*/
static inline void vecNd_print_to_string(char *dest, const int destSize,
                                         const double v[ ], const int n)
{
	int used = 0;
	used += snprintf(dest+used, destSize-used, "vec%dd(", n);
	for(int i=0; i<n; i++)
	{
		// Use a temporary variable since printf() parameters are not const.
		double tmp = v[i];
		used += snprintf(dest+used, destSize-used, "%10.3f ",tmp);
	}
	used += snprintf(dest+used, destSize-used, ")");
}

/** Print an N-component float vector to stdout (all on one line).
    @param v The N-component vector to print.
    @param n The number of components in the vector.
*/
static inline void vecNf_print(const float v[ ], const int n)
{
	char str[128];
	vecNf_print_to_string(str, 128, v, n);
	printf("%s\n", str);
}

/** Print an N-component double vector to stdout (all on one line).
    @param v The N-component vector to print.
    @param n The number of components in the vector.
*/
static inline void vecNd_print(const double v[ ], const int n)
{
	char str[128];
	vecNd_print_to_string(str, 128, v, n);
	printf("%s\n", str);
}

/** Print a vec3f vector to stdout (all on one line).
    @param v The vector to print.
*/
static inline void vec3f_print(const float v[3])
{ vecNf_print(v, 3); }
/** Print a vec3d vector to stdout (all on one line).
    @param v The vector to print.
*/
static inline void vec3d_print(const double v[3])
{ vecNd_print(v, 3); }
/** Print a vec4f vector to stdout (all on one line).
    @param v The vector to print.
*/
static inline void vec4f_print(const float v[4])
{ vecNf_print(v, 4); }
/** Print a vec4d vector to stdout (all on one line).
    @param v The vector to print.
*/
static inline void vec4d_print(const double v[4])
{ vecNd_print(v, 4); }


/** Cross product of two 3-component float vectors.
 * @param result An array to store the resulting calculations. It is OK if result points to the same location as A or B.
 * @param A The vector for the left side of the cross product.
 * @param B The vector for the right side of the cross product.
 */
static inline void vec3f_cross_new(float result[3], const float A[3], const float B[3])
{
	float temp[3];
	temp[0] = A[1]*B[2] - A[2]*B[1];
	temp[1] = A[2]*B[0] - A[0]*B[2];
	temp[2] = A[0]*B[1] - A[1]*B[0];
	vec3f_copy(result, temp);
}
/** Cross product of two 3-component double vectors.
 * @param result An array to store the resulting calculations. It is OK if result points to the same location as A or B.
 * @param A The vector for the left side of the cross product.
 * @param B The vector for the right side of the cross product.
 */
static inline void vec3d_cross_new(double result[3], const double A[3], const double B[3])
{
	double temp[3];
	temp[0] = A[1]*B[2] - A[2]*B[1];
	temp[1] = A[2]*B[0] - A[0]*B[2];
	temp[2] = A[0]*B[1] - A[1]*B[0];
	vec3d_copy(result, temp);
}

/** Calculates the dot product of two float vectors.
 *
 * @param A One of the vectors to take the dot product of.
 * @param B The other vector to take the dot product of.
 * @param n The number of components in the vectors.
 * @return The dot product of the vectors.
 */
static inline float vecNf_dot(const float A[ ], const float B[ ], const int n)
{
	float sum = 0.0f;
	for(int i=0; i<n; i++)
		sum += A[i]*B[i];
	return sum;
}
/** Calculates the dot product of two double vectors.
 *
 * @param A One of the vectors to take the dot product of.
 * @param B The other vector to take the dot product of.
 * @param n The number of components in the vectors.
 * @return The dot product of the vectors.
 */
static inline double vecNd_dot(const double A[ ], const double B[ ], const int n)
{
	double sum = 0.0;
	for(int i=0; i<n; i++)
		sum += A[i]*B[i];
	return sum;
}
/** Calculates the dot product of two 3-component float vectors.
 *
 * @param A One of the vectors to take the dot product of.
 * @param B The other vector to take the dot product of.
 * @return The dot product of the vectors.
 */
static inline float vec3f_dot(const float A[3], const float B[3])
{ return A[0]*B[0] + A[1]*B[1] + A[2]*B[2]; }
/** Calculates the dot product of two 3-component double vectors.
 *
 * @param A One of the vectors to take the dot product of.
 * @param B The other vector to take the dot product of.
 * @return The dot product of the vectors.
 */
static inline double vec3d_dot(const double A[3], const double B[3])
{ return A[0]*B[0] + A[1]*B[1] + A[2]*B[2]; }
/** Calculates the dot product of two 4-component double
 * vectors. Ignores the 4th component.
 *
 * @param A One of the vectors to take the dot product of.
 * @param B The other vector to take the dot product of.
 * @return The dot product of the vectors.
 */
static inline float vec4f_dot(const float A[4], const float B[4])
{ return A[0]*B[0] + A[1]*B[1] + A[2]*B[2] + A[3]*B[3]; }
/** Calculates the dot product of two 4-component double
 * vectors. Ignores the 4th component.
 *
 * @param A One of the vectors to take the dot product of.
 * @param B The other vector to take the dot product of.
 * @return The dot product of the vectors.
 */
static inline double vec4d_dot(const double A[4], const double B[4])
{ return A[0]*B[0] + A[1]*B[1] + A[2]*B[2] + A[3]*B[3]; }


/** Get the index of the value at a specific row and column in an NxN matrix.
    @param row The row that the value is at in the matrix (0=first row).
    @param col The column that the value is at in the matrix (0=first column).
    @param n The size of the matrix (use 3 for a 3x3 matrix)
    @return The index that the value is at in the matrix array. */
static inline int matN_getIndex(const int row, const int col, const int n)
{ return row+col*n; }
/** Get the index of the value at a specific row and column in an 3x3 matrix.
    @param row The row that the value is at in the matrix (0=first row).
    @param col The column that the value is at in the matrix (0=first column).
    @return The index that the value is at in the matrix array. */
static inline int mat3_getIndex(const int row, const int col)
{ return matN_getIndex(row, col, 3); }
/** Get the index of the value at a specific row and column in an 4x4 matrix.
    @param row The row that the value is at in the matrix (0=first row).
    @param col The column that the value is at in the matrix (0=first column).
    @return The index that the value is at in the matrix array. */
static inline int mat4_getIndex(const int row, const int col)
{ return matN_getIndex(row, col, 4); }
/** Get the index of the value at a specific row and column in an 3x3 float matrix. Equivalent to mat3_getIndex().
    @param row The row that the value is at in the matrix (0=first row).
    @param col The column that the value is at in the matrix (0=first column).
    @return The index that the value is at in the matrix array. */
static inline int mat3f_getIndex(const int row, const int col)
{ return matN_getIndex(row, col, 3); }
/** Get the index of the value at a specific row and column in an 4x4 float matrix. Equivalent to mat4_getIndex().
    @param row The row that the value is at in the matrix (0=first row).
    @param col The column that the value is at in the matrix (0=first column).
    @return The index that the value is at in the matrix array. */
static inline int mat4f_getIndex(const int row, const int col)
{ return matN_getIndex(row, col, 4); }
/** Get the index of the value at a specific row and column in an 3x3 double matrix. Equivalent to mat3_getIndex().
    @param row The row that the value is at in the matrix (0=first row).
    @param col The column that the value is at in the matrix (0=first column).
    @return The index that the value is at in the matrix array. */
static inline int mat3d_getIndex(const int row, const int col)
{ return matN_getIndex(row, col, 3); }
/** Get the index of the value at a specific row and column in an 4x4 double matrix. Equivalent to mat4_getIndex().
    @param row The row that the value is at in the matrix (0=first row).
    @param col The column that the value is at in the matrix (0=first column).
    @return The index that the value is at in the matrix array. */
static inline int mat4d_getIndex(const int row, const int col)
{ return matN_getIndex(row, col, 4); }

	
/** Multiplies a column vector by a row vector to produce a matrix.
 *
 * @param m The resulting matrix.
 * @param A Column vector.
 * @param B Row vector.
 * @param n Number of components in the vector.
 */
static inline void vecNf_mult_vecNf(float  m[ ],  const float  A[ ], const float  B[ ], const int n)
{
	for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
	        m[matN_getIndex(i,j,n)] = A[i] * B[j];
        }
    }
}
/** Multiplies a column vector by a row vector to produce a matrix.
 *
 * @param m The resulting matrix.
 * @param A Column vector.
 * @param B Row vector.
 * @param n Number of components in the vector.
 */
static inline void vecNd_mult_vecNd(double m[ ],  const double A[ ], const double B[ ], const int n)
{
	for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
	        m[matN_getIndex(i,j,n)] = A[i] * B[j];
        }
    }
}
/** Multiplies a column vector by a row vector to produce a matrix.
 *
 * @param m The resulting matrix.
 * @param A Column vector.
 * @param B Row vector.
 * @param n Number of components in the vector.
 */
static inline void vec3f_mult_vec3f(float  m[9],  const float  A[3], const float  B[3])
{ vecNf_mult_vecNf(m, A, B, 3); }
/** Multiplies a column vector by a row vector to produce a matrix.
 *
 * @param m The resulting matrix.
 * @param A Column vector.
 * @param B Row vector.
 * @param n Number of components in the vector.
 */
static inline void vec3d_mult_vec3d(double m[9],  const double A[3], const double B[3])
{ vecNd_mult_vecNd(m, A, B, 3); }
/** Multiplies a column vector by a row vector to produce a matrix.
 *
 * @param m The resulting matrix.
 * @param A Column vector.
 * @param B Row vector.
 * @param n Number of components in the vector.
 */
static inline void vec4f_mult_vec4f(float  m[16], const float  A[4], const float  B[4])
{ vecNf_mult_vecNf(m, A, B, 4); }
/** Multiplies a column vector by a row vector to produce a matrix.
 *
 * @param m The resulting matrix.
 * @param A Column vector.
 * @param B Row vector.
 * @param n Number of components in the vector.
 */
static inline void vec4d_mult_vec4d(double m[16], const double A[4], const double B[4])
{ vecNd_mult_vecNd(m, A, B, 4); }

	
/** Calculate the norm squared or length squared of a 3-component float vector.
 * @param A The vector to calculate the length of.
 * @return The length*length of the vector.
 */
static inline float vec3f_normSq(const float A[3])
{ return vec3f_dot(A,A); }
/** Calculate the norm squared or length squared of a 3-component double vector.
 * @param A The vector to calculate the length of.
 * @return The length*length of the vector.
 */
static inline double vec3d_normSq(const double A[3])
{ return vec3d_dot(A,A); }
/** Calculate the norm squared or length squared of a 4-component float vector.
 * @param A The vector to calculate the length of.
 * @return The length*length of the vector.
 */
static inline float vec4f_normSq(const float A[4])
{ return vec4f_dot(A,A); }
/** Calculate the norm squared or length squared of a 4-component double vector.
 * @param A The vector to calculate the length of.
 * @return The length*length of the vector.
 */
static inline double vec4d_normSq(const double A[4])
{ return vec4d_dot(A,A); }

/** Calculate the norm or length of a 3-component float vector.
 * @param A The vector to calculate the length of.
 * @return The length of the vector.
 */
static inline float vec3f_norm(const float A[3])
{ return sqrtf(vec3f_dot(A,A)); }

/** Calculate the norm or length of a 3-component double vector.
 * @param A The vector to calculate the length of.
 * @return The length of the vector.
 */
static inline double vec3d_norm(const double A[3])
{ return sqrt(vec3d_dot(A,A)); }
/** Calculate the norm or length of a 4-component float vector.
 * @param A The vector to calculate the length of.
 * @return The length of the vector.
 */
static inline float vec4f_norm(const float A[4])
{ return sqrtf(vec4f_dot(A,A)); }
/** Calculate the norm or length of a 4-component double vector.
 * @param A The vector to calculate the length of.
 * @return The length of the vector.
 */
static inline double vec4d_norm(const double A[4])
{ return sqrt(vec4d_dot(A,A)); }

/** Divide each element in a float vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 * @param n Number of components in vector. Can't be larger than 16.
 */
static inline void vecNf_scalarDiv_new(float result[], const float v[], const float scalar, const int n)
{
	/* If result and v point to the same place, we need a copy of v
	 * that we can print if we divided by zero to aid in debugging. */
	float vOrig[16];
	vecNf_copy(vOrig, v, n);
	for(int i=0; i<n; i++)
		result[i] = v[i]/scalar;
	if(!isfinite(result[0]))
	{
		msg(MSG_WARNING, "Divided the following vector by 0:\n");
		char str[128];
		vecNf_print_to_string(str, 128, vOrig, n);
		msg(MSG_WARNING, "%s", str);
	}
}
/** Divide each element in a double vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 * @param n Number of components in vector. Can't be larger than 16. */
static inline void vecNd_scalarDiv_new(double result[], const double v[], const double scalar, const int n)
{
	/* If result and v point to the same place, we need a copy of v
	 * that we can print if we divided by zero to aid in debugging. */
	double vOrig[16];
	vecNd_copy(vOrig, v, n);
	for(int i=0; i<n; i++)
		result[i] = v[i]/scalar;
	if(!isfinite(result[0]))
	{
		msg(MSG_WARNING, "Divided the following by 0:\n");
		char str[128];
		vecNd_print_to_string(str, 128, vOrig, n);
		msg(MSG_WARNING, "%s", str);
	}
}
/** Divide each element in a 3-component float vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 */
static inline void vec3f_scalarDiv_new(float result[3], const float v[3], const float scalar)
{ vecNf_scalarDiv_new(result, v, scalar, 3); }
/** Divide each element in a 3-component double vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 */
static inline void vec3d_scalarDiv_new(double result[3], const double v[3], const double scalar)
{ vecNd_scalarDiv_new(result, v, scalar, 3); }
/** Divide each element in a 4-component float vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 */
static inline void vec4f_scalarDiv_new(float result[4], const float v[4], const float scalar)
{ vecNf_scalarDiv_new(result, v, scalar, 4); }
/** Divide each element in a 4-component double vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 */
static inline void vec4d_scalarDiv_new(double result[4], const double v[4], const double scalar)
{ vecNd_scalarDiv_new(result, v, scalar, 4); }

/** Divide each element in a float vector by a scalar in place.
 * @param v Input vector to be replaced with the new result.
 * @param scalar Value to divide by.
 * @param n Number of components in the vector.
 */
static inline void vecNf_scalarDiv(float  v[ ], const float  scalar, const int n)
{ vecNf_scalarDiv_new(v, v, scalar, n); }
/** Divide each element in a double vector by a scalar in place.
 * @param v Input vector to be replaced with the new result.
 * @param scalar Value to divide by.
 * @param n Number of components in the vector.
 */
static inline void vecNd_scalarDiv(double v[ ], const double scalar, const int n)
{ vecNd_scalarDiv_new(v, v, scalar, n); }
/** Divide each element in a 3-component float vector by a scalar in place.
 * @param v Input vector to be replaced with the new result.
 * @param scalar Value to divide by.
 */
static inline void vec3f_scalarDiv(float  v[3], const float  scalar)
{ vec3f_scalarDiv_new(v, v, scalar); }
/** Divide each element in a 3-component double vector by a scalar in place.
 * @param v Input vector to be replaced with the new result.
 * @param scalar Value to divide by.
 */
static inline void vec3d_scalarDiv(double v[3], const double scalar)
{ vec3d_scalarDiv_new(v, v, scalar); }
/** Divide each element in a 4-component float vector by a scalar in place.
 * @param v Input vector to be replaced with the new result.
 * @param scalar Value to divide by.
 */
static inline void vec4f_scalarDiv(float  v[4], const float  scalar)
{ vec4f_scalarDiv_new(v, v, scalar); }
/** Divide each element in a 4-component double vector by a scalar in place.
 * @param v Input vector to be replaced with the new result.
 * @param scalar Value to divide by.
 */
static inline void vec4d_scalarDiv(double v[4], const double scalar)
{ vec4d_scalarDiv_new(v, v, scalar); }

/** Multiply each element in a float vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 * @param n Number of components in vector. */
static inline void vecNf_scalarMult_new(float result[], const float v[], const float scalar, const int n)
{
	for(int i=0; i<n; i++)
		result[i] = v[i]*scalar;
}
/** Multiply each element in a double vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 * @param n Number of components in vector. */
static inline void vecNd_scalarMult_new(double result[], const double v[], const double scalar, const int n)
{
	for(int i=0; i<n; i++)
		result[i] = v[i]*scalar;
}
/** Multiply each element in a 3-component float vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 */
static inline void vec3f_scalarMult_new(float result[3], const float v[3], const float scalar)
{ vecNf_scalarMult_new(result, v, scalar, 3); }
/** Multiply each element in a 3-component double vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 */
static inline void vec3d_scalarMult_new(double result[3], const double v[3], const double scalar)
{ vecNd_scalarMult_new(result, v, scalar, 3); }
/** Multiply each element in a 4-component float vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 */
static inline void vec4f_scalarMult_new(float result[4], const float v[4], const float scalar)
{ vecNf_scalarMult_new(result, v, scalar, 4); }
/** Multiply each element in a 4-component double vector by a scalar.
 * @param result Resulting vector
 * @param v Input vector
 * @param scalar Value to divide by.
 */
static inline void vec4d_scalarMult_new(double result[4], const double v[4], const double scalar)
{ vecNd_scalarMult_new(result, v, scalar, 4); }

/** Multiply each element in a float vector by a scalar in place.
 * @param v Input vector to be replaced by the new values.
 * @param scalar Value to divide by.
 * @param n Number of components in vector. */
static inline void vecNf_scalarMult(float  v[ ], const float  scalar, const int n)
{ vecNf_scalarMult_new(v, v, scalar, n); }
/** Multiply each element in a double vector by a scalar in place.
 * @param v Input vector to be replaced by the new values.
 * @param scalar Value to divide by.
 * @param n Number of components in vector. */
static inline void vecNd_scalarMult(double v[ ], const double scalar, const int n)
{ vecNd_scalarMult_new(v, v, scalar, n); }
/** Multiply each element in a 3-component float vector by a scalar in place.
 * @param v Input vector to be replaced by the new values.
 * @param scalar Value to divide by.
 */
static inline void vec3f_scalarMult(float  v[3], const float  scalar)
{ vec3f_scalarMult_new(v, v, scalar); }
/** Multiply each element in a 3-component double vector by a scalar in place.
 * @param v Input vector to be replaced by the new values.
 * @param scalar Value to divide by.
 */
static inline void vec3d_scalarMult(double v[3], const double scalar)
{ vec3d_scalarMult_new(v, v, scalar); }
/** Multiply each element in a 4-component float vector by a scalar in place.
 * @param v Input vector to be replaced by the new values.
 * @param scalar Value to divide by.
 */
static inline void vec4f_scalarMult(float  v[4], const float  scalar)
{ vec4f_scalarMult_new(v, v, scalar); }
/** Multiply each element in a 4-component double vector by a scalar in place.
 * @param v Input vector to be replaced by the new values.
 * @param scalar Value to divide by.
 */
static inline void vec4d_scalarMult(double v[4], const double scalar)
{ vec4d_scalarMult_new(v, v, scalar); }

/** Normalize a 3-component float vector and store result at a new location.
 * @param dest The new normalized vector.
 * @param src The input vector.
 */
static inline void vec3f_normalize_new(float  dest[3], const float  src[3])
{ float  len = vec3f_norm(src); vec3f_scalarDiv_new(dest, src, len); }
/** Normalize a 3-component double vector and store result at a new location.
 * @param dest The new normalized vector.
 * @param src The input vector.
 */
static inline void vec3d_normalize_new(double dest[3], const double src[3])
{ double len = vec3d_norm(src); vec3d_scalarDiv_new(dest, src, len); }
/** Normalize a 4-component float vector and store result at a new location.
 * @param dest The new normalized vector.
 * @param src The input vector.
 */
static inline void vec4f_normalize_new(float  dest[4], const float  src[4])
{ float  len = vec4f_norm(src); vec4f_scalarDiv_new(dest, src, len); }
/** Normalize a 4-component double vector and store result at a new location.
 * @param dest The new normalized vector.
 * @param src The input vector.
 */
static inline void vec4d_normalize_new(double dest[4], const double src[4])
{ double len = vec4d_norm(src); vec4d_scalarDiv_new(dest, src, len); }


/** Normalize a quaternion (float) and store the result in a new location.
 * @param dest The new normalized quaternion.
 * @param src The input quaternion.
 */
static inline void quatf_normalize_new(float  dest[4], const float  src[4])
{ vec4f_normalize_new(dest, src); }
/** Normalize a quaternion (double) and store the result in a new location.
 * @param dest The new normalized quaternion.
 * @param src The input quaternion.
 */
static inline void quatd_normalize_new(double dest[4], const double src[4])
{ vec4d_normalize_new(dest, src); }


	
/** Normalize a 3-component float vector in place.
 * @param v The vector that will be normalized in place. */
static inline void vec3f_normalize(float v[3])
{ float  len = vec3f_norm(v); vec3f_scalarDiv(v, len); }
/** Normalize a 3-component double vector in place.
 * @param v The vector that will be normalized in place. */
static inline void vec3d_normalize(double v[3])
{ double len = vec3d_norm(v); vec3d_scalarDiv(v, len); }
/** Normalize a 4-component float vector in place.
 * @param v The vector that will be normalized in place. */
static inline void vec4f_normalize(float v[4])
{ float  len = vec4f_norm(v); vec4f_scalarDiv(v, len); }
/** Normalize a 4-component double vector in place.
 * @param v The vector that will be normalized in place. */
static inline void vec4d_normalize(double v[4])
{ double len = vec4d_norm(v); vec4d_scalarDiv(v, len); }
/** Normalize a quaternion in place.
 * @param v The quaternion to normalize in place. */
static inline void quatf_normalize(float v[4])
{ vec4f_normalize(v); }
/** Normalize a quaternion in place.
 * @param v The quaternion to normalize in place. */
static inline void quatd_normalize(double v[4])
{ vec4d_normalize(v); }

	
/** Homogenize a 4-element float vector and store the result in a new
 * location. Homogenization divides all elements in the array by the
 * 4th component.
 * @param dest The new homogenized vector.
 * @param src The vector to be homogenized.
 */
static inline void vec4f_homogenize_new(float dest[4], const float  src[4])
{ vec4f_scalarDiv_new(dest, src, src[3]); }
/** Homogenize a 4-element double vector and store the result in a new
 * location. Homogenization divides all elements in the array by the
 * 4th component.
 * @param dest The new homogenized vector.
 * @param src The vector to be homogenized.
 */
static inline void vec4d_homogenize_new(double dest[4], const double src[4])
{ vec4d_scalarDiv_new(dest, src, src[3]); }

/** Homogenize a 4-element float vector in place. Homogenization
 * divides all elements in the array by the 4th component.
 * @param v The new homogenized vector to be replaced with a new
 * homogenized vector.
 */
static inline void vec4f_homogenize(float  v[4])
{ vec4f_scalarDiv(v, v[3]); }
/** Homogenize a 4-element double vector in place. Homogenization
 * divides all elements in the array by the 4th component.
 * @param v The new homogenized vector to be replaced with a new
 * homogenized vector.
 */
static inline void vec4d_homogenize(double v[4])
{ vec4d_scalarDiv(v, v[3]); }

/** Add two float vectors together and store the result in a new location.
 * @param result The sum of the two input vectors.
 * @param a One of the input vectors.
 * @param b The other input vector.
 * @param n The number of components in the vectors
 */
static inline void vecNf_add_new(float result[ ], const float a[ ], const float b[ ], const int n)
{
	for(int i=0; i<n; i++)
		result[i] = a[i]+b[i];
}
/** Add two double vectors together and store the result in a new location.
 * @param result The sum of the two input vectors.
 * @param a One of the input vectors.
 * @param b The other input vector.
 * @param n The number of components in the vectors
 */
static inline void vecNd_add_new(double result[ ], const double a[ ], const double b[ ], const int n)
{
	for(int i=0; i<n; i++)
		result[i] = a[i]+b[i];
}

/** Add two 3-component float vectors together and store the result in a new location.
 * @param result The sum of the two input vectors.
 * @param a One of the input vectors.
 * @param b The other input vector.
 */
static inline void vec3f_add_new(float result[3], const float a[3], const float b[3])
{ vecNf_add_new(result, a, b, 3); }
/** Add two 3-component double vectors together and store the result in a new location.
 * @param result The sum of the two input vectors.
 * @param a One of the input vectors.
 * @param b The other input vector.
 */
static inline void vec3d_add_new(double result[3], const double a[3], const double b[3])
{ vecNd_add_new(result, a, b, 3); }
/** Add two 4-component float vectors together and store the result in a new location.
 * @param result The sum of the two input vectors.
 * @param a One of the input vectors.
 * @param b The other input vector.
 */
static inline void vec4f_add_new(float result[4], const float a[4], const float b[4])
{ vecNf_add_new(result, a, b, 4); }
/** Add two 4-component double vectors together and store the result in a new location.
 * @param result The sum of the two input vectors.
 * @param a One of the input vectors.
 * @param b The other input vector.
 */
static inline void vec4d_add_new(double result[4], const double a[4], const double b[4])
{ vecNd_add_new(result, a, b, 4); }

/** Add two float vectors together and store the result in the first input vector.
 * @param a One of the input vectors (to be replaced with the new sum).
 * @param b The other input vector.
 * @param n The number of components in the vectors
 */
static inline void vecNf_add(float  a[ ], const float  b[ ], const int n)
{ vecNf_add_new(a, a, b, n); }
/** Add two double vectors together and store the result in the first input vector.
 * @param a One of the input vectors (to be replaced with the new sum).
 * @param b The other input vector.
 * @param n The number of components in the vectors
 */
static inline void vecNd_add(double a[ ], const double b[ ], const int n)
{ vecNd_add_new(a, a, b, n); }
/** Add two 3-component float vectors together and store the result in the first input vector.
 * @param a One of the input vectors (to be replaced with the new sum).
 * @param b The other input vector.
 */
static inline void vec3f_add(float  a[3], const float  b[3])
{ vecNf_add_new(a, a, b, 3); }
/** Add two 3-component double vectors together and store the result in the first input vector.
 * @param a One of the input vectors (to be replaced with the new sum).
 * @param b The other input vector.
 */
static inline void vec3d_add(double a[3], const double b[3])
{ vecNd_add_new(a, a, b, 3); }
/** Add two 4-component float vectors together and store the result in the first input vector.
 * @param a One of the input vectors (to be replaced with the new sum).
 * @param b The other input vector.
 */
static inline void vec4f_add(float  a[4], const float  b[4])
{ vecNf_add_new(a, a, b, 4); }
/** Add two 4-component double vectors together and store the result in the first input vector.
 * @param a One of the input vectors (to be replaced with the new sum).
 * @param b The other input vector.
 */
static inline void vec4d_add(double a[4], const double b[4])
{ vecNd_add_new(a, a, b, 4); }


static inline void vecNf_sub_new(float result[ ], const float a[ ], const float b[ ], const int n)
{
	for(int i=0; i<n; i++)
		result[i] = a[i]-b[i];
}
static inline void vecNd_sub_new(double result[ ], const double a[ ], const double b[ ], const int n)
{
	for(int i=0; i<n; i++)
		result[i] = a[i]-b[i];
}
static inline void vec3f_sub_new(float result[3], const float a[3], const float b[3])
{ vecNf_sub_new(result, a, b, 3); }
static inline void vec3d_sub_new(double result[3], const double a[3], const double b[3])
{ vecNd_sub_new(result, a, b, 3); }
static inline void vec4f_sub_new(float result[4], const float a[4], const float b[4])
{ vecNf_sub_new(result, a, b, 4); }
static inline void vec4d_sub_new(double result[4], const double a[4], const double b[4])
{ vecNd_sub_new(result, a, b, 4); }


/** Copy the values in a matrix column into a vector.
    @param result The vector containing the column from the matrix.
    @param m The matrix to copy a column from.
    @param col The column to copy (0=first column)
    @param n The size of the matrix (3 means 3x3 matrix and a 3-component output vector).
*/
static inline void matNf_getColumn(float  result[ ], const float  m[  ], const int col, const int n)
{
	if(col >= n)
		msg(MSG_ERROR, "Column %d must be less than %d\n", col, n);
	for(int i=0; i<n; i++)
		result[i] = m[i+col*n];
}
/** Copy the values in a matrix column into a vector.
    @param result The vector containing the column from the matrix.
    @param m The matrix to copy a column from.
    @param col The column to copy (0=first column)
    @param n The size of the matrix (3 means 3x3 matrix and a 3-component output vector).
*/
static inline void matNd_getColumn(double result[ ], const double m[  ], const int col, const int n)
{
	if(col >= n)
		msg(MSG_ERROR, "Column %d must be less than %d\n", col, n);
	for(int i=0; i<n; i++)
		result[i] = m[i+col*n];
}
/** Copy the values in a 4x4 matrix column into a vector.
    @param result The vector containing the column from the matrix.
    @param m The matrix to copy a column from.
    @param col The column to copy (0=first column)
*/
static inline void mat4f_getColumn(float  result[4], const float  m[16], const int col)
{ matNf_getColumn(result, m, col, 4); }
/** Copy the values in a 4x4 matrix column into a vector.
    @param result The vector containing the column from the matrix.
    @param m The matrix to copy a column from.
    @param col The column to copy (0=first column)
*/
static inline void mat4d_getColumn(double result[4], const double m[16], const int col)
{ matNd_getColumn(result, m, col, 4); }
/** Copy the values in a 3x3 matrix column into a vector.
    @param result The vector containing the column from the matrix.
    @param m The matrix to copy a column from.
    @param col The column to copy (0=first column)
*/
static inline void mat3f_getColumn(float  result[3], const float  m[ 9], const int col)
{ matNf_getColumn(result, m, col, 3); }
/** Copy the values in a 3x3 matrix column into a vector.
    @param result The vector containing the column from the matrix.
    @param m The matrix to copy a column from.
    @param col The column to copy (0=first column)
*/
static inline void mat3d_getColumn(double result[3], const double m[ 9], const int col)
{ matNd_getColumn(result, m, col, 3); }


/** Copy the values in a matrix row into a vector.
    @param result The vector containing the row from the matrix.
    @param m The matrix to copy a row from.
    @param row The row to copy (0=first row)
    @param n The size of the matrix (3 means 3x3 matrix and a 3-component output vector).
*/
static inline void matNf_getRow(float  result[ ], const float  m[  ], const int row, const int n)
{
	if(row >= n)
		msg(MSG_ERROR, "Row %d must be less than %d\n", row, n);
	for(int i=0; i<n; i++)
		result[i] = m[row+i*n];
}
/** Copy the values in a matrix row into a vector.
    @param result The vector containing the row from the matrix.
    @param m The matrix to copy a row from.
    @param row The row to copy (0=first row)
    @param n The size of the matrix (3 means 3x3 matrix and a 3-component output vector).
*/
static inline void matNd_getRow(double result[ ], const double m[  ], const int row, const int n)
{
	if(row >= n)
		msg(MSG_ERROR, "Row %d must be less than %d\n", row, n);
	for(int i=0; i<n; i++)
		result[i] = m[row+i*n];
}
/** Copy the values in a 4x4 matrix row into a vector.
    @param result The vector containing the row from the matrix.
    @param m The matrix to copy a row from.
    @param row The row to copy (0=first row)
*/
static inline void mat4f_getRow(float  result[4], const float  m[16], const int row)
{ matNf_getRow(result, m, row, 4); }
/** Copy the values in a 4x4 matrix row into a vector.
    @param result The vector containing the row from the matrix.
    @param m The matrix to copy a row from.
    @param row The row to copy (0=first row)
*/
static inline void mat4d_getRow(double result[4], const double m[16], const int row)
{ matNd_getRow(result, m, row, 4); }
/** Copy the values in a 3x3 matrix row into a vector.
    @param result The vector containing the row from the matrix.
    @param m The matrix to copy a row from.
    @param row The row to copy (0=first row)
*/
static inline void mat3f_getRow(float  result[3], const float  m[ 9], const int row)
{ matNf_getRow(result, m, row, 3); }
/** Copy the values in a 3x3 matrix row into a vector.
    @param result The vector containing the row from the matrix.
    @param m The matrix to copy a row from.
    @param row The row to copy (0=first row)
*/
static inline void mat3d_getRow(double result[3], const double m[ 9], const int row)
{ matNd_getRow(result, m, row, 3); }


/** Set a column of an NxN float matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific column of the matrix.
 @param col The column to replace (the first column is column 0).
 @param n The size of the matrix (and the length of the vector v)
*/
static inline void matNf_setColumn(float  matrix[  ], const float  v[ ], const int col, const int n)
{
	if(col >= n)
		msg(MSG_ERROR, "Column %d must be less than %d\n", col, n);
	for(int row=0; row<n; row++)
		matrix[matN_getIndex(row, col, n)] = v[row];
}
/** Set a column of an NxN double matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific column of the matrix.
 @param col The column to replace (the first column is column 0).
 @param n The size of the matrix (and the length of the vector v)
*/
static inline void matNd_setColumn(double matrix[  ], const double v[ ], const int col, const int n)
{
	if(col >= n)
		msg(MSG_ERROR, "Column %d must be less than %d\n", col, n);
	for(int row=0; row<n; row++)
		matrix[matN_getIndex(row, col, n)] = v[row];
}

/** Set a column of an 3x3 float matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific column of the matrix.
 @param col The column to replace (the first column is column 0).
*/
static inline void mat3f_setColumn(float  matrix[ 9], const float  v[3], const int col)
{ matNf_setColumn(matrix, v, col, 3); }
/** Set a column of an 3x3 double matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific column of the matrix.
 @param col The column to replace (the first column is column 0).
*/
static inline void mat3d_setColumn(double matrix[ 9], const double v[3], const int col)
{ matNd_setColumn(matrix, v, col, 3); }
/** Set a column of an 4x4 float matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific column of the matrix.
 @param col The column to replace (the first column is column 0).
*/
static inline void mat4f_setColumn(float  matrix[16], const float  v[4], const int col)
{ matNf_setColumn(matrix, v, col, 4); }
/** Set a column of an 4x4 double matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific column of the matrix.
 @param col The column to replace (the first column is column 0).
*/
static inline void mat4d_setColumn(double matrix[16], const double v[4], const int col)
{ matNd_setColumn(matrix, v, col, 4); }
/** Set a row of an NxN float matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific row of the matrix.
 @param row The row to replace (the first row is row 0).
 @param n The size of the matrix (and the length of the vector v)
*/
static inline void matNf_setRow(float  matrix[  ], const float  v[ ], const int row, const int n)
{
	if(row >= n)
		msg(MSG_ERROR, "Row %d must be less than %d\n", row, n);
	for(int col=0; col<n; col++)
		matrix[matN_getIndex(row, col, n)] = v[col];
}
/** Set a row of an NxN double matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific row of the matrix.
 @param row The row to replace (the first row is row 0).
 @param n The size of the matrix (and the length of the vector v)
*/
static inline void matNd_setRow(double matrix[  ], const double v[ ], const int row, const int n)
{
	if(row >= n)
		msg(MSG_ERROR, "Row %d must be less than %d\n", row, n);
	for(int col=0; col<n; col++)
		matrix[matN_getIndex(row, col, n)] = v[col];
}
/** Set a row of an 3x3 float matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific row of the matrix.
 @param row The row to replace (the first row is row 0).
*/
static inline void mat3f_setRow(float  matrix[ 9], const float  v[3], const int row)
{ matNf_setRow(matrix, v, row, 3); }
/** Set a row of an 3x3 double matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific row of the matrix.
 @param row The row to replace (the first row is row 0).
*/
static inline void mat3d_setRow(double matrix[ 9], const double v[3], const int row)
{ matNd_setRow(matrix, v, row, 3); }
/** Set a row of an 4x4 float matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific row of the matrix.
 @param row The row to replace (the first row is row 0).
*/
static inline void mat4f_setRow(float  matrix[16], const float  v[4], const int row)
{ matNf_setRow(matrix, v, row, 4); }
/** Set a row of an 4x4 double matrix to a specific set of values.
 @param matrix The matrix to be changed.
 @param v The values to be placed into a specific row of the matrix.
 @param row The row to replace (the first row is row 0).
*/
static inline void mat4d_setRow(double matrix[16], const double v[4], const int row)
{ matNd_setRow(matrix, v, row, 4); }




/** Copies a NxN float matrix.
 @param dest The location to store the new copy of the matrix.
 @param src The matrix to be copied.
 @param n The size of the matrix. */
static inline void matNf_copy(float dest[], const float src[], const int n)
{ memcpy(dest, src, n*n*sizeof(float)); }
/** Copies a NxN double matrix.
 @param dest The location to store the new copy of the matrix.
 @param src The matrix to be copied.
 @param n The size of the matrix. */
static inline void matNd_copy(double dest[], const double src[], const int n)
{ memcpy(dest, src, n*n*sizeof(double)); }
/** Copies a 3x3 float matrix.
 @param dest The location to store the new copy of the matrix.
 @param src The matrix to be copied. */
static inline void mat3f_copy(float dest[9], const float src[9])
{ matNf_copy(dest, src, 3); }
/** Copies a 3x3 double matrix.
 @param dest The location to store the new copy of the matrix.
 @param src The matrix to be copied. */
static inline void mat3d_copy(double dest[9], const double src[9])
{ matNd_copy(dest, src, 3); }
/** Copies a 4x4 float matrix.
 @param dest The location to store the new copy of the matrix.
 @param src The matrix to be copied. */
static inline void mat4f_copy(float dest[16], const float src[16])
{ matNf_copy(dest, src, 4); }
/** Copies a 4x4 double matrix.
 @param dest The location to store the new copy of the matrix.
 @param src The matrix to be copied. */
static inline void mat4d_copy(double dest[16], const double src[16])
{ matNd_copy(dest, src, 4); }

/** Multiplies two matrices together.

    @param result The resulting n x n matrix containing matA * matB.
    @param matA The left operand. An n x n matrix.
    @param matB The right operand. An n x n matrix.
    @param n The size of the matrices. Can't be larger than 4.
 */
static inline void matNf_mult_matNf_new(float  result[  ], const float  matA[  ], const float  matB[  ], const int n)
{
	/* Use a temporary matrix so callers can do:
	   matrixA = matrixA * matrixB */
	float tempMatrix[4*4]; // avoid use of VLAs
	float vecA[4], vecB[4];
	for(int i=0; i<n; i++)
	{
		/* Get the ith row from matrix a */
		matNf_getRow(vecA, matA, i, n);
		for(int j=0; j<n; j++)
		{
			/* Get the jth column from matrix b */
			matNf_getColumn(vecB, matB, j, n);
			/* Dot product */
			tempMatrix[matN_getIndex(i,j,n)] = vecNf_dot(vecA, vecB, n);
		}
	}
	matNf_copy(result, tempMatrix, n);
}
/** Multiplies two matrices together.

    @param result The resulting n x n matrix containing matA * matB.
    @param matA The left operand. An n x n matrix.
    @param matB The right operand. An n x n matrix.
    @param n The size of the matrices. Can't be larger than 4.
 */
static inline void matNd_mult_matNd_new(double result[  ], const double matA[ ], const double matB[], const int n)
{
	/* Use a temporary matrix so callers can do:
	   matrixA = matrixA * matrixB */
	double tempMatrix[4*4]; // avoid use of VLAs
	double vecA[4], vecB[4];
	for(int i=0; i<n; i++)
	{
		/* Get the ith row from matrix a */
		matNd_getRow(vecA, matA, i, n);
		for(int j=0; j<n; j++)
		{
			/* Get the jth column from matrix b */
			matNd_getColumn(vecB, matB, j, n);
			/* Dot product */
			tempMatrix[matN_getIndex(i,j,n)] = vecNd_dot(vecA, vecB, n);
		}
	}
	matNd_copy(result, tempMatrix, n);
}
static inline void mat3f_mult_mat3f_new(float  result[9], const float  matA[ 9], const float  matB[9])
{ matNf_mult_matNf_new(result, matA, matB, 3); }
static inline void mat3d_mult_mat3d_new(double result[9], const double matA[ 9], const double matB[9])
{ matNd_mult_matNd_new(result, matA, matB, 3); }
static inline void mat4f_mult_mat4f_new(float  result[16], const float  matA[16], const float  matB[16])
{ matNf_mult_matNf_new(result, matA, matB, 4); }
static inline void mat4d_mult_mat4d_new(double result[16], const double matA[16], const double matB[16])
{ matNd_mult_matNd_new(result, matA, matB, 4); }



/** Multiply a column vector by a matrix (i.e., matrix *
    vector). Works even if the result parameter and the vector
    parameter point to the same location.

    @param result The resulting vector.
    @param m The n x n matrix to multiply the vector against.
    @param v The n component vector to multiply against the matrix.
    @param n The size of the matrix and vector (usually 3 or 4). Can't
    be larger than 4.
*/
static inline void matNf_mult_vecNf_new(float result[], const float m[], const float v[], const int n)
{
	float tmp[4*4]; // avoid using a variable length array
	for(int i=0; i<n; i++)
	{
		tmp[i] = 0;
		for(int j=0; j<n; j++)
			tmp[i] += m[matN_getIndex(i,j,n)] * v[j];
	}
	vecNf_copy(result, tmp, n);
}

/** Multiply a column vector by a matrix (i.e., matrix *
    vector). Works even if the result parameter and the vector
    parameter point to the same location.

    @param result The resulting vector.
    @param m The n x n matrix to multiply the vector against.
    @param v The n component vector to multiply against the matrix.
    @param n The size of the matrix and vector (usually 3 or 4). Can't
    be larger than 4.
*/
static inline void matNd_mult_vecNd_new(double result[], const double m[], const double v[], const int n)
{
	double tmp[4*4]; // avoid using a variable length array
	for(int i=0; i<n; i++)
	{
		tmp[i] = 0;
		for(int j=0; j<n; j++)
			tmp[i] += m[matN_getIndex(i,j,n)] * v[j];
	}
	vecNd_copy(result, tmp, n);
}
static inline void mat3f_mult_vec3f_new(float result[3], const float m[9], const float v[3])
{ matNf_mult_vecNf_new(result, m, v, 3); }
static inline void mat3d_mult_vec3d_new(double result[3], const double m[9], const double v[3])
{ matNd_mult_vecNd_new(result, m, v, 3); }
static inline void mat4f_mult_vec4f_new(float result[4], const float m[16], const float v[4])
{ matNf_mult_vecNf_new(result, m, v, 4); }
static inline void mat4d_mult_vec4d_new(double result[4], const double m[16], const double v[4])
{ matNd_mult_vecNd_new(result, m, v, 4); }

/* in-place vector = matrix*vector multiplication */
static inline void matNf_mult_vecNf(float vector[], const float matrix[], const int n)
{ matNf_mult_vecNf_new(vector, matrix, vector, n); }
static inline void matNd_mult_vecNd(double vector[], const double matrix[], const int n)
{ matNd_mult_vecNd_new(vector, matrix, vector, n); }
static inline void mat3f_mult_vec3f(float vector[3], const float matrix[9])
{ mat3f_mult_vec3f_new(vector, matrix, vector); }
static inline void mat3d_mult_vec3d(double vector[3], const double matrix[9])
{ mat3d_mult_vec3d_new(vector, matrix, vector); }
static inline void mat4f_mult_vec4f(float vector[4], const float matrix[16])
{ mat4f_mult_vec4f_new(vector, matrix, vector); }
static inline void mat4d_mult_vec4d(double vector[4], const double matrix[16])
{ mat4d_mult_vec4d_new(vector, matrix, vector); }



/** Transpose a NxN matrix in place.
 @param m The matrix to be transposed.
 @param n The size of the matrix. */
static inline void matNf_transpose(float m[], const int n)
{
	float tmp;
	for(int row=0; row<n; row++)
	{
		for(int col=0; col<row; col++)
		{
			int index1 = row+col*n;
			int index2 = col+row*n;
			tmp = m[index1];
			m[index1] = m[index2];
			m[index2] = tmp;
		}
	}
}
/** Transpose a NxN matrix in place.
 @param m The matrix to be transposed.
 @param n The size of the matrix. */
static inline void matNd_transpose(double m[], const int n)
{
	double tmp;
	for(int row=0; row<n; row++)
	{
		for(int col=0; col<row; col++)
		{
			int index1 = row+col*n;
			int index2 = col+row*n;
			tmp = m[index1];
			m[index1] = m[index2];
			m[index2] = tmp;
		}
	}
}

/** Transpose a matrix in place.
 @param m The matrix to be transposed. */
static inline void mat3f_transpose(float m[9])
{ matNf_transpose(m, 3); }
/** Transpose a matrix in place.
 @param m The matrix to be transposed. */
static inline void mat3d_transpose(double m[9])
{ matNd_transpose(m, 3); }
/** Transpose a matrix in place.
 @param m The matrix to be transposed. */
static inline void mat4f_transpose(float m[16])
{ matNf_transpose(m, 4); }
/** Transpose a matrix in place.
 @param m The matrix to be transposed. */
static inline void mat4d_transpose(double m[16])
{ matNd_transpose(m, 4); }

/** Transpose a NxN matrix and store the result at a new location.
 @param dest Where the transposed matrix should be stored.
 @param src The matrix to be transposed.
 @param n The size of the matrix. */
static inline void matNf_transpose_new(float  dest[  ], const float  src[  ], const int n)
{ matNf_copy(dest, src, n); matNf_transpose(dest, n); }
/** Transpose a NxN matrix and store the result at a new location.
 @param dest Where the transposed matrix should be stored.
 @param src The matrix to be transposed.
 @param n The size of the matrix. */
static inline void matNd_transpose_new(double dest[  ], const double src[  ], const int n)
{ matNd_copy(dest, src, n); matNd_transpose(dest, n); }
/** Transpose a matrix and store the result at a new location.
 @param dest Where the transposed matrix should be stored.
 @param src The matrix to be transposed. */
static inline void mat3f_transpose_new(float  dest[ 9], const float  src[ 9])
{ mat3f_copy(dest, src); mat3f_transpose(dest); }
/** Transpose a matrix and store the result at a new location.
 @param dest Where the transposed matrix should be stored.
 @param src The matrix to be transposed. */
static inline void mat3d_transpose_new(double dest[ 9], const double src[ 9])
{ mat3d_copy(dest, src); mat3d_transpose(dest); }
/** Transpose a matrix and store the result at a new location.
 @param dest Where the transposed matrix should be stored.
 @param src The matrix to be transposed. */
static inline void mat4f_transpose_new(float  dest[16], const float  src[16])
{ mat4f_copy(dest, src); mat4f_transpose(dest); }
/** Transpose a matrix and store the result at a new location.
 @param dest Where the transposed matrix should be stored.
 @param src The matrix to be transposed. */
static inline void mat4d_transpose_new(double dest[16], const double src[16])
{ mat4d_copy(dest, src); mat4d_transpose(dest); }



/** Create a NxN float identity matrix.
 @param m Location to store the resulting identity matrix.
 @param n The size of the matrix.
*/
static inline void matNf_identity(float m[], int n)
{
	for(int row=0; row<n; row++)
		for(int col=0; col<n; col++)
			if(row==col)
				m[row+col*n] = 1.0f;
			else
				m[row+col*n] = 0.0f;
}
/** Create a NxN double identity matrix.
 @param m Location to store the resulting identity matrix.
 @param n The size of the matrix.
*/
static inline void matNd_identity(double m[], int n)
{
	for(int row=0; row<n; row++)
		for(int col=0; col<n; col++)
			if(row==col)
				m[row+col*n] = 1.0;
			else
				m[row+col*n] = 0.0;
}
/** Create a 3x3 float identity matrix.
 @param m Location to store the resulting identity matrix.
*/
static inline void mat3f_identity(float m[9])
{ matNf_identity(m, 3); }
/** Create a 3x3 double identity matrix.
 @param m Location to store the resulting identity matrix.
*/
static inline void mat3d_identity(double m[9])
{ matNd_identity(m, 3); }

/** Create a 4x4 float identity matrix.
 @param m Location to store the resulting identity matrix.
*/
static inline void mat4f_identity(float m[16])
{ matNf_identity(m, 4); }
/** Create a 4x4 double identity matrix.
 @param m Location to store the resulting identity matrix.
*/
static inline void mat4d_identity(double m[16])
{ matNd_identity(m, 4); }

/** Print an N-component float matrix into a string. String is null
    terminated. It does NOT end with a space or a newline.

    @param dest A buffer to store the string in. It must be allocated
    to contain at least destSize bytes.

    @param destSize The amount of space available in dest.

    @param v A vector to print to the string.

    @param n The number of components in the vector.
*/
static inline void matNf_print_to_string(char *dest, const int destSize,
                                         const float m[ ], const int n)
{
	int used = 0;
	used += snprintf(dest+used, destSize-used, "mat%df [ ", n);
	for(int i=0; i<n; i++)
	{
		if(i > 0)
			used += snprintf(dest+used, destSize-used, "      [ ");
		for(int j=0; j<n; j++)
		{
			// Use a temporary variable since printf() parameters are not const.
			float tmp = m[matN_getIndex(i,j,n)];
			used += snprintf(dest+used, destSize-used, "%10.3f ", tmp);
		}
		used += snprintf(dest+used, destSize-used, "]");
		if(i < n-1)
			used += snprintf(dest+used, destSize-used, "\n");
	}
}

/** Print an N-component double matrix into a string. String is null
    terminated. It does NOT end with a space or a newline.

    @param dest A buffer to store the string in. It must be allocated
    to contain at least destSize bytes.

    @param destSize The amount of space available in dest.

    @param v A vector to print to the string.

    @param n The number of components in the vector.
*/
static inline void matNd_print_to_string(char *dest, const int destSize,
                                         const double m[ ], const int n)
{
	int used = 0;
	used += snprintf(dest+used, destSize-used, "mat%dd [ ", n);
	for(int i=0; i<n; i++)
	{
		if(i > 0)
			used += snprintf(dest+used, destSize-used, "      [ ");
		for(int j=0; j<n; j++)
		{
			// Use a temporary variable since printf() parameters are not const.
			double tmp = m[matN_getIndex(i,j,n)];
			used += snprintf(dest+used, destSize-used, "%10.3f ", tmp);
		}
		used += snprintf(dest+used, destSize-used, "]");
		if(i < n-1)
			used += snprintf(dest+used, destSize-used, "\n");
	}
}


/** Print a NxN float matrix to standard out.
 @param m The matrix to print.
 @param n The size of the matrix.
*/
static inline void matNf_print(const float  m[], const int n)
{
	char str[256];
	matNf_print_to_string(str, 256, m, n);
	printf("%s\n", str);
}
/** Print a NxN double matrix to standard out.
 @param m The matrix to print.
 @param n The size of the matrix.
*/
static inline void matNd_print(const double m[], const int n)
{
	char str[256];
	matNd_print_to_string(str, 256, m, n);
	printf("%s\n", str);
}
/** Print a 3x3 float matrix to standard out.
 @param m The matrix to print.
*/
static inline void mat3f_print(const float m[9])
{ matNf_print(m, 3); }
/** Print a 3x3 double matrix to standard out.
 @param m The matrix to print.
*/
static inline void mat3d_print(const double m[9])
{ matNd_print(m, 3); }
/** Print a 4x4 float matrix to standard out.
 @param m The matrix to print.
*/
static inline void mat4f_print(const float m[16])
{ matNf_print(m, 4); }
/** Print a 4x4 double matrix to standard out.
 @param m The matrix to print.
*/
static inline void mat4d_print(const double m[16])
{ matNd_print(m, 4); }

/** Create a 3x3 double matrix from a 3x3 float matrix.
    @param dest Location to store new matrix.
    @param src Location of the original matrix.
*/
static inline void mat3d_from_mat3f(double dest[ 9], const float  src[ 9])
{ for(int i=0; i<9; i++) dest[i] = (double) src[i]; }
/** Create a 4x4 double matrix from a 4x4 float matrix.
    @param dest Location to store new matrix.
    @param src Location of the original matrix.
*/
static inline void mat4d_from_mat4f(double dest[16], const float  src[16])
{ for(int i=0; i<16; i++) dest[i] = (double) src[i]; }
/** Create a 3x3 float matrix from a 3x3 double matrix.
    @param dest Location to store new matrix.
    @param src Location of the original matrix.
*/
static inline void mat3f_from_mat3d(float  dest[ 9], const double src[ 9])
{ for(int i=0; i<9; i++) dest[i] = (float) src[i]; }
/** Create a 4x4 float matrix from a 4x4 double matrix.
    @param dest Location to store new matrix.
    @param src Location of the original matrix.
*/
static inline void mat4f_from_mat4d(float  dest[16], const double src[16])
{ for(int i=0; i<16; i++) dest[i] = (float) src[i]; }

/** Creates a 4x4 matrix from a 3x3 matrix. The new matrix is set to
    the identity and then the 3x3 matrix is copied into the upper left
    corner of the matrix.
    @param dest The new 4x4 matrix.
    @param src The original 3x3 matrix.
*/
static inline void mat4f_from_mat3f(float  dest[16], const float  src[ 9])
{
	mat4f_identity(dest);
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			dest[mat4_getIndex(i,j)] = src[mat3_getIndex(i,j)];
}
/** Creates a 4x4 matrix from a 3x3 matrix. The new matrix is set to
    the identity and then the 3x3 matrix is copied into the upper left
    corner of the matrix.
    @param dest The new 4x4 matrix.
    @param src The original 3x3 matrix.
*/
static inline void mat4d_from_mat3d(double dest[16], const double src[ 9])
{
	mat4d_identity(dest);
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			dest[mat4_getIndex(i,j)] = src[mat3_getIndex(i,j)];
}

/** Creates a 3x3 matrix from a 4x4 matrix by copying only the upper-left 3x3 components from the 4x4 matrix.
    @param dest The new 3x3 matrix.
    @param src The original 4x4 matrix.
*/
static inline void mat3f_from_mat4f(float  dest[ 9], const float  src[16])
{
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			dest[mat3_getIndex(i,j)] = src[mat4_getIndex(i,j)];
}
/** Creates a 3x3 matrix from a 4x4 matrix by copying only the upper-left 3x3 components from the 4x4 matrix.
    @param dest The new 3x3 matrix.
    @param src The original 4x4 matrix.
*/
static inline void mat3d_from_mat4d(double dest[ 9], const double src[16])
{
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			dest[mat3_getIndex(i,j)] = src[mat4_getIndex(i,j)];
}


/** Creates a new 4x4 float scale matrix with the rest of the matrix set to the identity.
    @param result The location to store the new scale matrix.
    @param x The amount that the matrix should scale the x-components by.
    @param y The amount that the matrix should scale the y-components by.
    @param z The amount that the matrix should scale the z-components by.
*/
static inline void mat4f_scale_new(float  result[16], float x, float y, float z)
{
	mat4f_identity(result);
	const char *message = "You created a scale matrix with %c=0; did you mean to set it to 1?";
	if(x == 0.0f) msg(MSG_WARNING, message, 'x');
	if(y == 0.0f) msg(MSG_WARNING, message, 'y');
	if(z == 0.0f) msg(MSG_WARNING, message, 'z');
	result[mat4_getIndex(0,0)] = x;
	result[mat4_getIndex(1,1)] = y;
	result[mat4_getIndex(2,2)] = z;
}
/** Creates a new 4x4 double scale matrix with the rest of the matrix set to the identity.
    @param result The location to store the new scale matrix.
    @param x The amount that the matrix should scale the x-components by.
    @param y The amount that the matrix should scale the y-components by.
    @param z The amount that the matrix should scale the z-components by.
*/
static inline void mat4d_scale_new(double result[16], double x, double y, double z)
{
	mat4d_identity(result);
	const char *message = "You created a scale matrix with %c=0; did you mean to set it to 1?";
	if(x == 0.0f) msg(MSG_WARNING, message, 'x');
	if(y == 0.0f) msg(MSG_WARNING, message, 'y');
	if(z == 0.0f) msg(MSG_WARNING, message, 'z');
	result[mat4_getIndex(0,0)] = x;
	result[mat4_getIndex(1,1)] = y;
	result[mat4_getIndex(2,2)] = z;
}
/** Creates a new 4x4 float scale matrix with the rest of the matrix set to the identity.
    @param result The location to store the new scale matrix.
    @param xyz A vector containing the amount to scale each component by.
*/
static inline void mat4f_scaleVec_new(float  result[16], const float  xyz[3])
{ mat4f_scale_new(result, xyz[0], xyz[1], xyz[2]); }
/** Creates a new 4x4 double scale matrix with the rest of the matrix set to the identity.
    @param result The location to store the new scale matrix.
    @param xyz A vector containing the amount to scale each component by.
*/
static inline void mat4d_scaleVec_new(double result[16], const double xyz[3])
{ mat4d_scale_new(result, xyz[0], xyz[1], xyz[2]); }
/** Creates a new 3x3 float scale matrix with the rest of the matrix set to the identity.
    @param result The location to store the new scale matrix.
    @param x The amount that the matrix should scale the x-components by.
    @param y The amount that the matrix should scale the y-components by.
    @param z The amount that the matrix should scale the z-components by.
*/
static inline void mat3f_scale_new(float  result[9], float x, float y, float z)
{
	mat3f_identity(result);
	result[mat3_getIndex(0,0)] = x;
	result[mat3_getIndex(1,1)] = y;
	result[mat3_getIndex(2,2)] = z;
}
/** Creates a new 3x3 double scale matrix with the rest of the matrix set to the identity.
    @param result The location to store the new scale matrix.
    @param x The amount that the matrix should scale the x-components by.
    @param y The amount that the matrix should scale the y-components by.
    @param z The amount that the matrix should scale the z-components by.
*/
static inline void mat3d_scale_new(double result[9], double x, double y, double z)
{
	mat3d_identity(result);
	result[mat3_getIndex(0,0)] = x;
	result[mat3_getIndex(1,1)] = y;
	result[mat3_getIndex(2,2)] = z;
}
/** Creates a new 3x3 float scale matrix with the rest of the matrix set to the identity.
    @param result The location to store the new scale matrix.
    @param xyz A vector containing the amount to scale each component by.
*/
static inline void mat3f_scaleVec_new(float  result[9], const float  xyz[3])
{ mat3f_scale_new(result, xyz[0], xyz[1], xyz[2]); }
/** Creates a new 3x3 double scale matrix with the rest of the matrix set to the identity.
    @param result The location to store the new scale matrix.
    @param xyz A vector containing the amount to scale each component by.
*/
static inline void mat3d_scaleVec_new(double result[9], const double xyz[3])
{ mat3d_scale_new(result, xyz[0], xyz[1], xyz[2]); }


/* Multiply two or more matrices together. */
void mat4f_mult_mat4f_many(float  out[16], const float  *in, ...);
void mat4d_mult_mat4d_many(double out[16], const double *in, ...);
void mat3f_mult_mat3f_many(float  out[9],  const float  *in, ...);
void mat3d_mult_mat3d_many(double out[9],  const double *in, ...);
	
/* mat[43][df]_invert_new() will invert a matrix and store the
 * inverted matrix at a new location. However, these functions work
 * correctly even if you try to invert a matrix in place. For example,
 * if you want to invert a 4x4 float matrix in place, you can use either of the
 * following:
 *    mat4f_invert_new(matrix, matrix);
 *    mat4f_invert(matrix);
 *
 * Returns 1 if matrix was inverted. If an error occurs, a message is
 * printed to stdout, the function returns 0 and the matrix 'out' is
 * left unchanged.
 */
int mat4f_invert_new(float  dest[16], const float  src[16]);
int mat4d_invert_new(double dest[16], const double src[16]);
int mat3f_invert_new(float  dest[ 9], const float  src[9]);
int mat3d_invert_new(double dest[ 9], const double src[9]);
/* Invert a matrix in place. */
int mat4f_invert(float  matrix[16]);
int mat4d_invert(double matrix[16]);
int mat3f_invert(float  matrix[ 9]);
int mat3d_invert(double matrix[ 9]);

/* Creates 3x3 rotation matrix from Euler angles. */
void mat3f_rotateEuler_new(float result[9], float a1_degrees, float a2_degrees, float a3_degrees, const char order[3]);
void mat3d_rotateEuler_new(double result[9], double a1_degrees, double a2_degrees, double a3_degrees, const char order[3]);
void mat4f_rotateEuler_new(float result[16], float a1_degrees, float a2_degrees, float a3_degrees, const char order[3]);
void mat4d_rotateEuler_new(double result[16], double a1_degrees, double a2_degrees, double a3_degrees, const char order[3]);

/* Calculate Euler angles from rotation matrices. */
void eulerf_from_mat3f(float  angles[3], const float  m[9],  const char order[3]);
void eulerd_from_mat3d(double angles[3], const double m[9],  const char order[3]);
void eulerf_from_mat4f(float  angles[3], const float  m[16], const char order[3]);
void eulerd_from_mat4d(double angles[3], const double m[16], const char order[3]);


/* Creates a new 3x3 rotation matrix (or a 4x4 matrix with 0,0,0,1 in
   the 4th column and 4th row). The matrix will rotate a point around
   the given "axis" by the number of "degrees".  Stores the resulting
   matrix in the "result" matrix (translation part of 4x4 matrix set
   to identity). Any data in the 'result' matrix that you pass to
   these functions will be ignored and lost. */
void mat3f_rotateAxisVec_new(float  result[ 9], float  degrees, const float  axis[3]);
void mat3d_rotateAxisVec_new(double result[ 9], double degrees, const double axis[3]);
void mat4f_rotateAxisVec_new(float  result[16], float  degrees, const float  axis[3]);
void mat4d_rotateAxisVec_new(double result[16], double degrees, const double axis[3]);
void mat3f_rotateAxis_new(float  result[ 9], float  degrees, float  axisX, float  axisY, float  axisZ);
void mat3d_rotateAxis_new(double result[ 9], double degrees, double axisX, double axisY, double axisZ);
void mat4f_rotateAxis_new(float  result[16], float  degrees, float  axisX, float  axisY, float  axisZ);
void mat4d_rotateAxis_new(double result[16], double degrees, double axisX, double axisY, double axisZ);

/* Calculate rotation matrices from a quaternion */
void mat3f_rotateQuatVec_new(float matrix[9], const float quat[4]);
void mat3d_rotateQuatVec_new(double matrix[9], const double quat[4]);
void mat4f_rotateQuatVec_new(float matrix[16], const float quat[4]);
void mat4d_rotateQuatVec_new(double matrix[16], const double quat[4]);
void mat3f_rotateQuat_new(float  matrix[9],  float  x, float  y, float  z, float w);
void mat3d_rotateQuat_new(double matrix[9],  double x, double y, double z, double w);
void mat4f_rotateQuat_new(float  matrix[16], float  x, float  y, float  z, float w);
void mat4d_rotateQuat_new(double matrix[16], double x, double y, double z, double w);

/* Create unit quaternion from a rotation matrix */
void quatf_from_mat3f(float  quat[4], const float  matrix[9]);
void quatd_from_mat3d(double quat[4], const double matrix[9]);
void quatf_from_mat4f(float  quat[4], const float  matrix[16]);
void quatd_from_mat4d(double quat[4], const double matrix[16]);

/* Create a quaternion (x,y,z,w) based an axis and angle */
void quatf_rotateAxis_new(float quat[4], float degrees, float x, float y, float z);
void quatd_rotateAxis_new(double quat[4], double degrees, double x, double y, double z);
void quatf_rotateAxisVec_new(float quat[4], float degrees, const float axis[3]);
void quatd_rotateAxisVec_new(double quat[4], double degrees, const double axis[3]);

/* Spherical linear interpolation of quaternions. */
void quatf_slerp_new(float  result[4], const float  start[4], const float  end[4], float  t);
void quatd_slerp_new(double result[4], const double start[4], const double end[4], double t);

/* Create a new translation matrix (rotation part set to
   identity). Any data in the 'result' matrix that you pass to these
   functions will be ignored and lost. */
void mat4f_translate_new(float  result[16], float  x, float  y, float  z);
void mat4d_translate_new(double result[16], double x, double y, double z);
void mat4f_translateVec_new(float  result[16], const float  xyz[3]);
void mat4d_translateVec_new(double result[16], const double xyz[3]);


/* Sets dest to the identity and then copies src into the upper-left
 * corner of dest. */
void mat4f_from_mat3f(float  dest[16], const float  src[ 9]);
void mat4d_from_mat3d(double dest[16], const double src[ 9]);
/* Copies upper-left 3x3 portion of src into dest. */
void mat3f_from_mat4f(float  dest[ 9], const float  src[16]);
void mat3d_from_mat4d(double dest[ 9], const double src[16]);

/* Replacements for glFrustum, glOrtho, gluPerspective */
void mat4f_frustum_new(float  result[16], float  left, float  right, float  bottom, float  top, float  near, float  far);
void mat4d_frustum_new(double result[16], double left, double right, double bottom, double top, double near, double far);
void mat4f_ortho_new(float  result[16], float  left, float  right, float  bottom, float  top, float  near, float  far);
void mat4d_ortho_new(double result[16], double left, double right, double bottom, double top, double near, double far);
void mat4f_perspective_new(float  result[16], float  fovy, float  aspect, float  near, float  far);
void mat4d_perspective_new(double result[16], double fovy, double aspect, double near, double far);

/* Replacements for gluLookAt(). Also, provide versions where inputs
 * are arrays. */
void mat4f_lookat_new(float result[16], float camPosX, float camPosY, float camPosZ, float lookAtPtX, float lookAtPtY, float lookAtPtZ, float upVecX, float upVecY, float upVecz);
void mat4d_lookat_new(double result[16], double camPosX, double camPosY, double camPosZ, double lookAtPtX, double lookAtPtY, double lookAtPtZ, double upVecX, double upVecY, double upVecZ);
void mat4f_lookatVec_new(float  result[16], const float  camPos[3], const float  lookAtPt[3], const float  upVec[3]);
void mat4d_lookatVec_new(double result[16], const double camPos[3], const double lookAtPt[3], const double upVec[3]);


/* Matrix stack implementation */
void mat4f_stack_push(list *l);
void mat4f_stack_mult(list *l, float m[16]);
void mat4f_stack_pop(list *l);
void mat4f_stack_peek(const list *l, float m[16]);

	
#ifdef __cplusplus
} // end extern "C"
#endif
