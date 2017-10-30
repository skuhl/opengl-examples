/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include "vecmat.h"


/* Set a vector to a value */
extern inline void vec3f_set(float  v[3], float  a, float  b, float  c);
extern inline void vec3d_set(double v[3], double a, double b, double c);
extern inline void vec4f_set(float  v[4], float  a, float  b, float  c, float  d);
extern inline void vec4d_set(double v[4], double a, double b, double c, double d);

/* Copy a vector into another */
extern inline void vecNf_copy(float  result[ ], const float  a[ ], const int n);
extern inline void vecNd_copy(double result[ ], const double a[ ], const int n);
extern inline void vec3f_copy(float  result[3], const float  a[3]);
extern inline void vec3d_copy(double result[3], const double a[3]);
extern inline void vec4f_copy(float  result[4], const float  a[4]);
extern inline void vec4d_copy(double result[4], const double a[4]);

/* Cross product of two vectors. Works even if result points to the same
 * location in memory as the A or B vectors. This allow you to
 * calculate A=A cross B by calling vec3f(A,A,B) */
extern inline void vec3f_cross_new(float  result[3], const float  A[3], const float  B[3]);
extern inline void vec3d_cross_new(double result[3], const double A[3], const double B[3]);

/* Vector dot products.
   Equivalent to multiplying a RowVec * ColVec = scalar */
extern inline float  vecNf_dot(const float  A[ ], const float  B[ ], const int n);
extern inline double vecNd_dot(const double A[ ], const double B[ ], const int n);
extern inline float  vec3f_dot(const float  A[3], const float  B[3]);
extern inline double vec3d_dot(const double A[3], const double B[3]);
extern inline float  vec4f_dot(const float  A[4], const float  B[4]);
extern inline double vec4d_dot(const double A[4], const double B[4]);

/* Vector multiplication.
   Multiply a ColVec * RowVec = Matrix */
extern inline void vecNf_mult_vecNf(float  m[ ],  const float  A[ ], const float  B[ ], const int n);
extern inline void vecNd_mult_vecNd(double m[ ],  const double A[ ], const double B[ ], const int n);
extern inline void vec3f_mult_vec3f(float  m[9],  const float  A[3], const float  B[3]);
extern inline void vec3d_mult_vec3d(double m[9],  const double A[3], const double B[3]);
extern inline void vec4f_mult_vec4f(float  m[16], const float  A[4], const float  B[4]);
extern inline void vec4d_mult_vec4d(double m[16], const double A[4], const double B[4]);



/* Calculate the norm squared (i.e., length squared) of a vector. This
 * is different than normalizing a vector. */
extern inline float  vec3f_normSq(const float  A[3]);
extern inline double vec3d_normSq(const double A[3]);
extern inline float  vec4f_normSq(const float  A[4]);
extern inline double vec4d_normSq(const double A[4]);

/* Calculate the norm (i.e., length) of a vector. This is different
 * than normalizing a vector! */
extern inline float  vec3f_norm(const float  A[3]);
extern inline double vec3d_norm(const double A[3]);
extern inline float  vec4f_norm(const float  A[4]);
extern inline double vec4d_norm(const double A[4]);

/* Divide every element in vector with the scalar value: result = vector / scalar */
extern inline void vecNf_scalarDiv_new(float  result[ ], const float  v[ ], const float  scalar, const int n);
extern inline void vecNd_scalarDiv_new(double result[ ], const double v[ ], const double scalar, const int n);
extern inline void vec3f_scalarDiv_new(float  result[3], const float  v[3], const float  scalar);
extern inline void vec3d_scalarDiv_new(double result[3], const double v[3], const double scalar);
extern inline void vec4f_scalarDiv_new(float  result[4], const float  v[4], const float  scalar);
extern inline void vec4d_scalarDiv_new(double result[4], const double v[4], const double scalar);
/* In-place scalar division */
extern inline void vecNf_scalarDiv(float  v[ ], const float  scalar, const int n);
extern inline void vecNd_scalarDiv(double v[ ], const double scalar, const int n);
extern inline void vec3f_scalarDiv(float  v[3], const float  scalar);
extern inline void vec3d_scalarDiv(double v[3], const double scalar);
extern inline void vec4f_scalarDiv(float  v[4], const float  scalar);
extern inline void vec4d_scalarDiv(double v[4], const double scalar);


/* Multiply each element in the vector by a scalar (result = v *
 * scalar) . Works even if the result vector points to the same
 * location in memory as the input vector. */
extern inline void vecNf_scalarMult_new(float  result[ ], const float  v[ ], const float  scalar, const int n);
extern inline void vecNd_scalarMult_new(double result[ ], const double v[ ], const double scalar, const int n);
extern inline void vec3f_scalarMult_new(float  result[3], const float  v[3], const float  scalar);
extern inline void vec3d_scalarMult_new(double result[3], const double v[3], const double scalar);
extern inline void vec4f_scalarMult_new(float  result[4], const float  v[4], const float  scalar);
extern inline void vec4d_scalarMult_new(double result[4], const double v[4], const double scalar);
/* In-place scalar multiplication */
extern inline void vecNf_scalarMult(float  v[ ], const float  scalar, const int n);
extern inline void vecNd_scalarMult(double v[ ], const double scalar, const int n);
extern inline void vec3f_scalarMult(float  v[3], const float  scalar);
extern inline void vec3d_scalarMult(double v[3], const double scalar);
extern inline void vec4f_scalarMult(float  v[4], const float  scalar);
extern inline void vec4d_scalarMult(double v[4], const double scalar);


/* Normalize the vector so that it is a unit vector. */
extern inline void vec3f_normalize_new(float  dest[3], const float  src[3]);
extern inline void vec3d_normalize_new(double dest[3], const double src[3]);
extern inline void vec4f_normalize_new(float  dest[4], const float  src[4]);
extern inline void vec4d_normalize_new(double dest[4], const double src[4]);
/* Normalize a vector in place. */
extern inline void vec3f_normalize(float  v[3]);
extern inline void vec3d_normalize(double v[3]);
extern inline void vec4f_normalize(float  v[4]);
extern inline void vec4d_normalize(double v[4]);

/* Homogenize a 4-element vector, store result at a new location */
extern inline void vec4f_homogenize_new(float  dest[4], const float  src[4]);
extern inline void vec4d_homogenize_new(double dest[4], const double src[4]);
/* Homogenize a 4-element vector in place */
extern inline void vec4f_homogenize(float  v[4]);
extern inline void vec4d_homogenize(double v[4]);

/* Add two vectors together. Store result in a new location: result = vectorA + vectorB */
extern inline void vecNf_add_new(float  result[ ], const float  a[ ], const float  b[ ], const int n);
extern inline void vecNd_add_new(double result[ ], const double a[ ], const double b[ ], const int n);
extern inline void vec3f_add_new(float  result[3], const float  a[3], const float  b[3]);
extern inline void vec3d_add_new(double result[3], const double a[3], const double b[3]);
extern inline void vec4f_add_new(float  result[4], const float  a[4], const float  b[4]);
extern inline void vec4d_add_new(double result[4], const double a[4], const double b[4]);

/* Add two vectors together. Store the resulting sum in the first parameter: a = a+b */
extern inline void vecNf_add(float  a[ ], const float  b[ ], const int n);
extern inline void vecNd_add(double a[ ], const double b[ ], const int n);
extern inline void vec3f_add(float  a[3], const float  b[3]);
extern inline void vec3d_add(double a[3], const double b[3]);
extern inline void vec4f_add(float  a[4], const float  b[4]);
extern inline void vec4d_add(double a[4], const double b[4]);

/* Subtract two vectors from each other. Store the result of the calculation in a new location:
   result = vectorA - vectorB

   IMPORTANT: There are no vecNf_sub() methods that do this calculation in place because it isn't clear if it should implement: a=a-b or a=b-a. However, you can call vec3f_sub_new(a,a,b) to calculate a=a-b.
*/
extern inline void vecNf_sub_new(float  result[ ], const float  a[ ], const float  b[ ], const int n);
extern inline void vecNd_sub_new(double result[ ], const double a[ ], const double b[ ], const int n);
extern inline void vec3f_sub_new(float  result[3], const float  a[3], const float  b[3]);
extern inline void vec3d_sub_new(double result[3], const double a[3], const double b[3]);
extern inline void vec4f_sub_new(float  result[4], const float  a[4], const float  b[4]);
extern inline void vec4d_sub_new(double result[4], const double a[4], const double b[4]);

/* Print a vector to a string */
static inline void vecNf_print_to_string(char *dest, const int destSize,
                                         const float v[ ], const int n);
static inline void vecNd_print_to_string(char *dest, const int destSize,
                                         const double v[ ], const int n);

/* Print the vector to standard out. */
extern inline void vecNf_print(const float  v[ ], const int n);
extern inline void vecNd_print(const double v[ ], const int n);
extern inline void vec3f_print(const float  v[3]);
extern inline void vec3d_print(const double v[3]);
extern inline void vec4f_print(const float  v[4]);
extern inline void vec4d_print(const double v[4]);

/* Given a row and column, get the index for that entry in the
 * matrix. These functions don't depend on if the matrix is composed
 * of doubles or floats, but the extra functions such as
 * mat3f_getIndex() are provided for convenience and consistency with
 * the rest of the functions. */
extern inline int matN_getIndex(const int row, const int col, const int n);
extern inline int mat3_getIndex(const int row, const int col);
extern inline int mat4_getIndex(const int row, const int col);
extern inline int mat3f_getIndex(const int row, const int col);
extern inline int mat4d_getIndex(const int row, const int col);
extern inline int mat3f_getIndex(const int row, const int col);
extern inline int mat4d_getIndex(const int row, const int col);

/* Get a row or column from a matrix. First row/column is 0! */
extern inline void matNf_getColumn(float  result[ ], const float  m[  ], const int col, const int n);
extern inline void matNd_getColumn(double result[ ], const double m[  ], const int col, const int n);
extern inline void mat3f_getColumn(float  result[3], const float  m[ 9], const int col);
extern inline void mat3d_getColumn(double result[3], const double m[ 9], const int col);
extern inline void mat4f_getColumn(float  result[4], const float  m[16], const int col);
extern inline void mat4d_getColumn(double result[4], const double m[16], const int col);
extern inline void matNf_getRow(float  result[ ], const float  m[  ], const int row, const int n);
extern inline void matNd_getRow(double result[ ], const double m[  ], const int row, const int n);
extern inline void mat3f_getRow(float  result[3], const float  m[ 9], const int row);
extern inline void mat3d_getRow(double result[3], const double m[ 9], const int row);
extern inline void mat4f_getRow(float  result[4], const float  m[16], const int row);
extern inline void mat4d_getRow(double result[4], const double m[16], const int row);

/* Set the specific column or row in matrix to the values stored in
 * vector v. The first row/column is 0. The size of the matrix must
 * match the size of the vector for these to work correctly (i.e., if
 * you want to set a row or column of a 4x4 matrix, you must use
 * mat4[fd]_set[Row|Column]() and pass in a vector with 4 elements. */
extern inline void matNf_setColumn(float  matrix[  ], const float  v[ ], const int col, const int n);
extern inline void matNd_setColumn(double matrix[  ], const double v[ ], const int col, const int n);
extern inline void mat3f_setColumn(float  matrix[ 9], const float  v[3], const int col);
extern inline void mat3d_setColumn(double matrix[ 9], const double v[3], const int col);
extern inline void mat4f_setColumn(float  matrix[16], const float  v[4], const int col);
extern inline void mat4d_setColumn(double matrix[16], const double v[4], const int col);
extern inline void matNf_setRow(float  matrix[  ], const float  m[ ], const int row, const int n);
extern inline void matNd_setRow(double matrix[  ], const double m[ ], const int row, const int n);
extern inline void mat3f_setRow(float  matrix[ 9], const float  m[3], const int row);
extern inline void mat3d_setRow(double matrix[ 9], const double m[3], const int row);
extern inline void mat4f_setRow(float  matrix[16], const float  m[4], const int row);
extern inline void mat4d_setRow(double matrix[16], const double m[4], const int row);


/* Copy a matrix */
extern inline void matNf_copy(float  dest[], const float  src[], const int n);
extern inline void matNd_copy(double dest[], const double src[], const int n);
extern inline void mat3f_copy(float  dest[ 9], const float  src[ 9]);
extern inline void mat3d_copy(double dest[ 9], const double src[ 9]);
extern inline void mat4f_copy(float  dest[16], const float  src[16]);
extern inline void mat4d_copy(double dest[16], const double src[16]);

/* result = matrix * vector; Works even if result and vector point to the same place. */
extern inline void matNf_mult_vecNf_new(float  result[ ], const float  m[  ], const float  v[ ], const int n);
extern inline void matNd_mult_vecNd_new(double result[ ], const double m[  ], const double v[ ], const int n);
extern inline void mat3f_mult_vec3f_new(float  result[3], const float  m[ 9], const float  v[3]);
extern inline void mat3d_mult_vec3d_new(double result[3], const double m[ 9], const double v[3]);
extern inline void mat4f_mult_vec4f_new(float  result[4], const float  m[16], const float  v[4]);
extern inline void mat4d_mult_vec4d_new(double result[4], const double m[16], const double v[4]);
/* vector = matrix * vector */
extern inline void matNf_mult_vecNf(float vector[], const float matrix[], const int n);
extern inline void matNd_mult_vecNd(double vector[], const double matrix[], const int n);
extern inline void mat3f_mult_vec3f(float vector[3], const float matrix[9]);
extern inline void mat3d_mult_vec3d(double vector[3], const double matrix[9]);
extern inline void mat4f_mult_vec4f(float vector[4], const float matrix[16]);
extern inline void mat4d_mult_vec4d(double vector[4], const double matrix[16]);

/* result = matrixA * matrixB; Works even if result and matrixA (or matrixB) point to the same place */
extern inline void matNf_mult_matNf_new(float  result[ ], const float  matA[  ], const float  matB[  ], const int n);
extern inline void matNd_mult_matNd_new(double result[ ], const double matA[  ], const double matB[  ], const int n);
extern inline void mat3f_mult_mat3f_new(float  result[3], const float  matA[ 9], const float  matB[ 9]);
extern inline void mat3d_mult_mat3d_new(double result[3], const double matA[ 9], const double matB[ 9]);
extern inline void mat4f_mult_mat4f_new(float  result[4], const float  matA[16], const float  matB[16]);
extern inline void mat4d_mult_mat4d_new(double result[4], const double matA[16], const double matB[16]);

/* Transpose a matrix in place. */
extern inline void matNf_transpose(float  m[  ], const int n);
extern inline void matNd_transpose(double m[  ], const int n);
extern inline void mat3f_transpose(float  m[ 9]);
extern inline void mat3d_transpose(double m[ 9]);
extern inline void mat4f_transpose(float  m[16]);
extern inline void mat4d_transpose(double m[16]);

/* Transpose a matrix and store the result at a different location. */
extern inline void matNf_transpose_new(float  dest[  ], const float  src[  ], const int n);
extern inline void matNd_transpose_new(double dest[  ], const double src[  ], const int n);
extern inline void mat3f_transpose_new(float  dest[ 9], const float  src[ 9]);
extern inline void mat3d_transpose_new(double dest[ 9], const double src[ 9]);
extern inline void mat4f_transpose_new(float  dest[16], const float  src[16]);
extern inline void mat4d_transpose_new(double dest[16], const double src[16]);


/* Set matrix to identity */
extern inline void matNf_identity(float  m[  ], const int n);
extern inline void matNd_identity(double m[  ], const int n);
extern inline void mat3f_identity(float  m[ 9]);
extern inline void mat3d_identity(double m[ 9]);
extern inline void mat4f_identity(float  m[16]);
extern inline void mat4d_identity(double m[16]);

/* Print matrix */
extern inline void matNf_print(const float  m[  ], const int n);
extern inline void matNd_print(const double m[  ], const int n);
extern inline void mat3f_print(const float  m[ 9]);
extern inline void mat3d_print(const double m[ 9]);
extern inline void mat4f_print(const float  m[16]);
extern inline void mat4d_print(const double m[16]);


/* Set the matrix to the identity and then set the first three numbers along the diagonal starting from the upper-left corner of the matrix */
extern inline void mat4f_scale_new(float  result[16], float  x, float  y, float  z);
extern inline void mat4d_scale_new(double result[16], double x, double y, double z);
extern inline void mat4f_scaleVec_new(float  result[16], const float  xyz[3]);
extern inline void mat4d_scaleVec_new(double result[16], const double xyz[3]);
extern inline void mat3f_scale_new(float  result[9], float x, float y, float z);
extern inline void mat3d_scale_new(double result[9], double x, double y, double z);
extern inline void mat3f_scaleVec_new(float  result[9], const float  xyz[3]);
extern inline void mat3d_scaleVec_new(double result[9], const double xyz[3]);


/* Convert between 3x3 and 4x4 matrices */
extern inline void mat3d_from_mat3f(double dest[ 9], const float  src[ 9]);
extern inline void mat4d_from_mat4f(double dest[16], const float  src[16]);
extern inline void mat3f_from_mat3d(float  dest[ 9], const double src[ 9]);
extern inline void mat4f_from_mat4d(float  dest[16], const double src[16]);
extern inline void mat4f_from_mat3f(float  dest[16], const float  src[ 9]);
extern inline void mat4d_from_mat3d(double dest[16], const double src[ 9]);
extern inline void mat3f_from_mat4f(float  dest[ 9], const float  src[16]);
extern inline void mat3d_from_mat4d(double dest[ 9], const double src[16]);


/** Multiply an arbitrary list of matrices together. The last matrix must be NULL.

    @param out The product of all of the other matrices.
    @param in One or more matrices to be multiplied together.
 */
void mat4f_mult_mat4f_many(float out[16], const float *in, ...)
{
	/* Initialize argList. */
	va_list argList;
	va_start(argList, in);

	/* Handle first matrix */
	mat4f_copy(out, in);

	/* Multiply all of the other matrices with the existing one until
	 * we reach NULL. */
	float *this = va_arg(argList, float*);
	while(this != NULL)
	{
		mat4f_mult_mat4f_new(out, out, this);
		this = va_arg(argList,float*);
	}
	va_end(argList);
}

/** Multiply an arbitrary list of matrices together. The last matrix must be NULL.

    @param out The product of all of the other matrices.
    @param in One or more matrices to be multiplied together.
 */
void mat4d_mult_mat4d_many(double out[16], const double *in, ...)
{
	/* Initialize argList. */
	va_list argList;
	va_start(argList, in);

	/* Handle first matrix */
	mat4d_copy(out, in);

	/* Multiply all of the other matrices with the existing one until
	 * we reach NULL. */
	double *this = va_arg(argList, double*);
	while(this != NULL)
	{
		mat4d_mult_mat4d_new(out, out, this);
		this = va_arg(argList,double*);
	}
	va_end(argList);
}

/** Multiply an arbitrary list of matrices together. The last matrix must be NULL.

    @param out The product of all of the other matrices.
    @param in One or more matrices to be multiplied together.
 */
void mat3f_mult_mat3f_many(float out[9], const float *in, ...)
{
	/* Initialize argList. */
	va_list argList;
	va_start(argList, in);

	/* Handle first matrix */
	mat3f_copy(out, in);

	/* Multiply all of the other matrices with the existing one until
	 * we reach NULL. */
	float *this = va_arg(argList, float*);
	while(this != NULL)
	{
		mat3f_mult_mat3f_new(out, out, this);
		this = va_arg(argList,float*);
	}
	va_end(argList);
}

/** Multiply an arbitrary list of matrices together. The last matrix must be NULL.

    @param out The product of all of the other matrices.
    @param in One or more matrices to be multiplied together.
 */
void mat3d_mult_mat3d_many(double out[9], const double *in, ...)
{
	/* Initialize argList. */
	va_list argList;
	va_start(argList, in);

	/* Handle first matrix */
	mat3d_copy(out, in);

	/* Multiply all of the other matrices with the existing one until
	 * we reach NULL. */
	double *this = va_arg(argList, double*);
	while(this != NULL)
	{
		mat3d_mult_mat3d_new(out, out, this);
		this = va_arg(argList,double*);
	}
	va_end(argList);
}




/** Inverts a 4x4 float matrix.
 *
 * This works regardless of if we are treating the data as row major
 * or column major order because: (A^T)^-1 == (A^-1)^T
 *
 * @param out Location to store the inverted matrix.
 * @param m The matrix to invert.
 * @return Returns 1 if the matrix was inverted. Returns 0 if an error occurred. When an error occurs, a message is also printed out to standard out and the output matrix is left unchanged.
 */
int mat4f_invert_new(float out[16], const float m[16])
{
	float inv[16], det;
	inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
	inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
	inv[8] =   m[4]*m[9]*m[15]  - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
	inv[12]=  -m[4]*m[9]*m[14]  + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
	inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
	inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
	inv[9] =  -m[0]*m[9]*m[15]  + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
	inv[13] =  m[0]*m[9]*m[14]  - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
	inv[2] =   m[1]*m[6]*m[15]  - m[1]*m[7]*m[14]  - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7]  - m[13]*m[3]*m[6];
	inv[6] =  -m[0]*m[6]*m[15]  + m[0]*m[7]*m[14]  + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7]  + m[12]*m[3]*m[6];
	inv[10] =  m[0]*m[5]*m[15]  - m[0]*m[7]*m[13]  - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7]  - m[12]*m[3]*m[5];
	inv[14] = -m[0]*m[5]*m[14]  + m[0]*m[6]*m[13]  + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6]  + m[12]*m[2]*m[5];
	inv[3] =  -m[1]*m[6]*m[11]  + m[1]*m[7]*m[10]  + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7]   + m[9]*m[3]*m[6];
	inv[7] =   m[0]*m[6]*m[11]  - m[0]*m[7]*m[10]  - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7]   - m[8]*m[3]*m[6];
	inv[11] = -m[0]*m[5]*m[11]  + m[0]*m[7]*m[9]   + m[4]*m[1]*m[11] - m[4]*m[3]*m[9]  - m[8]*m[1]*m[7]   + m[8]*m[3]*m[5];
	inv[15] =  m[0]*m[5]*m[10]  - m[0]*m[6]*m[9]   - m[4]*m[1]*m[10] + m[4]*m[2]*m[9]  + m[8]*m[1]*m[6]   - m[8]*m[2]*m[5];
	det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
	if (det == 0)
	{
		msg(MSG_ERROR, "Failed to invert the following matrix\n");
		char str[256];
		matNf_print_to_string(str, 256, m, 4);
		msg(MSG_ERROR, "%s", str);
		return 0;
	}

	det = 1.0f / det;

	for(int i = 0; i < 16; i++)
		out[i] = inv[i] * det;

	return 1;
}

/** Inverts a 4x4 double matrix.
 *
 * This works regardless of if we are treating the data as row major
 * or column major order because: (A^T)^-1 == (A^-1)^T
 *
 * @param out Location to store the inverted matrix.
 * @param m The matrix to invert.
 * @return Returns 1 if the matrix was inverted. Returns 0 if an error occurred. When an error occurs, a message is also printed out to standard out and the output matrix is left unchanged.
 */
int mat4d_invert_new(double out[16], const double m[16])
{
	double inv[16], det;
	inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
	inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
	inv[8] =   m[4]*m[9]*m[15]  - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
	inv[12]=  -m[4]*m[9]*m[14]  + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
	inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
	inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
	inv[9] =  -m[0]*m[9]*m[15]  + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
	inv[13] =  m[0]*m[9]*m[14]  - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
	inv[2] =   m[1]*m[6]*m[15]  - m[1]*m[7]*m[14]  - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7]  - m[13]*m[3]*m[6];
	inv[6] =  -m[0]*m[6]*m[15]  + m[0]*m[7]*m[14]  + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7]  + m[12]*m[3]*m[6];
	inv[10] =  m[0]*m[5]*m[15]  - m[0]*m[7]*m[13]  - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7]  - m[12]*m[3]*m[5];
	inv[14] = -m[0]*m[5]*m[14]  + m[0]*m[6]*m[13]  + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6]  + m[12]*m[2]*m[5];
	inv[3] =  -m[1]*m[6]*m[11]  + m[1]*m[7]*m[10]  + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7]   + m[9]*m[3]*m[6];
	inv[7] =   m[0]*m[6]*m[11]  - m[0]*m[7]*m[10]  - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7]   - m[8]*m[3]*m[6];
	inv[11] = -m[0]*m[5]*m[11]  + m[0]*m[7]*m[9]   + m[4]*m[1]*m[11] - m[4]*m[3]*m[9]  - m[8]*m[1]*m[7]   + m[8]*m[3]*m[5];
	inv[15] =  m[0]*m[5]*m[10]  - m[0]*m[6]*m[9]   - m[4]*m[1]*m[10] + m[4]*m[2]*m[9]  + m[8]*m[1]*m[6]   - m[8]*m[2]*m[5];
	det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
	if (det == 0)
	{
		msg(MSG_ERROR, "Failed to invert the following matrix\n");
		char str[256];
		matNd_print_to_string(str, 256, m, 4);
		msg(MSG_ERROR, "%s", str);
		return 0;
	}

	det = 1.0 / det;

	for(int i = 0; i < 16; i++)
		out[i] = inv[i] * det;

	return 1;
}

/** Inverts a 3x3 float matrix.
 *
 * This works regardless of if we are treating the data as row major
 * or column major order because: (A^T)^-1 == (A^-1)^T
 *
 * @param out Location to store the inverted matrix.
 * @param m The matrix to invert.
 * @return Returns 1 if the matrix was inverted. Returns 0 if an error occurred. When an error occurs, a message is also printed out to standard out and the output matrix is left unchanged.
 */
int mat3f_invert_new(float out[9], const float m[9])
{
	float inv[9], det;
	inv[0] = m[4] * m[8] - m[5] * m[7];
	inv[3] = m[6] * m[5] - m[3] * m[8];
	inv[6] = m[3] * m[7] - m[6] * m[4];
	inv[1] = m[7] * m[2] - m[1] * m[8];
	inv[4] = m[0] * m[8] - m[6] * m[2];
	inv[7] = m[1] * m[6] - m[0] * m[7];
	inv[2] = m[1] * m[5] - m[2] * m[4];
	inv[5] = m[2] * m[3] - m[0] * m[5];
	inv[8] = m[0] * m[4] - m[1] * m[3];
	det = m[0]*inv[0] + m[3]*inv[1] + m[6]*inv[2];
	if (det == 0)
	{
		msg(MSG_ERROR, "Failed to invert the following matrix\n");
		char str[256];
		matNf_print_to_string(str, 256, m, 3);
		msg(MSG_ERROR, "%s", str);
		return 0;
	}

	det = 1.0f/det;

	for(int i = 0; i < 9; i++)
		out[i] = inv[i] * det;

	return 1;
}

/** Inverts a 3x3 double matrix.
 *
 * This works regardless of if we are treating the data as row major
 * or column major order because: (A^T)^-1 == (A^-1)^T
 *
 * @param out Location to store the inverted matrix.
 * @param m The matrix to invert.
 * @return Returns 1 if the matrix was inverted. Returns 0 if an error occurred. When an error occurs, a message is also printed out to standard out and the output matrix is left unchanged.
 */
int mat3d_invert_new(double out[9], const double m[9])
{
	double inv[9], det;
	inv[0] = m[4] * m[8] - m[5] * m[7];
	inv[3] = m[6] * m[5] - m[3] * m[8];
	inv[6] = m[3] * m[7] - m[6] * m[4];
	inv[1] = m[7] * m[2] - m[1] * m[8];
	inv[4] = m[0] * m[8] - m[6] * m[2];
	inv[7] = m[1] * m[6] - m[0] * m[7];
	inv[2] = m[1] * m[5] - m[2] * m[4];
	inv[5] = m[2] * m[3] - m[0] * m[5];
	inv[8] = m[0] * m[4] - m[1] * m[3];
	det = m[0]*inv[0] + m[3]*inv[1] + m[6]*inv[2];
	if (det == 0)
	{
		msg(MSG_ERROR, "Failed to invert the following matrix\n");
		char str[256];
		matNd_print_to_string(str, 256, m, 3);
		msg(MSG_ERROR, "%s", str);
		return 0;
	}

	det = 1.0/det;

	for(int i = 0; i < 9; i++)
		out[i] = inv[i] * det;

	return 1;
}
/** Inverts a 4x4 float matrix in place.
 * @param matrix The matrix to be inverted in place.
 * @return Returns 1 if the matrix was inverted. Returns 0 if an error occurred. When an error occurs, a message is also printed out to standard out and the matrix is left unchanged.
 */
int mat4f_invert(float  matrix[16])
{ return mat4f_invert_new(matrix, matrix); }
/** Inverts a 4x4 double matrix in place.
 * @param matrix The matrix to be inverted in place.
 * @return Returns 1 if the matrix was inverted. Returns 0 if an error occurred. When an error occurs, a message is also printed out to standard out and the matrix is left unchanged.
 */
int mat4d_invert(double matrix[16])
{ return mat4d_invert_new(matrix, matrix); }
/** Inverts a 3x3 float matrix in place.
 * @param matrix The matrix to be inverted in place.
 * @return Returns 1 if the matrix was inverted. Returns 0 if an error occurred. When an error occurs, a message is also printed out to standard out and the matrix is left unchanged.
 */
int mat3f_invert(float  matrix[ 9])
{ return mat3f_invert_new(matrix, matrix); }
/** Inverts a 3x3 double matrix in place.
 * @param matrix The matrix to be inverted in place.
 * @return Returns 1 if the matrix was inverted. Returns 0 if an error occurred. When an error occurs, a message is also printed out to standard out and the matrix is left unchanged.
 */
int mat3d_invert(double matrix[ 9])
{ return mat3d_invert_new(matrix, matrix); }


/** Creates a 3x3 rotation matrix of floats from Euler angles.

If order="XYZ", we will create a rotation matrix which rotates a point
around X, Y and then Z using intrinsic rotations. This results in a
single matrix that is comprised of three rotation matrices:
RotZ*RotY*RotX. If you prefer to think in extrinsic rotations, using
order="XYZ" is equivalent to rotating around Z, Y, and then X.

This implementation matches what Wolfram Alpha does except that the
signs on all of the angles need to be negated.  For example, see the
matrices for right-handed systems on Wikipedia and note that Wolfram
alpha has all of the sin()s negated---making it use a left-handed
system.

If you want to rotate a camera in OpenGL, you may wish to invert this
matrix (if R = RxRyRz, then R^-1 = Rz^-1 Ry^-1 Rx^-1). Otherwise, this
matrix can be applied to rotate vertices in an object.

Intended to work with:
XYZ XZY YXZ YZX ZXY ZYX (Tait-Bryan angles)
XYX XZX YXY YZY ZXZ ZYZ (Euler angles)

@param result The location to store the rotation matrix calculated
from the Euler angles.

@param a1_degrees The amount of rotation around the first axis in
degrees (-180 to 180).

@param a2_degrees The amount of rotation around the second axis in
degrees. If first and last rotation axes are different (Tait-Bryan
angles), must be between -90 to 90. If the first and last rotation
axes are the same (traditional Euler angles), must be between 0 and
180. When this parameter is near the limits, gimbal lock occurs.

@param a3_degrees The amount of rotation around the third axis in
degrees (-180 to 180).

@param order A string representing the order that the rotations should
be applied. In graphics, typically "XYZ".
*/
void mat3f_rotateEuler_new(float result[9], float a1_degrees, float a2_degrees, float a3_degrees, const char order[3])
{
	// Convert from degrees to radians
	float angles[3] = { a1_degrees, a2_degrees, a3_degrees };
	mat3f_identity(result);
	float rot[9];
	for(int i=0; i<3; i++)
	{
		if(order[i] == 'X' || order[i] == '1')
			mat3f_rotateAxis_new(rot, angles[i], 1, 0, 0);
		else if(order[i] == 'Y' || order[i] == '2')
			mat3f_rotateAxis_new(rot, angles[i], 0, 1, 0);
		else if(order[i] == 'Z' || order[i] == '3')
			mat3f_rotateAxis_new(rot, angles[i], 0, 0, 1);
		else
			msg(MSG_ERROR, "Unknown axis: %c\n", order[i]);
		mat3f_mult_mat3f_new(result, rot, result);
	}
}

/** Creates a 3x3 rotation matrix of doubles from intrinsic Euler
    angles. For full documentation, see mat3f_rotateEuler_new().

    @see mat3f_rotateEuler_new()
*/
void mat3d_rotateEuler_new(double result[9], double a1_degrees, double a2_degrees, double a3_degrees, const char order[3])
{
	// Convert from degrees to radians
	double angles[3] = { a1_degrees, a2_degrees, a3_degrees };
	mat3d_identity(result);
	double rot[9];
	for(int i=0; i<3; i++)
	{
		if(order[i] == 'X' || order[i] == '1')
			mat3d_rotateAxis_new(rot, angles[i], 1, 0, 0);
		else if(order[i] == 'Y' || order[i] == '2')
			mat3d_rotateAxis_new(rot, angles[i], 0, 1, 0);
		else if(order[i] == 'Z' || order[i] == '3')
			mat3d_rotateAxis_new(rot, angles[i], 0, 0, 1);
		else
			msg(MSG_ERROR, "Unknown axis: %c\n", order[i]);
		mat3d_mult_mat3d_new(result, rot, result);
	}
}

/** Creates a 4x4 rotation matrix of floats from intrinsic Euler
    angles. For full documentation, see mat3f_rotateEuler_new().

    @see mat3f_rotateEuler_new()
*/
void mat4f_rotateEuler_new(float result[16], float a1_degrees, float a2_degrees, float a3_degrees, const char order[3])
{
	float tmpMat[9];
	mat3f_rotateEuler_new(tmpMat, a1_degrees, a2_degrees, a3_degrees, order);
	mat4f_from_mat3f(result, tmpMat);
}

/** Creates a 4x4 rotation matrix of doubles from intrinsic Euler
    angles. For full documentation, see mat3f_rotateEuler_new().

    @see mat3f_rotateEuler_new()
*/
void mat4d_rotateEuler_new(double result[16], double a1_degrees, double a2_degrees, double a3_degrees, const char order[3])
{
	double tmpMat[9];
	mat3d_rotateEuler_new(tmpMat, a1_degrees, a2_degrees, a3_degrees, order);
	mat4d_from_mat3d(result, tmpMat);
}



/** Given a 3x3 rotation matrix and a Euler rotation ordering,
    calculate Euler angles that could be used to produce the matrix.

    Gimbal lock can occur and depending on the value of the second
    Euler angle. If you are using traditional Euler angles (first and
    last axis are the same), gimbal lock occurs when the second angle
    is either 0 or 180 degrees. If you are using Tait Brian angles
    (first and last axis are different), then gimbal lock occurs when
    the second angle is -90 or 90 degrees. In those cases, expect that
    the Euler->matrix->Euler conversions may not produce the same
    output Euler angle output as the input since there are multiple
    Euler angles representing the same orientation under gimbal lock.

    This implementation uses the method described in "Extracting Euler
    Angles from a Rotation Matrix" by Mike Day (Insomniac Games) to
    allow matrix->Euler->matrix conversion to have the output matrix
    be the same (or very similar) to the input matrix. Kevin
    Shoemake's "Euler Angle Conversion" in Graphics Gems IV also
    served as a source of inspiration for this code.
    
    @param angles The resulting Euler angles in degrees. The first and
    last angles will be in the range of -180 and 180 degrees. If using
    traditional Euler angles (first and last axis are the same), the
    second angle will be between 0 and 90. If using Tait-Bryan angles
    (first and last axis are different), the second angle will be
    between -90 and 90. If the second angle is near the range limits,
    gimbal lock has occurred or almost has occurred.
    
    @param m The rotation matrix to calculate the Euler angles from.
    
    @param order The axis ordering to use (for example "XYZ"). "XYZ"
    is commonly used in graphics and aerospace engineering (in OpenGL,
    where you are looking down -Z, the angles correspond to pitch,
    yaw, roll, respectively.).
*/
void eulerf_from_mat3f(float angles[3], const float m[9], const char order[3])
{
	/* Create an easy-to-use array of the axis ordering from the
	 * user-provided input. */
	int index[3] = { 0, 0, 0 };
	for(int i=0; i<3; i++)
	{
		if(order[i] == 'X' || order[i] == '1')
			index[i] = 0;
		else if(order[i] == 'Y' || order[i] == '2')
			index[i] = 1;
		else if(order[i] == 'Z' || order[i] == '3')
			index[i] = 2;
		else
			msg(MSG_ERROR, "Unknown axis: %c\n", order[i]);
	}

    // Check if the first and last rotations are around the same axis.
	if(index[0] == index[2])
	{
		float sign = 1;
		if((index[0] == 0 && index[1] == 1 && index[2] == 0) ||
		   (index[0] == 1 && index[1] == 2 && index[2] == 1) ||
		   (index[0] == 2 && index[1] == 0 && index[2] == 2))
		{
			sign = -1;
		}

		/* Set index[2] to indicate the 3rd dimension that was left out of
		   order. */
		if(index[0] != 0 && index[1] != 0 && index[2] != 0)
			index[2] = 0;
		if(index[0] != 1 && index[1] != 1 && index[2] != 1)
			index[2] = 1;
		if(index[0] != 2 && index[1] != 2 && index[2] != 2)
			index[2] = 2;

		// Make code easier to read by storing matrix values directly so
		// we don't need to calculate the location of the value in the 1D
		// array in each of our calculations.
		float index00 = m[mat3_getIndex(index[0],index[0])];
		float index01 = m[mat3_getIndex(index[0],index[1])];
		float index02 = m[mat3_getIndex(index[0],index[2])];
		float index10 = m[mat3_getIndex(index[1],index[0])];
		//float index11 = m[mat3_getIndex(index[1],index[1])];
		//float index12 = m[mat3_getIndex(index[1],index[2])];
		float index20 = m[mat3_getIndex(index[2],index[0])];
		//float index21 = m[mat3_getIndex(index[2],index[1])];
		float index22 = m[mat3_getIndex(index[2],index[2])];

		float sy = sqrtf(index01*index01 + index02*index02);
		angles[0] = atan2f(index01, -sign*index02);
		angles[1] = atan2f(sy, index00);
		angles[2] = atan2f(index10, sign*index20);

		if(angles[1] == 0)
		{
			angles[0] = acosf(index22);
			angles[2] = 0;
		}
	}
	else // first and last rotations are different axes - tait bryan
	{
		float sign = 1;
		if((index[0] == 1 && index[1] == 2 && index[2] == 0) ||
		   (index[0] == 2 && index[1] == 0 && index[2] == 1) ||
		   (index[0] == 0 && index[1] == 1 && index[2] == 2))
		{
			sign = -1;
		}

		// Make code easier to read by storing matrix values directly so
		// we don't need to calculate the location of the value in the 1D
		// array in each of our calculations.
		float index00 = m[mat3_getIndex(index[0],index[0])];
		float index01 = m[mat3_getIndex(index[0],index[1])];
		float index02 = m[mat3_getIndex(index[0],index[2])];
		float index10 = m[mat3_getIndex(index[1],index[0])];
		float index11 = m[mat3_getIndex(index[1],index[1])];
		float index12 = m[mat3_getIndex(index[1],index[2])];
		float index20 = m[mat3_getIndex(index[2],index[0])];
		float index21 = m[mat3_getIndex(index[2],index[1])];
		float index22 = m[mat3_getIndex(index[2],index[2])];

		float cy = sqrtf(index00*index00+index10*index10);
		angles[0] = -sign*atan2f(index21, index22);
		angles[1] = -sign*atan2f(-index20, cy);
		float s1= -sign*sinf(angles[0]);
		float c1=cosf(angles[0]);
		angles[2] = -sign*atan2f((s1*index02-c1*index01),
		                         (c1*index11-s1*index12));
	}


	// Convert to degrees.
	for(int i=0; i<3; i++)
		angles[i] = angles[i] * 180.0f/(float)M_PI;
}


/** Given a 3x3 rotation matrix and a Euler rotation ordering,
    calculate the Euler angles used to produce the matrix.

 @see eulerf_from_mat3f()
*/
void eulerd_from_mat3d(double angles[3], const double m[9], const char order[3])
{
	/* Create an easy-to-use array of the axis ordering from the
	 * user-provided input. */
	int index[3] = { 0, 0, 0 };
	for(int i=0; i<3; i++)
	{
		if(order[i] == 'X' || order[i] == '1')
			index[i] = 0;
		else if(order[i] == 'Y' || order[i] == '2')
			index[i] = 1;
		else if(order[i] == 'Z' || order[i] == '3')
			index[i] = 2;
		else
			msg(MSG_ERROR, "Unknown axis: %c\n", order[i]);
	}

    // Check if the first and last rotations are around the same axis.
	if(index[0] == index[2])
	{
		double sign = 1;
		if((index[0] == 0 && index[1] == 1 && index[2] == 0) ||
		   (index[0] == 1 && index[1] == 2 && index[2] == 1) ||
		   (index[0] == 2 && index[1] == 0 && index[2] == 2))
		{
			sign = -1;
		}

		/* Set index[2] to indicate the 3rd dimension that was left out of
		   order. */
		if(index[0] != 0 && index[1] != 0 && index[2] != 0)
			index[2] = 0;
		if(index[0] != 1 && index[1] != 1 && index[2] != 1)
			index[2] = 1;
		if(index[0] != 2 && index[1] != 2 && index[2] != 2)
			index[2] = 2;

		// Make code easier to read by storing matrix values directly so
		// we don't need to calculate the location of the value in the 1D
		// array in each of our calculations.
		double index00 = m[mat3_getIndex(index[0],index[0])];
		double index01 = m[mat3_getIndex(index[0],index[1])];
		double index02 = m[mat3_getIndex(index[0],index[2])];
		double index10 = m[mat3_getIndex(index[1],index[0])];
		//double index11 = m[mat3_getIndex(index[1],index[1])];
		//double index12 = m[mat3_getIndex(index[1],index[2])];
		double index20 = m[mat3_getIndex(index[2],index[0])];
		//double index21 = m[mat3_getIndex(index[2],index[1])];
		double index22 = m[mat3_getIndex(index[2],index[2])];

		double sy = sqrt(index01*index01 + index02*index02);
		angles[0] = atan2(index01, -sign*index02);
		angles[1] = atan2(sy, index00);
		angles[2] = atan2(index10, sign*index20);

		if(angles[1] == 0)
		{
			angles[0] = acos(index22);
			angles[2] = 0;
		}
	}
	else // first and last rotations are different axes - tait bryan
	{
		double sign = 1;
		if((index[0] == 1 && index[1] == 2 && index[2] == 0) ||
		   (index[0] == 2 && index[1] == 0 && index[2] == 1) ||
		   (index[0] == 0 && index[1] == 1 && index[2] == 2))
		{
			sign = -1;
		}

		// Make code easier to read by storing matrix values directly so
		// we don't need to calculate the location of the value in the 1D
		// array in each of our calculations.
		double index00 = m[mat3_getIndex(index[0],index[0])];
		double index01 = m[mat3_getIndex(index[0],index[1])];
		double index02 = m[mat3_getIndex(index[0],index[2])];
		double index10 = m[mat3_getIndex(index[1],index[0])];
		double index11 = m[mat3_getIndex(index[1],index[1])];
		double index12 = m[mat3_getIndex(index[1],index[2])];
		double index20 = m[mat3_getIndex(index[2],index[0])];
		double index21 = m[mat3_getIndex(index[2],index[1])];
		double index22 = m[mat3_getIndex(index[2],index[2])];

		double cy = sqrt(index00*index00+index10*index10);
		angles[0] = -sign*atan2(index21, index22);
		angles[1] = -sign*atan2(-index20, cy);
		double s1= -sign*sin(angles[0]);
		double c1=cos(angles[0]);
		angles[2] = -sign*atan2((s1*index02-c1*index01),
		                        (c1*index11-s1*index12));
	}


	// Convert to degrees.
	for(int i=0; i<3; i++)
		angles[i] = angles[i] * 180/M_PI;
}


/** Given a 4x4 rotation matrix and a Euler rotation ordering,
 calculate the Euler angles used to produce the matrix.

 @see eulerf_from_mat3f()
*/
void eulerf_from_mat4f(float angles[3], const float m[16], const char order[3])
{
	float tmpMat[9];
	mat3f_from_mat4f(tmpMat, m);
	eulerf_from_mat3f(angles, tmpMat, order);
}
/** Given a 4x4 rotation matrix and a Euler rotation ordering,
 calculate the Euler angles used to produce the matrix.

 @see eulerf_from_mat3f()
*/
void eulerd_from_mat4d(double angles[3], const double m[16], const char order[3])
{
	double tmpMat[9];
	mat3d_from_mat4d(tmpMat, m);
	eulerd_from_mat3d(angles, tmpMat, order);
}


/** Create a 3x3 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axis A vector representing the axis to rotate around (must have a non-zero length). */
void mat3f_rotateAxisVec_new(float result[9], float degrees, const float axis[3])
{
	float angle = (float)(degrees * M_PI/180.0f);
	float c = cosf(angle);
	float s = sinf(angle);
	float t = 1-c;
	// 1-c is numerically unsound when angle is small.
	// See: https://en.wikipedia.org/wiki/Loss_of_significance
	// Use fix described at:
	// http://math.stackexchange.com/questions/38144
	if(c > .9)
		t = 2.0f * sinf(angle/2.0f)*sinf(angle/2.0f);

	// If zero vector is passed in, return identity matrix
	float length = vec3f_norm(axis);
	if(length < .00001f)
	{
		msg(MSG_ERROR, "Vector to rotate around was 0!");
		mat3f_identity(result);
		return;
	}

	float x = axis[0]/length;
	float y = axis[1]/length;
	float z = axis[2]/length;

	// first row
	result[0] = x*x*t+c;
	result[3] = x*y*t-z*s;
	result[6] = x*z*t+y*s;

	// second row
	result[1] = y*x*t+z*s;
	result[4] = y*y*t+c;
	result[7] = y*z*t-x*s;

	// third row
	result[2] = z*x*t-y*s;
	result[5] = z*y*t+x*s;
	result[8] = z*z*t+c;
}


/** Create a 3x3 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axis A vector representing the axis to rotate around (must have a non-zero length). */
void mat3d_rotateAxisVec_new(double result[9], double degrees, const double axis[3])
{
	double angle = degrees * M_PI/180;
	double c = cos(angle);
	double s = sin(angle);
	double t = 1-c;
	// 1-c is numerically unsound when angle is small.
	// See: https://en.wikipedia.org/wiki/Loss_of_significance
	// Use fix described at:
	// http://math.stackexchange.com/questions/38144
	if(angle < .01)
		t = 2.0 * sin(angle/2.0)*sin(angle/2.0);

	// If zero vector is passed in, return identity matrix
	double length = vec3d_norm(axis);
	if(length < .00001)
	{
		msg(MSG_ERROR, "Vector to rotate around was 0!");
		mat3d_identity(result);
		return;
	}

	double x = axis[0]/length;
	double y = axis[1]/length;
	double z = axis[2]/length;

	// first row
	result[0] = x*x*t+c;
	result[3] = x*y*t-z*s;
	result[6] = x*z*t+y*s;

	// second row
	result[1] = y*x*t+z*s;
	result[4] = y*y*t+c;
	result[7] = y*z*t-x*s;

	// third row
	result[2] = z*x*t-y*s;
	result[5] = z*y*t+x*s;
	result[8] = z*z*t+c;
}

/** Create a 4x4 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axis A vector representing the axis to rotate around (must have a non-zero length). */
void mat4f_rotateAxisVec_new(float result[16], float degrees, const float axis[3])
{
	float tmpMat[9];
	mat3f_rotateAxisVec_new(tmpMat, degrees, axis);
	mat4f_from_mat3f(result, tmpMat);
}
/** Create a 4x4 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axis A vector representing the axis to rotate around (must have a non-zero length). */
void mat4d_rotateAxisVec_new(double result[16], double degrees, const double axis[3])
{
	double tmpMat[9];
	mat3d_rotateAxisVec_new(tmpMat, degrees, axis);
	mat4d_from_mat3d(result, tmpMat);
}

/** Create a 3x3 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axisX The x-component of the axis to rotate around.
 * @param axisY The y-component of the axis to rotate around.
 * @param axisZ The z-component of the axis to rotate around.
 */
void mat3f_rotateAxis_new(float  result[ 9], float  degrees, float axisX, float axisY, float axisZ)
{
	float vec[3];
	vec3f_set(vec, axisX, axisY, axisZ);
	mat3f_rotateAxisVec_new(result, degrees, vec);
}
/** Create a 3x3 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axisZ The x-component of the axis to rotate around.
 * @param axisY The y-component of the axis to rotate around.
 * @param axisZ The z-component of the axis to rotate around.
 */
void mat3d_rotateAxis_new(double result[ 9], double degrees, double axisX, double axisY, double axisZ)
{
	double vec[3];
	vec3d_set(vec, axisX, axisY, axisZ);
	mat3d_rotateAxisVec_new(result, degrees, vec);
}
/** Create a 4x4 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axisX The x-component of the axis to rotate around.
 * @param axisY The y-component of the axis to rotate around.
 * @param axisZ The z-component of the axis to rotate around.
 */
void mat4f_rotateAxis_new(float  result[16], float  degrees, float axisX, float axisY, float axisZ)
{
	float vec[3];
	vec3f_set(vec, axisX, axisY, axisZ);
	mat4f_rotateAxisVec_new(result, degrees, vec);
}
/** Create a 4x4 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axisX The x-component of the axis to rotate around.
 * @param axisY The y-component of the axis to rotate around.
 * @param axisZ The z-component of the axis to rotate around.
 */
void mat4d_rotateAxis_new(double result[16], double degrees, double axisX, double axisY, double axisZ)
{
	double vec[3];
	vec3d_set(vec, axisX, axisY, axisZ);
	mat4d_rotateAxisVec_new(result, degrees, vec);
}


/** Creates a 3x3 rotation matrix from a quaternion (x,y,z,w).

    This method makes assumptions that are commonly made in this file:
    A column vector is multiplied on the left of the matrix produced
    by this function. We are using a right-handed coordinate system
    and right-handed rotations.

    This code is based on Ken Shoemake's SIGGRAPH Tutorial on Quaternions:
    http://www.cs.ucr.edu/~vbz/resources/quatut.pdf

    @param matrix The location to store the output matrix.
   
    @param quat The input quaternion. The quaternion does not need
    to be unit length.
*/
void mat3f_rotateQuatVec_new(float matrix[9], const float quat[4])
{
	int X=0, Y=1, Z=2, W=3;
	float s = 2.0f / (quat[X]*quat[X] + quat[Y]*quat[Y] +
	                  quat[Z]*quat[Z] + quat[W]*quat[W]);
	float xs, ys, zs,
	      wx, wy, wz,
	      xx, xy, xz,
	      yy, yz, zz;

	xs = quat[X] * s;   ys = quat[Y] * s;   zs = quat[Z] * s;
	wx = quat[W] * xs;  wy = quat[W] * ys;  wz = quat[W] * zs;
	xx = quat[X] * xs;  xy = quat[X] * ys;  xz = quat[X] * zs;
	yy = quat[Y] * ys;  yz = quat[Y] * zs;  zz = quat[Z] * zs;

	// first row
	matrix[0] = 1.0f - (yy + zz);
	matrix[3] = xy - wz;
	matrix[6] = xz + wy;

	// second row
	matrix[1] = xy + wz;
	matrix[4] = 1.0f - (xx + zz);
	matrix[7] = yz - wx;

	// third row
	matrix[2] = xz - wy;
	matrix[5] = yz + wx;
	matrix[8] = 1.0f - (xx + yy);
}

/** Creates a 3x3 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat3f_rotateQuatVec_new() */
void mat3d_rotateQuatVec_new(double matrix[9], const double quat[4])
{
	int X=0, Y=1, Z=2, W=3;
	double s = 2.0 / (quat[X]*quat[X] + quat[Y]*quat[Y] +
	                 quat[Z]*quat[Z] + quat[W]*quat[W]);
	double xs, ys, zs,
	       wx, wy, wz,
	       xx, xy, xz,
	       yy, yz, zz;

	xs = quat[X] * s;   ys = quat[Y] * s;   zs = quat[Z] * s;
	wx = quat[W] * xs;  wy = quat[W] * ys;  wz = quat[W] * zs;
	xx = quat[X] * xs;  xy = quat[X] * ys;  xz = quat[X] * zs;
	yy = quat[Y] * ys;  yz = quat[Y] * zs;  zz = quat[Z] * zs;

	// first row
	matrix[0] = 1.0 - (yy + zz);
	matrix[3] = xy - wz;
	matrix[6] = xz + wy;

	// second row
	matrix[1] = xy + wz;
	matrix[4] = 1.0 - (xx + zz);
	matrix[7] = yz - wx;

	// third row
	matrix[2] = xz - wy;
	matrix[5] = yz + wx;
	matrix[8] = 1.0 - (xx + yy);
}
/** Creates a 4x4 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat4f_rotateQuatVec_new(float matrix[16], const float quat[4])
{
	float tmpMat[9];
	mat3f_rotateQuatVec_new(tmpMat, quat);
	mat4f_from_mat3f(matrix, tmpMat);
}
/** Creates a 4x4 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat4d_rotateQuatVec_new(double matrix[16], const double quat[4])
{
	double tmpMat[9];
	mat3d_rotateQuatVec_new(tmpMat, quat);
	mat4d_from_mat3d(matrix, tmpMat);
}
/** Creates a 3x3 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat3f_rotateQuat_new(float matrix[9], float x, float y, float z, float w)
{
	float quat[4] = { x,y,z,w };
	mat3f_rotateQuatVec_new(matrix, quat);
}
/** Creates a 3x3 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat3d_rotateQuat_new(double matrix[9], double x, double y, double z, double w)
{
	double quat[4] = { x,y,z,w };
	mat3d_rotateQuatVec_new(matrix, quat);
}
/** Creates a 4x4 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat4f_rotateQuat_new(float matrix[16], float x, float y, float z, float w)
{
	float quat[4] = { x,y,z,w };
	mat4f_rotateQuatVec_new(matrix, quat);
}
/** Creates a 4x4 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat4d_rotateQuat_new(double matrix[16], double x, double y, double z, double w)
{
	double quat[4] = { x,y,z,w };
	mat4d_rotateQuatVec_new(matrix, quat);
}

/** Creates a unit quaternion (x,y,z,w) from a rotation matrix.

    This code is based on Ken Shoemake's SIGGRAPH Tutorial on Quaternions:
    http://www.cs.ucr.edu/~vbz/resources/quatut.pdf
    It is also based code in quat.c on VRPN 7.26 (public domain).

    @param matrix The location to store the output matrix.
   
    @param quat The input quaternion. The quaternion does not need
    to be unit length.
*/
void quatf_from_mat3f(float quat[4], const float matrix[9])
{
	int X=0, Y=1, Z=2, W=3;
	float trace = matrix[0]+matrix[4]+matrix[8]; // sum of diagonal

   if (trace > 0.0f)
   {
	   float s = sqrtf(trace + 1.0f);
	   quat[W] = s * 0.5f;
	   s = 0.5f / s;

	   quat[X] = (matrix[mat3_getIndex(Z,Y)] - matrix[mat3_getIndex(Y,Z)]) * s;
	   quat[Y] = (matrix[mat3_getIndex(X,Z)] - matrix[mat3_getIndex(Z,X)]) * s;
	   quat[Z] = (matrix[mat3_getIndex(Y,Z)] - matrix[mat3_getIndex(X,Y)]) * s;
   }

   else
   {
	   int next[3] = {Y, Z, X};
	   int i = X;
	   if (matrix[mat3_getIndex(Y,Y)] > matrix[mat3_getIndex(X,X)])
		   i = Y;
	   if (matrix[mat3_getIndex(Z,Z)] > matrix[mat3_getIndex(i,i)])
		   i = Z;
	   int j = next[i];
	   int k = next[j];
	   
	   float s = sqrtf( (matrix[mat3_getIndex(i,i)] - (matrix[mat3_getIndex(j,j)] + matrix[mat3_getIndex(k,k)])) + 1.0f );
	   quat[i] = s * 0.5f;
	   
	   s = 0.5f / s;
	   
	   quat[W] = (matrix[mat3_getIndex(k,j)] - matrix[mat3_getIndex(j,k)]) * s;
	   quat[j] = (matrix[mat3_getIndex(j,i)] + matrix[mat3_getIndex(i,j)]) * s;
	   quat[k] = (matrix[mat3_getIndex(k,i)] + matrix[mat3_getIndex(i,k)]) * s;
   }
}
/** Creates a unit quaternion (x,y,z,w) from a rotation matrix. For full documentation, see quatf_from_mat3f() */
void quatd_from_mat3d(double quat[4], const double matrix[9])
{
	int X=0, Y=1, Z=2, W=3;
	double trace = matrix[0]+matrix[4]+matrix[8]; // sum of diagonal

   if (trace > 0.0)
   {
	   double s = sqrt(trace + 1.0);
	   quat[W] = s * 0.5;
	   s = 0.5 / s;

	   quat[X] = (matrix[mat3_getIndex(Z,Y)] - matrix[mat3_getIndex(Y,Z)]) * s;
	   quat[Y] = (matrix[mat3_getIndex(X,Z)] - matrix[mat3_getIndex(Z,X)]) * s;
	   quat[Z] = (matrix[mat3_getIndex(Y,Z)] - matrix[mat3_getIndex(X,Y)]) * s;
   }

   else
   {
	   int next[3] = {Y, Z, X};
	   int i = X;
	   if (matrix[mat3_getIndex(Y,Y)] > matrix[mat3_getIndex(X,X)])
		   i = Y;
	   if (matrix[mat3_getIndex(Z,Z)] > matrix[mat3_getIndex(i,i)])
		   i = Z;
	   int j = next[i];
	   int k = next[j];
	   
	   double s = sqrt( (matrix[mat3_getIndex(i,i)] - (matrix[mat3_getIndex(j,j)] + matrix[mat3_getIndex(k,k)])) + 1.0 );
	   quat[i] = s * 0.5;
	   
	   s = 0.5 / s;

	   quat[W] = (matrix[mat3_getIndex(k,j)] - matrix[mat3_getIndex(j,k)]) * s;
	   quat[j] = (matrix[mat3_getIndex(j,i)] + matrix[mat3_getIndex(i,j)]) * s;
	   quat[k] = (matrix[mat3_getIndex(k,i)] + matrix[mat3_getIndex(i,k)]) * s;
   }
}

/** Creates a unit quaternion (x,y,z,w) from a rotation matrix. For full documentation, see quatf_from_mat3f() */
void quatf_from_mat4f(float quat[4], const float matrix[16])
{
	float tmpMat[9];
	mat3f_from_mat4f(tmpMat, matrix);
	quatf_from_mat3f(quat, tmpMat);
}
/** Creates a unit quaternion (x,y,z,w) from a rotation matrix. For full documentation, see quatf_from_mat3f() */
void quatd_from_mat4d(double quat[4], const double matrix[16])
{
	double tmpMat[9];
	mat3d_from_mat4d(tmpMat, matrix);
	quatd_from_mat3d(quat, tmpMat);
}

/** Creates a quaternion (x,y,z,w) based on an axis and the number of degrees to rotate around that axis. 

    Based code in quat.c on VRPN 7.26 (public domain).

    @param quat The location to store the output quaternion. If the axis is a zero vector, the identity quaternion is returned.
    @param x The x-component of a vector representing the axis to rotate around.
    @param y The y-component of a vector representing the axis to rotate around.
    @param z The z-component of a vector representing the axis to rotate around.
    @param degrees The amount to rotate around the given axis in degrees.
*/
void quatf_rotateAxis_new(float quat[4], float degrees, float x, float y, float z)
{
	int X=0,Y=1,Z=2,W=3;
	// Angle needs to be negated to make it correspond to the behavior of mat3f_rotateAxis_new().
	float angle = (float)(-degrees * M_PI/180.0f);

	/* normalize vector */
	float length = sqrtf( x*x + y*y + z*z );

	/* If zero vector passed in for the axis, just return identity quaternion   */
	if (length < 1e-10) {
		quat[X] = 0;
		quat[Y] = 0;
		quat[Z] = 0;
		quat[W] = 1;
		return;
	}

	x /= length;
	y /= length;
	z /= length;

	float cosA = cosf(angle / 2.0f);
	float sinA = sinf(angle / 2.0f);
	quat[W] = cosA;
	quat[X] = sinA * x;
	quat[Y] = sinA * y;
	quat[Z] = sinA * z;
}

/** Creates a quaternion (x,y,z,w) based on an axis and the number of
 * degrees to rotate around that axis. See quatf_rotateAxis_new() for
 * full documentation.
*/
void quatd_rotateAxis_new(double quat[4], double degrees, double x, double y, double z)
{
	int X=0,Y=1,Z=2,W=3;
	// Angle needs to be negated to make it correspond to the behavior of mat3f_rotateAxis_new().
	double angle = -degrees * M_PI/180;

	/* normalize vector */
	double length = sqrt( x*x + y*y + z*z );

	/* If zero vector passed in for the axis, just return identity quaternion   */
	if (length < 1e-10) {
		quat[X] = 0;
		quat[Y] = 0;
		quat[Z] = 0;
		quat[W] = 1;
		return;
	}

	x /= length;
	y /= length;
	z /= length;

	double cosA = cos(angle / 2.0);
	double sinA = sin(angle / 2.0);
	quat[W] = cosA;
	quat[X] = sinA * x;
	quat[Y] = sinA * y;
	quat[Z] = sinA * z;
}
/** Creates a quaternion (x,y,z,w) based on an axis and the number of
 * degrees to rotate around that axis. See quatf_rotateAxis_new() for
 * full documentation.
*/
void quatf_rotateAxisVec_new(float quat[4], float degrees, const float axis[3])
{
	quatf_rotateAxis_new(quat, degrees, axis[0], axis[1], axis[2]);
}

/** Creates a quaternion (x,y,z,w) based on an axis and the number of
 * degrees to rotate around that axis. See quatf_rotateAxis_new() for
 * full documentation.
*/
void quatd_rotateAxisVec_new(double quat[4], double degrees, const double axis[3])
{
	quatd_rotateAxis_new(quat, degrees, axis[0], axis[1], axis[2]);
}


/** Spherical linear interpolation of unit quaternion.

 This code is based on Ken Shoemake's code and is in the public
 domain.

 @param result The interpolated quaternion.
 @param start The starting quaternion.
 @param end The ending quaternion.
 @param t As t goes from 0 to 1, the "result" quaternion goes from the
 "start" quaternion to the "end" quaternion. The routine should always
 return a point along the shorter of the two paths between the two
 (the vector may be negated in the end).
 */
void quatf_slerp_new(float result[4], const float start[4], const float end[4], float t)
{
	float copyOfStart[4];
	vec4f_copy(copyOfStart, start);
	float cosOmega = vec4f_dot(start, end);

	if(cosOmega<0)
	{
		cosOmega = -cosOmega;
		vec4f_scalarMult(copyOfStart, -1);
	}
	
	if(1+cosOmega > 1e-10)
	{
		float startScale, endScale;
		if(1-cosOmega > 1e-10)
		{
			float omega = acosf(cosOmega);
			float sinOmega = sinf(omega);
			startScale = sinf((1.0f-t)*omega / sinOmega);
			endScale = sinf(t*omega)/sinOmega;
		}
		else
		{
			startScale = 1.0f-t;
			endScale = t;
		}
		float scaledStart[4], scaledEnd[4];
		vec4f_scalarMult_new(scaledStart, copyOfStart, startScale);
		vec4f_scalarMult_new(scaledEnd,   end,         endScale);
		vec4f_add_new(result, scaledStart, scaledEnd);
	}
	else
	{
		vec4f_set(result, -copyOfStart[1], copyOfStart[0], -copyOfStart[3], copyOfStart[2]);
		float startScale = sinf((0.5f-t)*((float)M_PI));
		float endScale = sinf(t*((float)M_PI));
		float scaledStart[4], scaledEnd[4];
		vec4f_scalarMult_new(scaledStart, copyOfStart,  startScale);
		vec4f_scalarMult_new(scaledEnd,   result,       endScale);
		vec4f_add_new(result, scaledStart, scaledEnd);
	}
	//vec4f_normalize(result);
}

/** Spherical linear interpolation of unit quaternion.

 This code is based on Ken Shoemake's code and is in the public
 domain.

 @param result The interpolated quaternion.
 @param start The starting quaternion.
 @param end The ending quaternion.
 @param t As t goes from 0 to 1, the "result" quaternion goes from the
 "start" quaternion to the "end" quaternion. The routine should always
 return a point along the shorter of the two paths between the two
 (the vector may be negated in the end).
 */
void quatd_slerp_new(double result[4], const double start[4], const double end[4], double t)
{
	double copyOfStart[4];
	vec4d_copy(copyOfStart, start);
	double cosOmega = vec4d_dot(start, end);

	if(cosOmega<0)
	{
		cosOmega = -cosOmega;
		vec4d_scalarMult(copyOfStart, -1);
	}
	
	if(1+cosOmega > 1e-10)
	{
		double startScale, endScale;
		if(1-cosOmega > 1e-10)
		{
			double omega = acos(cosOmega);
			double sinOmega = sin(omega);
			startScale = sin((1.0-t)*omega / sinOmega);
			endScale = sin(t*omega)/sinOmega;
		}
		else
		{
			startScale = 1.0-t;
			endScale = t;
		}
		double scaledStart[4], scaledEnd[4];
		vec4d_scalarMult_new(scaledStart, copyOfStart, startScale);
		vec4d_scalarMult_new(scaledEnd,   end,         endScale);
		vec4d_add_new(result, scaledStart, scaledEnd);
	}
	else
	{
		vec4d_set(result, -copyOfStart[1], copyOfStart[0], -copyOfStart[3], copyOfStart[2]);
		double startScale = sin((0.5-t)*M_PI);
		double endScale = sin(t*M_PI);
		double scaledStart[4], scaledEnd[4];
		vec4d_scalarMult_new(scaledStart, copyOfStart, startScale);
		vec4d_scalarMult_new(scaledEnd,   result,      endScale);
		vec4d_add_new(result, scaledStart, scaledEnd);
	}
	vec4d_normalize(result);
}

	


/** Creates a new 4x4 float translation matrix with the rest of the matrix set to the identity.
    @param result The location to store the new translation matrix.
    @param x The x coordinate to be placed into the matrix.
    @param y The y coordinate to be placed into the matrix.
    @param z The z coordinate to be placed into the matrix.
*/
void mat4f_translate_new(float  result[16], float x, float y, float z)
{
	mat4f_identity(result);
	result[12] = x;
	result[13] = y;
	result[14] = z;
	result[15] = 1;
}
/** Creates a new 4x4 double translation matrix with the rest of the matrix set to the identity.
    @param result The location to store the new translation matrix.
    @param x The x coordinate to be placed into the matrix.
    @param y The y coordinate to be placed into the matrix.
    @param z The z coordinate to be placed into the matrix.
*/
void mat4d_translate_new(double result[16], double x, double y, double z)
{
	mat4d_identity(result);
	result[12] = x;
	result[13] = y;
	result[14] = z;
	result[15] = 1;
}
/** Creates a new 4x4 float translation matrix with the rest of the matrix set to the identity.
    @param result The location to store the new translation matrix.
    @param xyz A vector containing the translation value to put in the matrix.
*/
void mat4f_translateVec_new(float  result[16], const float  xyz[3])
{ mat4f_translate_new(result, xyz[0], xyz[1], xyz[2]); }
/** Creates a new 4x4 double translation matrix with the rest of the matrix set to the identity.
    @param result The location to store the new translation matrix.
    @param xyz A vector containing the translation value to put in the matrix.
*/
void mat4d_translateVec_new(double result[16], const double xyz[3])
{ mat4d_translate_new(result, xyz[0], xyz[1], xyz[2]); }

/** Creates a view frustum projection matrix (float). This
 * creates a matrix similar to the one that glFrustum() would
 * apply to the OpenGL 2.0 matrix stack. A simpler (but less
 * flexible) alternative to this function is mat4f_perspective_new().
 * Prints a message and returns the identity matrix on error.
 *
 * @param result The resulting 4x4 view frustum projection matrix.
 *
 * @param left Coordinate of left edge of the screen.
 * @param right Coordinate of right edge of the screen.
 * @param bottom Coordinate of bottom edge of the screen.
 * @param top Coordinate of top edge of the screen.
 * @param near Near clipping plane distance (positive).
 * @param far Far clipping plane distance (positive).
 */
void mat4f_frustum_new(float result[16], float left, float right, float bottom, float top, float near, float far)
{
	// glFrustum() requires near and far to be positive numbers.
	near = fabsf(near);
	far  = fabsf(far);
	mat4f_identity(result);
	if(left == right || bottom == top || near == far)
	{
		msg(MSG_ERROR, "Frustum values would result in divide by zero.");
		msg(MSG_ERROR, "Frustum values were: l=%f r=%f b=%f t=%f n=%f f=%f",
		    left, right, bottom, top, near, far);
		return;
	}
	if(near == 0)
	{
		msg(MSG_WARNING, "Near plane should be a value greater than 0.");
		msg(MSG_WARNING, "Frustum values were: l=%f r=%f b=%f t=%f n=%f f=%f",
		    left, right, bottom, top, near, far);
		
	}
	if(left > right || bottom > top || near > far)
	{
		msg(MSG_WARNING, "Frustum values seemed to be swapped (e.g., left should be less than right).");
		msg(MSG_WARNING, "Frustum values were: l=%f r=%f b=%f t=%f n=%f f=%f",
		    left, right, bottom, top, near, far);
	}
	result[0]  =  2.0f * near    / (right - left);
    result[5]  =  2.0f * near    / (top   - bottom);
	result[8]  =  (right + left) / (right - left);
    result[9]  =  (top + bottom) / (top   - bottom);
    result[10] = -(far + near)   / (far - near);
    result[11] = -1.0f;
    result[14] = -(2.0f * far * near) / (far - near);
    result[15] =  0.0f;
}
/** Creates a view frustum projection matrix (double). This
 * creates a matrix similar to the one that glFrustum() would
 * apply to the OpenGL 2.0 matrix stack. A simpler (but less
 * flexible) alternative to this function is mat4f_perspective_new().
 * Prints a message and returns the identity matrix on error.
 *
 * @param result The resulting 4x4 view frustum projection matrix.
 *
 * @param left Coordinate of left edge of the screen.
 * @param right Coordinate of right edge of the screen.
 * @param bottom Coordinate of bottom edge of the screen.
 * @param top Coordinate of top edge of the screen.
 * @param near Near clipping plane distance (positive).
 * @param far Far clipping plane distance (positive).
 */
void mat4d_frustum_new(double result[16], double left, double right, double bottom, double top, double near, double far)
{
	// glFrustum() requires near and far to be positive numbers.
	near = fabs(near);
	far  = fabs(far);
	mat4d_identity(result);
	if(left == right || bottom == top || near == far)
	{
		msg(MSG_ERROR, "Frustum values would result in divide by zero.");
		msg(MSG_ERROR, "Frustum values were: l=%f r=%f b=%f t=%f n=%f f=%f",
		    left, right, bottom, top, near, far);
		return;
	}
	if(near == 0)
	{
		msg(MSG_WARNING, "Near plane should be a value greater than 0.");
		msg(MSG_WARNING, "Frustum values were: l=%f r=%f b=%f t=%f n=%f f=%f",
		    left, right, bottom, top, near, far);
		
	}
	if(left > right || bottom > top || near > far)
	{
		msg(MSG_WARNING, "Frustum values seemed to be swapped (e.g., left should be less than right).");
		msg(MSG_WARNING, "Frustum values were: l=%f r=%f b=%f t=%f n=%f f=%f",
		    left, right, bottom, top, near, far);
	}
	result[0]  =  2.0f * near    / (right - left);
    result[5]  =  2.0f * near    / (top   - bottom);
	result[8]  =  (right + left) / (right - left);
    result[9]  =  (top + bottom) / (top   - bottom);
    result[10] = -(far + near)   / (far   - near);
    result[11] = -1.0f;
    result[14] = -(2.0f * far * near) / (far - near);
    result[15] =  0.0f;
}

/** Creates a orthographic projection matrix (float). This creates a
 * matrix similar to the one that glOrtho() would apply to the OpenGL
 * 2.0 matrix stack. Prints a message and returns the identity matrix
 * on error.
 *
 * @param result The resulting 4x4 orthographic projection matrix.
 *
 * @param left Coordinate of left edge of the screen.
 * @param right Coordinate of right edge of the screen.
 * @param bottom Coordinate of bottom edge of the screen.
 * @param top Coordinate of top edge of the screen.
 * @param near Near clipping plane distance (negative values are behind the viewer).
 * @param far Far clipping plane distance (negative values are behind the viewer).
 */
void mat4f_ortho_new(float result[16], float left, float right, float bottom, float top, float near, float far)
{
	mat4f_identity(result);
	if(left == right || bottom == top || near == far)
	{
		msg(MSG_ERROR, "Invalid orthographic projection matrix.\n");
		return;
	}
	result[0]  =  2 / (right-left);
	result[5]  =  2 / (top-bottom);
	result[10] = -2 / (far-near);
	result[12] = -(right+left)/(right-left);
	result[13] = -(top+bottom)/(top-bottom);
	result[14] = -(far+near)/(far-near);
}

/** Creates a orthographic projection matrix (double). This creates a
 * matrix similar to the one that glOrtho() would apply to the OpenGL
 * 2.0 matrix stack. Prints a message and returns the identity matrix
 * on error.
 *
 * @param result The resulting 4x4 orthographic projection matrix.
 *
 * @param left Coordinate of left edge of the screen.
 * @param right Coordinate of right edge of the screen.
 * @param bottom Coordinate of bottom edge of the screen.
 * @param top Coordinate of top edge of the screen.
 * @param near Near clipping plane distance (negative values are behind the viewer).
 * @param far Far clipping plane distance (negative values are behind the viewer).
 */
void mat4d_ortho_new(double result[16], double left, double right, double bottom, double top, double near, double far)
{
	mat4d_identity(result);
	if(left == right || bottom == top || near == far)
	{
		msg(MSG_ERROR, "Invalid orthographic projection matrix.\n");
		return;
	}
	result[0]  =  2 / (right-left);
	result[5]  =  2 / (top-bottom);
	result[10] = -2 / (far-near);
	result[12] = -(right+left)/(right-left);
	result[13] = -(top+bottom)/(top-bottom);
	result[14] = -(far+near)/(far-near);
}

/** Creates a perspective projection matrix (float). This creates a matrix
 * that is similar to what gluPerspective() would typically apply to the
 * matrix stack earlier versions of OpenGL.
 * Prints a message and returns the identity matrix on error.
 *
 * @param result The resulting 4x4 view frustum projection matrix.
 *
 * @param fovy The field of view in the horizontal direction (degrees)
 *
 * @param aspect The aspect ratio of the screen/window/viewport. The
 * aspect ratio is the width of the screen divided by the height of
 * the screen. Larger numbers mean wider screens.
 *
 * @param near Near clipping plane distance (positive)
 *
 * @param far Far clipping plane distance (positive)
 */
void mat4f_perspective_new(float result[16], float  fovy, float  aspect, float  near, float  far)
{
	near = fabsf(near);
	far = fabsf(far);
	mat4f_identity(result);
	// Our mat*_frustum_new functions check for near=0, near > far, etc.
	if(aspect <= 0)
	{
		msg(MSG_ERROR, "Aspect ratio must be a positive, non-zero number. You set it to %f\n", aspect);
		return;
	}

	if(fovy <= 0 || fovy >=180)
	{
		msg(MSG_ERROR, "Field of view must be between 0 and 180 degrees. You set it to %f\n", fovy);
		return;
	}
	float fovyRad = fovy * ((float)M_PI)/180.0f;
	float height = near * tanf(fovyRad/2.0f);
	float width = height * aspect;
	mat4f_frustum_new(result, -width, width, -height, height, near, far);
}
/** Creates a perspective projection matrix (double). This creates a matrix
 * that is similar to what gluPerspective() would typically apply to the
 * matrix stack earlier versions of OpenGL.
 * Prints a message and returns the identity matrix on error.
 *
 * @param result The resulting 4x4 view frustum projection matrix.
 *
 * @param aspect The aspect ratio of the screen/window/viewport. The
 * aspect ratio is the width of the screen divided by the height of
 * the screen. Larger numbers mean wider screens.
 *
 * @param aspect The aspect ratio of the screen/window/viewport.
 *
 * @param near Near clipping plane distance (positive)
 *
 * @param far Far clipping plane distance (positive)
 */
void mat4d_perspective_new(double result[16], double fovy, double aspect, double near, double far)
{
	near = fabs(near);
	far = fabs(far);
	mat4d_identity(result);
	// Our mat*_frustum_new functions check for near=0, near > far, etc.
	if(aspect <= 0)
	{
		msg(MSG_ERROR, "Aspect ratio must be a positive, non-zero number. You set it to %f\n", aspect);
		return;
	}

	if(fovy <= 0 || fovy >=180)
	{
		msg(MSG_ERROR, "Field of view must be between 0 and 180 degrees. You set it to %f\n", fovy);
		return;
	}

	double fovyRad = fovy * M_PI/180.0;
	double height = near * tan(fovyRad/2.0);
	double width = height * aspect;
	mat4d_frustum_new(result, -width, width, -height, height, near, far);
}

/** Creates a new lookat matrix (aka viewing transformation) which
 * defines the position and orientation of the virtual camera. This
 * creates a matrix that is similar to what gluLookAt() would
 * typically apply to the matrix stack in earlier versions of OpenGL.
 *
 * @param result The resulting view transformation matrix.
 *
 * @param camPos The position of the virtual camera (or eye).
 *
 * @param lookAtPt A point in 3D space that the camera is looking
 * at. (This value is not a vector that the camera facing).
 *
 * @param upVec An up vector. If you don't know what to put here, start
 * with 0,1,0. (The up vector must not be parallel to the view vector
 * calculated as lookAtPt-camPos.)
 */
void mat4f_lookatVec_new(float result[16],
                         const float camPos[3],
                         const float lookAtPt[3],
                         const float upVec[3])
{
	/* Calculate look vector, sanity check */
	float look[3], side[3], newUpVec[3], upVecCopy[3];
	vec3f_sub_new(look, lookAtPt, camPos); // a look vector
	if(vec3f_normSq(look) < .001)
	{
		msg(MSG_ERROR, "Your camera position (%f %f %f) is the same (or nearly the same) as the point that the camera should be looking at (%f %f %f). Setting view matrix to identity.\n",
		    camPos[0],    camPos[1],    camPos[2],
		    lookAtPt[0], lookAtPt[1], lookAtPt[2]);
		mat4f_identity(result);
		return;
	}

	/* Sanity check up vector */
	vec3f_copy(upVecCopy, upVec); // a version of up variable that we can change.
	if(vec3f_normSq(upVecCopy) < .001)
	{
		msg(MSG_ERROR, "Your up vector (%f %f %f) is a zero vector or almost a zero vector. Assuming up vector is 0,1,0.\n", upVecCopy[0], upVecCopy[1], upVecCopy[2]);
		vec3f_set(upVecCopy, 0, 1, 0);
	}

	vec3f_cross_new(side, look, upVecCopy);
	if(vec3f_normSq(side) < .001)
	{
		msg(MSG_ERROR, "Your camera is facing the same direction as your up vector.");
		msg(MSG_INFO, "CamPos:         %5.2f %5.2f %5.2f\n", camPos[0], camPos[1], camPos[2]);
		msg(MSG_INFO, "CamLookAtPoint: %5.2f %5.2f %5.2f\n", lookAtPt[0], lookAtPt[1], lookAtPt[2]);
		msg(MSG_INFO, "CamLookVec:     %5.2f %5.2f %5.2f (calculated from camera position and lookat point)\n", look[0], look[1], look[2]);
		msg(MSG_INFO, "CamUp:          %5.2f %5.2f %5.2f\n", upVecCopy[0], upVecCopy[1], upVecCopy[2]);
		mat4f_identity(result);
		return;
	}
	vec3f_normalize(look);
	vec3f_normalize(side);
	vec3f_cross_new(newUpVec, side, look);

	/* Calculate rotation matrix that will be used to compute final matrix. */
	float rotationPart[16];
	mat4f_identity(rotationPart);
	rotationPart[ 0] = side[0];
	rotationPart[ 4] = side[1];
	rotationPart[ 8] = side[2];
	rotationPart[ 1] = newUpVec[0];
	rotationPart[ 5] = newUpVec[1];
	rotationPart[ 9] = newUpVec[2];
	rotationPart[ 2] = -look[0];
	rotationPart[ 6] = -look[1];
	rotationPart[10] = -look[2];

	/* Calculate translation matrix that will be used to compute final matrix. */
	float negCamPos[3];
	vec3f_scalarMult_new(negCamPos, camPos, -1);
	float translationPart[16];
	mat4f_translateVec_new(translationPart, negCamPos);

	/* Multiply the matrices together */
	mat4f_mult_mat4f_new(result, rotationPart, translationPart);
}
/** Creates a new lookat matrix (aka viewing transformation) which
 * defines the position and orientation of the virtual camera.
 * For full documentation, see mat4f_lookatVec_new()
 */
void mat4d_lookatVec_new(double result[16], const double camPos[3], const double lookAtPt[3], const double upVec[3])
{
	/* Calculate look vector, sanity check */
	double look[3], side[3], newUpVec[3], upVecCopy[3];
	vec3d_sub_new(look, lookAtPt, camPos); // a look vector
	if(vec3d_norm(look) < .001)
	{
		msg(MSG_ERROR, "Your camera position (%f %f %f) is the same (or nearly the same) as the point that the camera should be looking at (%f %f %f). Setting view matrix to identity.\n",
		    camPos[0],    camPos[1],    camPos[2],
		    lookAtPt[0], lookAtPt[1], lookAtPt[2]);
		mat4d_identity(result);
		return;
	}

	/* Sanity check up vector */
	vec3d_copy(upVecCopy, upVec);
	if(vec3d_norm(upVecCopy) < .001)
	{
		msg(MSG_ERROR, "Your up vector (%f %f %f) is a zero vector or almost a zero vector. Assuming up vector is 0,1,0.\n", upVecCopy[0], upVecCopy[1], upVecCopy[2]);
		vec3d_set(upVecCopy, 0, 1, 0);
	}

	vec3d_cross_new(side, look, upVecCopy);
	if(vec3d_normSq(side) < .001)
	{
		msg(MSG_ERROR, "Your camera is facing the same direction as your up vector.");
		msg(MSG_INFO, "CamPos:         %5.2f %5.2f %5.2f\n", camPos[0], camPos[1], camPos[2]);
		msg(MSG_INFO, "CamLookAtPoint: %5.2f %5.2f %5.2f\n", lookAtPt[0], lookAtPt[1], lookAtPt[2]);
		msg(MSG_INFO, "CamLookVec:     %5.2f %5.2f %5.2f (calculated from camera position and lookat point)\n", look[0], look[1], look[2]);
		msg(MSG_INFO, "CamUp:          %5.2f %5.2f %5.2f\n", upVecCopy[0], upVecCopy[1], upVecCopy[2]);
		mat4d_identity(result);
		return;
	}
	vec3d_normalize(look);
	vec3d_normalize(side);
	vec3d_cross_new(newUpVec, side, look);

	/* Calculate rotation matrix that will be used to compute final matrix. */
	double rotationPart[16];
	mat4d_identity(rotationPart);
	rotationPart[ 0] = side[0];
	rotationPart[ 4] = side[1];
	rotationPart[ 8] = side[2];
	rotationPart[ 1] = newUpVec[0];
	rotationPart[ 5] = newUpVec[1];
	rotationPart[ 9] = newUpVec[2];
	rotationPart[ 2] = -look[0];
	rotationPart[ 6] = -look[1];
	rotationPart[10] = -look[2];

	/* Calculate translation matrix that will be used to compute final matrix. */
	double negCamPos[3];
	vec3d_scalarMult_new(negCamPos, camPos, -1);
	double translationPart[16];
	mat4d_translateVec_new(translationPart, negCamPos);

	/* Multiply the matrices together */
	mat4d_mult_mat4d_new(result, rotationPart, translationPart);
}

/** Creates a new lookat matrix (aka viewing transformation) which
 * defines the position and orientation of the virtual camera.
 * For full documentation, see mat4f_lookatVec_new()
 */
void mat4f_lookat_new(float result[16], float camPosX, float camPosY, float camPosZ, float lookAtPtX, float lookAtPtY, float lookAtPtZ, float upVecX, float upVecY, float upVecZ)
{
	float camPos[3], lookAtPt[3], upVec[3];
	vec3f_set(camPos,       camPosX,    camPosY,    camPosZ);
	vec3f_set(lookAtPt, lookAtPtX, lookAtPtY, lookAtPtZ);
	vec3f_set(upVec,         upVecX,     upVecY,     upVecZ);
	mat4f_lookatVec_new(result, camPos, lookAtPt, upVec);
}
/** Creates a new lookat matrix (aka viewing transformation) which
 * defines the position and orientation of the virtual camera.
 * For full documentation, see mat4f_lookatVec_new()
 */
void mat4d_lookat_new(double result[16], double camPosX, double camPosY, double camPosZ, double lookAtPtX, double lookAtPtY, double lookAtPtZ, double upVecX, double upVecY, double upVecZ)
{
	double camPos[3], lookAtPt[3], upVec[3];
	vec3d_set(camPos,       camPosX,    camPosY,    camPosZ);
	vec3d_set(lookAtPt, lookAtPtX, lookAtPtY, lookAtPtZ);
	vec3d_set(upVec,         upVecX,     upVecY,     upVecZ);
	mat4d_lookatVec_new(result, camPos, lookAtPt, upVec);
}


/** Pushes a copy of a matrix currently on top of the stack onto the
    top of the stack. A list structure is used to represent the stack.

    @param l A list structure to store the stack.

    @param m The matrix to multiply against the top matrix on the
    stack and then push the result onto the stack.

    @return If l is NULL, a newly allocated stack that should
    eventually be free()'d with list_free(). Otherwise, the same value
    as l.
 */
void mat4f_stack_push(list *l)
{
	float peek[16];

	if(l->length == 0)
	{
		/* Push an identity matrix if the stack is empty */
		mat4f_identity(peek);
		list_push(l, peek);
	}
	else
	{
		/* Get the top matrix on the stack or use the identity if the
		 * stack is empty */
		list_peek(l, peek);

		if(list_push(l, peek) == 0)
		{
			msg(MSG_FATAL, "Failed to push a matrix onto the stack");
			exit(EXIT_FAILURE);
		}
	}
}

/** Pop a matrix from the top of the stack. Similar to OpenGL 2.0
    glPopMatrix().

    @param l The stack to pop from.
 */
void mat4f_stack_pop(list *l)
{
	if(l == NULL || l->length == 0)
		return;
	list_pop(l, NULL);
}

/** Retrieve a copy of the top matrix from the stack without changing
    the contents of the stack.

    @param l The stack to retrieve the top matrix from.
    
    @param m The location to copy the top matrix into.
 */
void mat4f_stack_peek(const list *l, float m[16])
{
	if(list_peek(l, m) == 0)
		mat4f_identity(m);
}


/** Multiplies the top matrix on the stack with the given matrix. A
    list structure is used to represent the stack. If the stack is
    empty, the matrix will instead be pushed onto the stack.

    @param l The stack that the matrix should be applied to.

    @param m The matrix to be multiplied against the top matrix on the
    stack.
    
    @return If l is NULL, a newly allocated stack that should
    eventually be free()'d with list_free(). Otherwise, the same value
    as l.
 */
void mat4f_stack_mult(list *l, float m[16])
{
	if(l->length == 0)
		list_push(l, m);
	else
	{
		float *top = (float*) list_getptr(l, l->length-1);
		mat4f_mult_mat4f_new(top, top, m);
	}
}
