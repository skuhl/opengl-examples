/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */


#include <GL/glew.h>
#include <GL/glx.h> // for setting vsync interval

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h> // for FLT_MAX
#include <libgen.h> // for dirname()
#include <sys/time.h> // gettimeofday()
#include <unistd.h> // usleep()

#ifdef KUHL_UTIL_USE_ASSIMP
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include "kuhl-util.h"
#ifdef KUHL_UTIL_USE_IMAGEMAGICK
#include "imageio.h"
#endif

#define EPSILON 0.0001

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

/* Vector dot products */
extern inline float  vecNf_dot(const float  A[ ], const float  B[ ], const int n);
extern inline double vecNd_dot(const double A[ ], const double B[ ], const int n);
extern inline float  vec3f_dot(const float  A[3], const float  B[3]);
extern inline double vec3d_dot(const double A[3], const double B[3]);
extern inline float  vec4f_dot(const float  A[4], const float  B[4]);
extern inline double vec4d_dot(const double A[4], const double B[4]);

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

/* Convert between 3x3 and 4x4 matrices */
extern inline void mat3f_to_mat3d(double dest[ 9], const float  src[ 9]);
extern inline void mat4f_to_mat4d(double dest[16], const float  src[16]);
extern inline void mat3d_to_mat3f(float  dest[ 9], const double src[ 9]);
extern inline void mat4d_to_mat4f(float  dest[16], const double src[16]);

/** Don't call this function, call kuhl_errorcheck() instead. */
int kuhl_errorcheckFileLine(char *file, int line)
{
	GLenum errCode = glGetError();
	if(errCode != GL_NO_ERROR)
	{
		fprintf(stderr, "!!!!! OpenGL Error !!!!! %s - occurred before %s:%d\n",
		        gluErrorString(errCode), file, line);
		return 1;
	}
	return 0;
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
		printf("mat4f_invert_new(): Failed to invert matrix.\n");
		return 0;
	}

	det = 1.0 / det;

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
		printf("mat4d_invert_new(): Failed to invert matrix.\n");
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
	float inv[9];
	inv[0] = m[4] * m[8] - m[5] * m[7];
	inv[3] = m[6] * m[5] - m[3] * m[8];
	inv[6] = m[3] * m[7] - m[6] * m[4];
	inv[1] = m[7] * m[2] - m[1] * m[8];
	inv[4] = m[0] * m[8] - m[6] * m[2];
	inv[7] = m[1] * m[6] - m[0] * m[7];
	inv[2] = m[1] * m[5] - m[2] * m[4];
	inv[5] = m[2] * m[3] - m[0] * m[5];
	inv[8] = m[0] * m[4] - m[1] * m[3];
	float det = m[0] * (m[4] * m[8] - m[5] * m[7]) -
	            m[3] * (m[1] * m[8] - m[7] * m[2]) +
	            m[6] * (m[1] * m[5] - m[4] * m[2]);
	if (det == 0)
	{
		printf("mat3f_invert_new(): Failed to invert matrix.\n");
		return 0;
	}

	det = 1.0/det;

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
	float inv[9];
	inv[0] = m[4] * m[8] - m[5] * m[7];
	inv[3] = m[6] * m[5] - m[3] * m[8];
	inv[6] = m[3] * m[7] - m[6] * m[4];
	inv[1] = m[7] * m[2] - m[1] * m[8];
	inv[4] = m[0] * m[8] - m[6] * m[2];
	inv[7] = m[1] * m[6] - m[0] * m[7];
	inv[2] = m[1] * m[5] - m[2] * m[4];
	inv[5] = m[2] * m[3] - m[0] * m[5];
	inv[8] = m[0] * m[4] - m[1] * m[3];
	float det = m[0] * (m[4] * m[8] - m[5] * m[7]) -
	            m[3] * (m[1] * m[8] - m[7] * m[2]) +
	            m[6] * (m[1] * m[5] - m[4] * m[2]);
	if (det == 0)
	{
		printf("mat3d_invert_new(): Failed to invert matrix.\n");
		return 0;
	}

	det = 1.0/det;

	for(int i = 0; i < 9; i++)
		out[i] = inv[i] * det;

	return 1;
}
/** Inverts a 4x4 float matrix in place.
 * @param matrix The matrix to be inverted in place.
 */
int mat4f_invert(float  matrix[16])
{ return mat4f_invert_new(matrix, matrix); }
/** Inverts a 4x4 double matrix in place.
 * @param matrix The matrix to be inverted in place.
 */
int mat4d_invert(double matrix[16])
{ return mat4d_invert_new(matrix, matrix); }
/** Inverts a 3x3 float matrix in place.
 * @param matrix The matrix to be inverted in place.
 */
int mat3f_invert(float  matrix[ 9])
{ return mat3f_invert_new(matrix, matrix); }
/** Inverts a 3x3 double matrix in place.
 * @param matrix The matrix to be inverted in place.
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

@param result The location to store the rotation matrix calculated from the Euler angles.
@param a1_degrees The amount of rotation around the first axis in degrees (-180 to 180).

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
			printf("%s: Unknown axis: %c\n", __func__, order[i]);
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
			mat3d_rotateAxis_new(rot, 1, 0, 0, angles[i]);
		else if(order[i] == 'Y' || order[i] == '2')
			mat3d_rotateAxis_new(rot, 0, 1, 0, angles[i]);
		else if(order[i] == 'Z' || order[i] == '3')
			mat3d_rotateAxis_new(rot, 0, 0, 1, angles[i]);
		else
			printf("%s: Unknown axis: %c\n", __func__, order[i]);
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
	mat3f_to_mat4f(result, tmpMat);
}

/** Creates a 4x4 rotation matrix of doubles from intrinsic Euler
    angles. For full documentation, see mat3f_rotateEuler_new().

    @see mat3f_rotateEuler_new()
*/
void mat4d_rotateEuler_new(double result[16], double a1_degrees, double a2_degrees, double a3_degrees, const char order[3])
{
	double tmpMat[9];
	mat3d_rotateEuler_new(tmpMat, a1_degrees, a2_degrees, a3_degrees, order);
	mat3d_to_mat4d(result, tmpMat);
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
void euler_from_mat3f(float angles[3], const float m[9], const char order[3])
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
			printf("%s: Unknown axis: %c\n", __func__, order[i]);
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
//		float index10 = m[mat3_getIndex(index[1],index[0])]; // unused
		float index11 = m[mat3_getIndex(index[1],index[1])];
		float index12 = m[mat3_getIndex(index[1],index[2])];
//		float index20 = m[mat3_getIndex(index[2],index[0])]; // unused
		float index21 = m[mat3_getIndex(index[2],index[1])];
		float index22 = m[mat3_getIndex(index[2],index[2])];

		double sy = sqrtf(index01*index01 + index02*index02);
		angles[0] = atan2f(index01, -sign*index02);
		angles[1] = atan2f(sy, index00);
		float s1=sinf(angles[0]);
		float c1=cosf(angles[0]);
		float c2=cosf(angles[1]);
		angles[2] = atan2f(c1*index12-s1*index22,
		                   c1*index11+s1*c2*index21);
	}
	else // first and last rotations are different axes
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
		angles[i] = angles[i] * 180/M_PI;
}


/** Given a 3x3 rotation matrix and a Euler rotation ordering,
    calculate the Euler angles used to produce the matrix.

 @see euler_from_mat3f()
*/
void euler_from_mat3d(double angles[3], const double m[9], const char order[3])
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
			printf("%s: Unknown axis: %c\n", __func__, order[i]);
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
//		float index10 = m[mat3_getIndex(index[1],index[0])]; // unused
		double index11 = m[mat3_getIndex(index[1],index[1])];
		double index12 = m[mat3_getIndex(index[1],index[2])];
//		double index20 = m[mat3_getIndex(index[2],index[0])]; // unused
		double index21 = m[mat3_getIndex(index[2],index[1])];
		double index22 = m[mat3_getIndex(index[2],index[2])];

		double sy = sqrt(index01*index01 + index02*index02);
		angles[0] = atan2(index01, -sign*index02);
		angles[1] = atan2(sy, index00);
		double s1=sin(angles[0]);
		double c1=cos(angles[0]);
		double c2=cos(angles[1]);
		angles[2] = atan2(c1*index12-s1*index22,
		                   c1*index11+s1*c2*index21);
	}
	else // first and last rotations are different axes
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

 @see euler_from_mat3f()
*/
void euler_from_mat4f(float angles[3], const float m[16], const char order[3])
{
	float tmpMat[9];
	mat4f_to_mat3f(tmpMat, m);
	euler_from_mat3f(angles, tmpMat, order);
}
/** Given a 4x4 rotation matrix and a Euler rotation ordering,
 calculate the Euler angles used to produce the matrix.

 @see euler_from_mat3f()
*/
void euler_from_mat4d(double angles[3], const double m[16], const char order[3])
{
	double tmpMat[9];
	mat4d_to_mat3d(tmpMat, m);
	euler_from_mat3d(angles, tmpMat, order);
}


/** Create a 3x3 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param axis A vector representing the axis to rotate around (must have a non-zero length). */
void mat3f_rotateAxisVec_new(float result[9], float degrees, const float axis[3])
{
	float angle = degrees * M_PI/180;
	float c = cosf(angle);
	float s = sinf(angle);
	float t = 1-c;
	// 1-c is numerically unsound when angle is small.
	// See: https://en.wikipedia.org/wiki/Loss_of_significance
	// Use fix described at:
	// http://math.stackexchange.com/questions/38144
	if(c > .9)
		t = 2.0 * sinf(angle/2.0)*sinf(angle/2.0);

	// If zero vector is passed in, return identity matrix
	float length = vec3f_norm(axis);
	if(length < EPSILON)
	{
		printf("%s: Vector to rotate around was 0!", __func__);
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
	if(length < EPSILON)
	{
		printf("%s: Vector to rotate around was 0!", __func__);
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
	mat3f_to_mat4f(result, tmpMat);
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
	mat3d_to_mat4d(result, tmpMat);
}

/** Create a 3x3 rotation matrix given a rotation axis and the number
 * of degrees to rotate.
 *
 * @param result The location to store the resulting matrix.
 * @param degrees The number of degrees to rotate around the axis.
 * @param x The x-component of the axis to rotate around.
 * @param y The y-component of the axis to rotate around.
 * @param z The z-component of the axis to rotate around.
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
 * @param x The x-component of the axis to rotate around.
 * @param y The y-component of the axis to rotate around.
 * @param z The z-component of the axis to rotate around.
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
 * @param x The x-component of the axis to rotate around.
 * @param y The y-component of the axis to rotate around.
 * @param z The z-component of the axis to rotate around.
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
 * @param x The x-component of the axis to rotate around.
 * @param y The y-component of the axis to rotate around.
 * @param z The z-component of the axis to rotate around.
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
	float s = 2.0 / (quat[X]*quat[X] + quat[Y]*quat[Y] +
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
   matrix[0] = 1.0 - (yy + zz);
   matrix[3] = xy + wz;
   matrix[6] = xz - wy;

   // second row
   matrix[1] = xy - wz;
   matrix[4] = 1.0 - (xx + zz);
   matrix[7] = yz + wx;

   // third row
   matrix[2] = xz + wy;
   matrix[5] = yz - wx;
   matrix[8] = 1.0 - (xx + yy);
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
   matrix[3] = xy + wz;
   matrix[6] = xz - wy;

   // second row
   matrix[1] = xy - wz;
   matrix[4] = 1.0 - (xx + zz);
   matrix[7] = yz + wx;

   // third row
   matrix[2] = xz + wy;
   matrix[5] = yz - wx;
   matrix[8] = 1.0 - (xx + yy);
}
/** Creates a 4x4 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat4f_rotateQuatVec_new(float matrix[16], const float quat[4])
{
	float tmpMat[9];
	mat3f_rotateQuatVec_new(tmpMat, quat);
	mat3f_to_mat4f(matrix, tmpMat);
}
/** Creates a 4x4 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat4d_rotateQuatVec_new(double matrix[16], const double quat[4])
{
	double tmpMat[9];
	mat3d_rotateQuatVec_new(tmpMat, quat);
	mat3d_to_mat4d(matrix, tmpMat);
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
	mat3f_rotateQuatVec_new(matrix, quat);
}
/** Creates a 4x4 rotation matrix from a quaternion (x,y,z,w). For
 * full documentation, see mat4f_rotateQuatVec_new() */
void mat4d_rotateQuat_new(double matrix[16], double x, double y, double z, double w)
{
	double quat[4] = { x,y,z,w };
	mat3d_rotateQuatVec_new(matrix, quat);
}

/** Creates a unit quaternion (x,y,z,w) from a rotation matrix.

    This code is based on Ken Shoemake's SIGGRAPH Tutorial on Quaternions:
    http://www.cs.ucr.edu/~vbz/resources/quatut.pdf
    It is also based code in quat.c on VRPN 2.76 (public domain).

    @param matrix The location to store the output matrix.
   
    @param quat The input quaternion. The quaternion does not need
    to be unit length.
*/
void quatf_from_mat3f(float quat[4], const float matrix[9])
{
	int X=0, Y=1, Z=2, W=3;
	float trace = matrix[0]+matrix[4]+matrix[8]; // sum of diagonal

   if (trace > 0.0)
   {
	   float s = sqrtf(trace + 1.0);
	   quat[W] = s * 0.5;
	   s = 0.5 / s;

	   quat[X] = (matrix[mat3_getIndex(Y,Z)] - matrix[mat3_getIndex(Z,Y)]) * s;
	   quat[Y] = (matrix[mat3_getIndex(Z,X)] - matrix[mat3_getIndex(X,Z)]) * s;
	   quat[Z] = (matrix[mat3_getIndex(X,Y)] - matrix[mat3_getIndex(Y,X)]) * s;
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
	   
	   float s = sqrtf( (matrix[mat3_getIndex(i,i)] - (matrix[mat3_getIndex(j,j)] + matrix[mat3_getIndex(k,k)])) + 1.0 );
	   quat[i] = s * 0.5;
	   
	   s = 0.5 / s;
	   
	   quat[W] = (matrix[mat3_getIndex(j,k)] - matrix[mat3_getIndex(k,j)]) * s;
	   quat[j] = (matrix[mat3_getIndex(i,j)] + matrix[mat3_getIndex(j,i)]) * s;
	   quat[k] = (matrix[mat3_getIndex(i,k)] + matrix[mat3_getIndex(k,i)]) * s;
   }
}
/** Creates a unit quaternion (x,y,z,w) from a rotation matrix. For full documentation, see quatf_from_mat3f() */
void quatd_from_mat3d(double quat[4], const double matrix[9])
{
	int X=0, Y=1, Z=2, W=3;
	double trace = matrix[0]+matrix[4]+matrix[8]; // sum of diagonal

   if (trace > 0.0)
   {
	   double s = sqrtf(trace + 1.0);
	   quat[W] = s * 0.5;
	   s = 0.5 / s;

	   quat[X] = (matrix[mat3_getIndex(Y,Z)] - matrix[mat3_getIndex(Z,Y)]) * s;
	   quat[Y] = (matrix[mat3_getIndex(Z,X)] - matrix[mat3_getIndex(X,Z)]) * s;
	   quat[Z] = (matrix[mat3_getIndex(X,Y)] - matrix[mat3_getIndex(Y,X)]) * s;
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
	   
	   float s = sqrtf( (matrix[mat3_getIndex(i,i)] - (matrix[mat3_getIndex(j,j)] + matrix[mat3_getIndex(k,k)])) + 1.0 );
	   quat[i] = s * 0.5;
	   
	   s = 0.5 / s;
	   
	   quat[W] = (matrix[mat3_getIndex(j,k)] - matrix[mat3_getIndex(k,j)]) * s;
	   quat[j] = (matrix[mat3_getIndex(i,j)] + matrix[mat3_getIndex(j,i)]) * s;
	   quat[k] = (matrix[mat3_getIndex(i,k)] + matrix[mat3_getIndex(k,i)]) * s;
   }
}

/** Creates a unit quaternion (x,y,z,w) from a rotation matrix. For full documentation, see quatf_from_mat3f() */
void quatf_from_mat4f(float quat[4], const float matrix[16])
{
	float tmpMat[9];
	mat4f_to_mat3f(tmpMat, matrix);
	quatf_from_mat3f(quat, tmpMat);
}
/** Creates a unit quaternion (x,y,z,w) from a rotation matrix. For full documentation, see quatf_from_mat3f() */
void quatd_from_mat4d(double quat[4], const double matrix[16])
{
	double tmpMat[9];
	mat4d_to_mat3d(tmpMat, matrix);
	quatd_from_mat3d(quat, tmpMat);
}

/** Creates a quaternion (x,y,z,w) based on an axis and the number of degrees to rotate around that axis. 

    Based code in quat.c on VRPN 2.76 (public domain).

    @param quat The location to store the output quaternion. If the axis is a zero vector, the identity quaternion is returned.
    @param x The x-component of a vector representing the axis to rotate around.
    @param y The y-component of a vector representing the axis to rotate around.
    @param z The z-component of a vector representing the axis to rotate around.
    @param degrees The amount to rotate around the given axis in degrees.
*/
void quatf_rotateAxis_new(float quat[4], float x, float y, float z, float degrees)
{
	int X=0,Y=1,Z=2,W=3;
	// Angle needs to be negated to make it correspond to the behavior of mat3f_rotateAxis_new().
	float angle = -degrees * M_PI/180;

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

	float cosA = cosf(angle / 2.0);
	float sinA = sinf(angle / 2.0);
	quat[W] = cosA;
	quat[X] = sinA * x;
	quat[Y] = sinA * y;
	quat[Z] = sinA * z;
}

/** Creates a quaternion (x,y,z,w) based on an axis and the number of
 * degrees to rotate around that axis. See quatf_rotateAxis_new() for
 * full documentation.
*/
void quatd_rotateAxis_new(double quat[4], double x, double y, double z, double degrees)
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
void quatd_rotateAxisVec_new(double quat[4], double axis[3], double degrees)
{
	quatd_rotateAxis_new(quat, axis[0], axis[1], axis[2], axis[3], degrees);
}
/** Creates a quaternion (x,y,z,w) based on an axis and the number of
 * degrees to rotate around that axis. See quatf_rotateAxis_new() for
 * full documentation.
*/
void quatf_rotateAxisVec_new(float quat[4], float axis[3], double degrees)
{
	quatf_rotateAxis_new(quat, axis[0], axis[1], axis[2], axis[3], degrees);
}

void mat4f_translate_new(float  result[16], float x, float y, float z)
{
	mat4f_identity(result);
	result[12] = x;
	result[13] = y;
	result[14] = z;
	result[15] = 1;
}
void mat4d_translate_new(double result[16], double x, double y, double z)
{
	mat4d_identity(result);
	result[12] = x;
	result[13] = y;
	result[14] = z;
	result[15] = 1;
}
void mat4f_translateVec_new(float  result[16], const float  xyz[3])
{ mat4f_translate_new(result, xyz[0], xyz[1], xyz[2]); }
void mat4d_translateVec_new(double result[16], const double xyz[3])
{ mat4d_translate_new(result, xyz[0], xyz[1], xyz[2]); }

/* Create a scale matrix */
void mat4f_scale_new(float  result[16], float x, float y, float z)
{
	mat4f_identity(result);
	result[mat4_getIndex(0,0)] = x;
	result[mat4_getIndex(1,1)] = y;
	result[mat4_getIndex(2,2)] = z;
}
void mat4d_scale_new(double result[16], double x, double y, double z)
{
	mat4d_identity(result);
	result[mat4_getIndex(0,0)] = x;
	result[mat4_getIndex(1,1)] = y;
	result[mat4_getIndex(2,2)] = z;
}
void mat4f_scaleVec_new(float  result[16], const float  xyz[3])
{ mat4f_scale_new(result, xyz[0], xyz[1], xyz[2]); }
void mat4d_scaleVec_new(double result[16], const double xyz[3])
{ mat4d_scale_new(result, xyz[0], xyz[1], xyz[2]); }
void mat3f_scale_new(float  result[9], float x, float y, float z)
{
	mat3f_identity(result);
	result[mat3_getIndex(0,0)] = x;
	result[mat3_getIndex(1,1)] = y;
	result[mat3_getIndex(2,2)] = z;
}
void mat3d_scale_new(double result[9], double x, double y, double z)
{
	mat3d_identity(result);
	result[mat3_getIndex(0,0)] = x;
	result[mat3_getIndex(1,1)] = y;
	result[mat3_getIndex(2,2)] = z;
}
void mat3f_scaleVec_new(float  result[9], const float  xyz[3])
{ mat3f_scale_new(result, xyz[0], xyz[1], xyz[2]); }
void mat3d_scaleVec_new(double result[9], const double xyz[3])
{ mat3d_scale_new(result, xyz[0], xyz[1], xyz[2]); }


/* Sets dest to the identity and then copies src into the upper-left
 * corner of dest. */
void mat3f_to_mat4f(float  dest[16], const float  src[ 9])
{
	mat4f_identity(dest);
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			dest[mat4_getIndex(i,j)] = src[mat3_getIndex(i,j)];
}
void mat3d_to_mat4d(double dest[16], const double src[ 9])
{
	mat4d_identity(dest);
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			dest[mat4_getIndex(i,j)] = src[mat3_getIndex(i,j)];
}

/* Copies upper-left 3x3 portion of src into dest. */
void mat4f_to_mat3f(float  dest[ 9], const float  src[16])
{
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			dest[mat3_getIndex(i,j)] = src[mat4_getIndex(i,j)];
}
void mat4d_to_mat3d(double dest[ 9], const double src[16])
{
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			dest[mat3_getIndex(i,j)] = src[mat4_getIndex(i,j)];
}

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
	far = fabsf(far);
	mat4f_identity(result);
	if(left == right || bottom == top || near == far || near == 0)
	{
		fprintf(stderr, "kuhl-util: Invalid view frustum matrix.\n");
		return;
	}
	result[0]  =  2.0f * near / (right - left);
    result[5]  =  2.0f * near / (top - bottom);
	result[8]  =  (right + left) / (right - left);
    result[9]  =  (top + bottom) / (top - bottom);
    result[10] = -(far + near) / (far - near);
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
	far = fabs(far);
	mat4d_identity(result);
	if(left == right || bottom == top || near == far || near == 0)
	{
		fprintf(stderr, "kuhl-util: Invalid view frustum matrix.\n");
		return;
	}
	result[0]  =  2.0f * near / (right - left);
    result[5]  =  2.0f * near / (top - bottom);
	result[8]  =  (right + left) / (right - left);
    result[9]  =  (top + bottom) / (top - bottom);
    result[10] = -(far + near) / (far - near);
    result[11] = -1.0f;
    result[14] = -(2.0f * far * near) / (far - near);
    result[15] =  0.0f;
}

/** Creates a orthographic projection matrix (float). This
 * creates a matrix similar to the one that glOrtho() would
 * apply to the OpenGL 2.0 matrix stack. A simpler (but less
 * flexible) alternative to this function is mat4f_perspective_new().
 * Prints a message and returns the identity matrix on error.
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
		fprintf(stderr, "kuhl-util: Invalid orthographic projection matrix.\n");
		return;
	}
	result[0]  =  2 / (right-left);
	result[5]  =  2 / (top-bottom);
	result[10] = -2 / (far-near);
	result[12] = (right+left)/(right-left);
	result[13] = (top+bottom)/(top-bottom);
	result[14] = (far+near)/(far-near);
}

/** Creates a orthographic projection matrix (double). This
 * creates a matrix similar to the one that glOrtho() would
 * apply to the OpenGL 2.0 matrix stack. A simpler (but less
 * flexible) alternative to this function is mat4d_perspective_new().
 * Prints a message and returns the identity matrix on error.
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
		fprintf(stderr, "kuhl-util: Invalid orthographic projection matrix.\n");
		return;
	}
	result[0]  =  2 / (right-left);
	result[5]  =  2 / (top-bottom);
	result[10] = -2 / (far-near);
	result[12] = (right+left)/(right-left);
	result[13] = (top+bottom)/(top-bottom);
	result[14] = (far+near)/(far-near);
}

/** Creates a perspective projection matrix (float). This creates a matrix
 * that is similar to what gluPerspective() would typically apply to the
 * matrix stack earlier versions of OpenGL.
 * Prints a message and returns the identity matrix on error.
 *
 * @param result The resulting 4x4 view frustum projection matrix.
 *
 * @param fovy The field of view in the horizontal direction (degrees)
 * @param aspect The aspect ratio of the screen/window/viewport.
 * @param near Near clipping plane distance (positive)
 * @param far Far clipping plane distance (positive)
 */
void mat4f_perspective_new(float result[16], float  fovy, float  aspect, float  near, float  far)
{
	near = fabs(near);
	far = fabs(far);
	if(near == 0)
	{
		fprintf(stderr, "kuhl-util: Invalid perspective projection matrix.\n");
		return;
	}
	float fovyRad = fovy * M_PI/180.0f;
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
 * @param fovy The field of view in the horizontal direction (degrees)
 * @param aspect The aspect ratio of the screen/window/viewport.
 * @param near Near clipping plane distance (positive)
 * @param far Far clipping plane distance (positive)
 */
void mat4d_perspective_new(double result[16], double fovy, double aspect, double near, double far)
{
	near = abs(near);
	far = abs(far);
	{
		fprintf(stderr, "kuhl-util: Invalid perspective projection matrix.\n");
		mat4d_identity(result);
		return;
	}
	double fovyRad = fovy * M_PI/180.0;
	double height = near * tan(fovyRad/2.0);
	double width = height * aspect;
	mat4d_frustum_new(result, -width, width, -height, height, near, far);
}

void mat4f_lookatVec_new(float  result[16], const float  eye[3], const float  center[3], const float  up[3])
{
	/* Calculate appropriate vectors */
	float look[3];
	vec3f_sub_new(look, center, eye);
	vec3f_normalize(look);
	float side[3];
	vec3f_cross_new(side, look, up);
	vec3f_normalize(side);
	float newUp[3];
	vec3f_cross_new(newUp, side, look);

	/* Calculate rotation matrix that will be used to compute final matrix. */
	float rotationPart[16];
	mat4f_identity(rotationPart);
	rotationPart[ 0] = side[0];
	rotationPart[ 4] = side[1];
	rotationPart[ 8] = side[2];
	rotationPart[ 1] = newUp[0];
	rotationPart[ 5] = newUp[1];
	rotationPart[ 9] = newUp[2];
	rotationPart[ 2] = -look[0];
	rotationPart[ 6] = -look[1];
	rotationPart[10] = -look[2];

	/* Calculate translation matrix that will be used to compute final matrix. */
	float negEye[3];
	vec3f_scalarMult_new(negEye, eye, -1);
	float translationPart[16];
	mat4f_translateVec_new(translationPart, negEye);

	/* Multiply the matrices together */
	mat4f_mult_mat4f_new(result, rotationPart, translationPart);
}
void mat4d_lookatVec_new(double result[16], const double eye[3], const double center[3], const double up[3])
{
	/* Calculate appropriate vectors */
	double look[3];
	vec3d_sub_new(look, center, eye);
	vec3d_normalize(look);
	double side[3];
	vec3d_cross_new(side, look, up);
	vec3d_normalize(side);
	double newUp[3];
	vec3d_cross_new(newUp, side, look);

	/* Calculate rotation matrix that will be used to compute final matrix. */
	double rotationPart[16];
	mat4d_identity(rotationPart);
	rotationPart[ 0] = side[0];
	rotationPart[ 4] = side[1];
	rotationPart[ 8] = side[2];
	rotationPart[ 1] = newUp[0];
	rotationPart[ 5] = newUp[1];
	rotationPart[ 9] = newUp[2];
	rotationPart[ 2] = -look[0];
	rotationPart[ 6] = -look[1];
	rotationPart[10] = -look[2];

	/* Calculate translation matrix that will be used to compute final matrix. */
	double negEye[3];
	vec3d_scalarMult_new(negEye, eye, -1);
	double translationPart[16];
	mat4d_translateVec_new(translationPart, negEye);

	/* Multiply the matrices together */
	mat4d_mult_mat4d_new(result, rotationPart, translationPart);
}
void mat4f_lookat_new(float result[16], float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ)
{
	float eye[3], center[3], up[3];
	vec3f_set(eye,       eyeX,    eyeY,    eyeZ);
	vec3f_set(center, centerX, centerY, centerZ);
	vec3f_set(up,         upX,     upY,     upZ);
	mat4f_lookatVec_new(result, eye, center, up);
}
void mat4d_lookat_new(double result[16], double eyeX, double eyeY, double eyeZ, double centerX, double centerY, double centerZ, double upX, double upY, double upZ)
{
	double eye[3], center[3], up[3];
	vec3d_set(eye,       eyeX,    eyeY,    eyeZ);
	vec3d_set(center, centerX, centerY, centerZ);
	vec3d_set(up,         upX,     upY,     upZ);
	mat4d_lookatVec_new(result, eye, center, up);
}


/** Reads a text file.
 *
 * @param filename The file that we want to read in.
 *
 * @return An array of characters for the file. This array should be
 * free()'d when the caller is finished with it. Exits if an error
 * occurs.
 */
char* kuhl_text_read(const char *filename)
{
	int chunkSize    = 1024;        /* read in chunkSize bytes at a time */
	int contentSpace = chunkSize;   /* space in 'content' array */

	/* We add one more character to create room to store a '\0' at the
	 * end. */
	char *content = (char*) malloc(sizeof(char)*(contentSpace+1));

	/* Pointer to where next chunk should be stored */
	char *contentLoc = content;

	FILE *fp = fopen(filename,"rt");
	int readChars;

	if( fp == NULL)
	{
		fprintf(stderr, "ERROR: Can't open %s\n", filename);
		exit(1);
	}

	do
	{
		readChars = fread(contentLoc, sizeof(char), chunkSize, fp);
		contentLoc[readChars] = '\0';
		contentSpace += chunkSize;
		content = (char*) realloc(content, sizeof(char)*(contentSpace+1));
		contentLoc = content + contentSpace - chunkSize;

	} while( readChars == chunkSize );

	/* We should now be at end of file. If not, there was an error. */
	if(feof(fp) == 0)
	{
		fprintf(stderr, "ERROR: Can't read %s\n", filename);
		exit(1);
	}

	fclose(fp);
	return content;
}


/** Creates a vertex of fragment shader from a file. This function
 * loads, compiles, and checks for errors for the shader.
 *
 * @param filename The file containing a GLSL shader.
 *
 * @param shader_type Either GL_FRAGMENT_SHADER or GL_VERTEX_SHADER
 *
 * @return The ID for the shader. Exits if an error occurs.
 */
GLuint kuhl_create_shader(const char *filename, GLuint shader_type)
{
	if((shader_type != GL_FRAGMENT_SHADER &&
	    shader_type != GL_VERTEX_SHADER ) ||
	   filename == NULL)
	{
		fprintf(stderr, "kuhl_create_shader(): ERROR: You passed inappropriate information into this function.\n");
		return 0;
	}

	/* Make sure that the shader program functions are available via
	 * an extension or because we are using a new enough version of
	 * OpenGL to be guaranteed that the functions exist. */
	if(shader_type == GL_FRAGMENT_SHADER && !glewIsSupported("GL_ARB_fragment_shader") && !glewIsSupported("GL_VERSION_2_0"))
	{
		fprintf(stderr, "kuhl_create_shader(): ERROR: glew said fragment shaders are not supported on this machine.\n");
		exit(1);
	}
	if(shader_type == GL_VERTEX_SHADER && !glewIsSupported("GL_ARB_vertex_shader") && !glewIsSupported("GL_VERSION_2_0"))
	{
		fprintf(stderr, "kuhl_create_shader(): ERROR: glew said vertex shaders are not supported on this machine.\n");
		exit(1);
	}

	/* read in program from the text file */
	// printf("%s shader: %s\n", shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment" , filename);
	GLuint shader = glCreateShader(shader_type);
	kuhl_errorcheck();
	char *text = kuhl_text_read(filename);
	glShaderSource(shader, 1, (const char**) &text, NULL);
	kuhl_errorcheck();
	free(text);

	/* compile program */
	glCompileShader(shader);

	/* Print log from shader compilation (if there is anything in the log) */
	char logString[1024];
	GLsizei actualLen = 0;
	glGetShaderInfoLog(shader, 1024, &actualLen, logString);
	if(actualLen > 0)
		printf("%s Shader log:\n%s\n", shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", logString);
	kuhl_errorcheck();

	/* If shader compilation wasn't successful, exit. */
	GLint shaderCompileStatus = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileStatus);
	if(shaderCompileStatus == GL_FALSE)
		exit(1);

	return shader;
}


/** Prints out useful information about an OpenGL program including a
 * listing of the active attribute variables and active uniform
 * variables.
 *
 * @param program The program that you want information about.
 */
void kuhl_print_program_info(GLuint program)
{
	/* Attributes */
	GLint numVarsInProg = 0;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numVarsInProg);
	printf("Active attributes in program %d: ", program);
	for(int i=0; i<numVarsInProg; i++)
	{
		char buf[1024];
		GLint arraySize = 0;
		GLenum type = 0;
		GLsizei actualLength = 0;

		glGetActiveAttrib(program, i, 1024, &actualLength, &arraySize, &type, buf);
		GLint location = glGetAttribLocation(program, buf);
		printf("%s@%d ", buf, location);
	}
	if(numVarsInProg == 0)
		printf("[none!]\n");
	else
		printf("\n");

	numVarsInProg = 0;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numVarsInProg);
	printf("Active uniforms in program %d: ", program);
	for(int i=0; i<numVarsInProg; i++)
	{
		char buf[1024];
		GLint arraySize = 0;
		GLenum type = 0;
		GLsizei actualLength = 0;

		glGetActiveUniform(program, i, 1024, &actualLength, &arraySize, &type, buf);
		printf("%s@%d ", buf, i);
	}
	if(numVarsInProg == 0)
		printf("[none!]\n");
	else
		printf("\n");

	GLint linkStatus=GL_FALSE, validateStatus=GL_FALSE;
	GLint attachedShaderCount=0;
	GLint binarySize=0;
	GLint deleteStatus=GL_FALSE;
	glGetProgramiv(program, GL_ATTACHED_SHADERS, &attachedShaderCount);
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
	glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binarySize);
	glGetProgramiv(program, GL_DELETE_STATUS, &deleteStatus);
	printf("LinkStatus=%s ValidateStatus=%s AttachedShaderCount=%d Size=%d %s\n",
	       linkStatus     == GL_TRUE ? "OK" : "Fail",
	       validateStatus == GL_TRUE ? "OK" : "Fail",
	       attachedShaderCount, binarySize,
	       deleteStatus   == GL_TRUE ? "DELETED!" : "");

}


/** Creates an OpenGL program from pair of files containing a vertex
 * shader and a fragment shader. This code handles checking for
 * support from the video card, error checking, and setting attribute
 * locations.
 *
 * @param vertexFilename The filename of the vertex program.
 *
 * @param fragFilename The filename of the fragment program.
 *
 * @param attribs A NULL terminated list of attributes in the shader
 * program. For example, if your vertices have in_Position and
 * in_Color attributes, you might set attribs to { "in_Position",
 * "in_Color", NULL } and then in_Position will be in location 0 and
 * in_Color will be in location 1 in the resulting OpenGL program.
 *
 * @return If success, returns the GLuint used to refer to the
 * program. Returns 0 if no shader program was created.
 */
GLuint kuhl_create_program(const char *vertexFilename, const char *fragFilename, char *attribs[])
{
	if(vertexFilename == NULL || fragFilename == NULL || attribs == NULL)
	{
		fprintf(stderr, "kuhl_create_program(): One or more of the parameters were NULL\n");
		return 0;
	}

	/* If your program is crashing here, you should ensure that the
	 * last attribute in attribs is NULL or an empty string. */
	int numAttribs = 0;
	while(attribs[numAttribs] != NULL)
		numAttribs++;

	/* Attributes can't begin with "gl_" */
	for(int i=0; i<numAttribs; i++)
		if(strncmp("gl_", attribs[i], 3) == 0)
			fprintf(stderr, "kuhl_create_program(): WARNING: Your attribute '%s' begins with 'gl_' (OpenGL doesn't allow this).", attribs[i]);

	/* Check for duplicates in the attribute list. */
	for(int i=1; i<numAttribs; i++)
		for(int j=0; j<i; j++)
			if(strcmp(attribs[i], attribs[j]) == 0)
				fprintf(stderr, "kuhl_create_program(): WARNING: Your attribute list contained a duplicate: %s\n", attribs[i]);
	
	/* Make sure we don't have too many attributes. */
	GLint maxAttribs = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
	if(numAttribs > maxAttribs)
		fprintf(stderr, "kuhl_create_program(): WARNING: You provided %d attributes in your list but OpenGL only supports up to %d attributes.\n", numAttribs, maxAttribs);

	/* Create a program to attach our shaders to. */
	GLuint program = glCreateProgram();
	if(program == 0)
	{
		fprintf(stderr, "kuhl_create_program(): ERROR: Failed to create program.\n");
		exit(1);
	}
	printf("Creating program %d from vertex shader (%s) and fragment shader (%s).\n",
	       program, vertexFilename, fragFilename);
	
	/* Create the shaders */
	GLuint fragShader = kuhl_create_shader(fragFilename, GL_FRAGMENT_SHADER);
	GLuint vertexShader = kuhl_create_shader(vertexFilename, GL_VERTEX_SHADER);

	/* Attach shaders, check for errors. */
	glAttachShader(program, fragShader);
	kuhl_errorcheck();
	glAttachShader(program, vertexShader);
	kuhl_errorcheck();

	/* We could let OpenGL assign attribute locations and get those
	   locations with glGetAttribLocation(), but sometimes this isn't
	   a good idea. (For example, see
	   http://stackoverflow.com/questions/4635913 ). Therefore, we
	   bind attributes to locations ourselves! If you bind them in a
	   consistent manner, you can use the location numbers directly
	   inside of display() instead of having to work out the correct
	   location from the name. */
	for(int i=0; i<numAttribs; i++)
	{
		glBindAttribLocation(program, i, attribs[i]);
		kuhl_errorcheck();
	}

	/* Try to link the program. */
	glLinkProgram(program);
	kuhl_errorcheck();

	/* Check if glLinkProgram was successful. */
	GLint linked;
	glGetProgramiv((GLuint)program, GL_LINK_STATUS, &linked);
	kuhl_errorcheck();

	if(linked == GL_FALSE)
	{
		kuhl_print_program_log(program);
		fprintf(stderr, "kuhl_create_program(): ERROR: Failed to link GLSL program.\n");
		exit(1);
	}

	/* Check that each of the attributes the user passed to were
	 * actually in the program. glBindAttribLocation() doesn't provide
	 * any indication of if the attribute exists in the program or
	 * not. We can't do this until we have linked the program. */
	int incorrectAttribCount = 0;
	for(int i=0; i<numAttribs; i++)
	{
		if(glGetAttribLocation(program, attribs[i]) == -1)
		{
			fprintf(stderr, "WARNING: You passed '%s' attribute to kuhl_create_program(), but the attribute didn't exist in the GLSL program.\n", attribs[i]);
			incorrectAttribCount++;
		}
	}

	/* Check that the number of attributes that were successfully
	 * bound matches the number of attributes actually in the
	 * program. */
	GLint numAttribsInProg = 0;
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttribsInProg);
	if(numAttribsInProg != numAttribs - incorrectAttribCount)
		fprintf(stderr, "kuhl_create_program(): NOTE: Some of the attributes in your GLSL program were missing from the list of attributes you passed to kuhl_create_program(). You will need to query the automatically assigned location yourself using glGetAttribLocation().\n");

	glValidateProgram(program);
	kuhl_errorcheck();

	/* Check if program validation was successful. */
	GLint validated;
	glGetProgramiv((GLuint)program, GL_VALIDATE_STATUS, &validated);
	kuhl_errorcheck();

	if(validated == GL_FALSE)
	{
		kuhl_print_program_log(program);
		fprintf(stderr, "kuhl_create_program(): ERROR: Failed to validate GLSL program.\n");
		exit(1);
	}

	kuhl_print_program_info(program);
	printf("GLSL program %d created successfully.\n", program);
	return program;
}

/** Prints a program log if there is one for an OpenGL program.
 *
 * @param program The OpenGL program that we want to print the log for.
 */
void kuhl_print_program_log(GLuint program)
{
	char logString[1024];
	GLsizei actualLen = 0;
	glGetProgramInfoLog(program, 1024, &actualLen, logString);
	if(actualLen > 0)
		printf("GLSL program log:\n%s\n", logString);
}

/** Deletes the given program, and also deletes the shaders
 * which were attached to it.
 *
 * @param program The program to delete.
 */
void kuhl_delete_program(GLuint program)
{
	GLuint shaders[256];
	int count = 0;
	glGetAttachedShaders(program, 256, &count, shaders);
	for(int i=0; i<count; i++)
		glDeleteShader(shaders[i]);
	glDeleteProgram(program);
}


/** glGetUniformLocation() with error checking. This function behaves
 * the same as glGetUniformLocation() except that when an error
 * occurs, it prints an error message if the uniform variable doesn't
 * exist (or is inactive) in the GLSL program. glGetUniformLocation()
 * only returns -1 when the uniform variable is not found.
 *
 * @param program The OpenGL shader program containing the uniform variable.
 *
 * @param uniformName The name of the uniform variable.
 *
 * @return The location of the uniform variable.
 */
GLint kuhl_get_uniform(GLuint program, const char *uniformName)
{
	GLint loc = glGetUniformLocation(program, uniformName);
	if(loc == -1)
		fprintf(stderr, "kuhl_get_uniform(): Uniform variable '%s' is missing or is inactive in your GLSL program.\n", uniformName);
	return loc;
}




/** Converts an array containing RGBA image data into an OpenGL texture.
 *
 * @param array Contains a row-major list of pixels in R, G, B, A format starting from the bottom left corner of the image. Each pixel is a value form 0 to 255.
 *
 * @param width The width of the image represented by the array in pixels.
 *
 * @param height The height of the image represented by the array in pixels.
 *
 * @return The texture name that you can use with glBindTexture() to
 * enable this particular texture when drawing.
 */
GLuint kuhl_read_texture_rgba_array(const char* array, int width, int height)
{
	GLuint texName = 0;
	if(!GLEW_VERSION_2_0)
	{
		/* OpenGL 2.0+ supports non-power-of-2 textures. Also, need to
		 * ensure we have a new enough version for the different
		 * mipmap generation techniques below. */
		printf("ERROR: kuhl_read_texture_rgba_array() requires OpenGL 2.0 to generate mipmaps.\n");
		printf("Either your video card/driver doesn't support OpenGL 2.0 or better OR you forgot to call glewInit() at the appropriate time at the beginning of your program.\n");
		return 0;
	}

	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	/* If anisotropic filtering is available, turn it on.  This does not
	 * override the MIN_FILTER. The MIN_FILTER setting may affect how the
	 * videocard decides to do anisotropic filtering, however.  For more info:
	 * http://www.opengl.org/registry/specs/EXT/texture_filter_anisotropic.txt
	 *
	 * Note that anisotropic filtering may not be available if you ask
	 * for an OpenGL core profile. For more information, see:
	 * http://gamedev.stackexchange.com/questions/70829
	 */
	if(glewIsSupported("GL_EXT_texture_filter_anisotropic"))
	{
		float maxAniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
		printf("Anisotropic filtering: Available, set to maximum value (%0.1f)\n",
		       maxAniso);
	}

	kuhl_errorcheck();
	
	/* Try to see if OpenGL will accept this texture.  If the dimensions of
	 * the file are too big, OpenGL might not load it. NOTE: The parameters
	 * here should match the parameters of the actual (non-proxy) calls to
	 * glTexImage2D() below. */
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA8, width, height,
	             0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	int tmp;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tmp);
	if(tmp == 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		return 0;
	}

	/* The recommended way to produce mipmaps depends on your OpenGL
	 * version. */
	if (glGenerateMipmap != NULL)
	{
		/* In OpenGL 3.0 or newer, it is recommended that you use
		 * glGenerateMipmaps().  Older versions of OpenGL that provided the
		 * same capability as an extension, called it
		 * glGenerateMipmapsEXT(). */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
		             0, GL_RGBA, GL_UNSIGNED_BYTE, array);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else // if(glewIsSupported("GL_SGIS_generate_mipmap"))
	{
		/* Should be used for 1.4 <= OpenGL version < 3.   */
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
		             0, GL_RGBA, GL_UNSIGNED_BYTE, array);
	}

	/* render textures perspectively correct---instead of interpolating
	   textures in screen-space. */
	kuhl_errorcheck();

	/* The following two lines of code are only useful for OpenGL 1 or
	 * 2 programs. They may cause an error message when called in a
	 * newer version of OpenGL. */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // only use texture, no other lighting applied!
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glGetError(); // discard any error messages.

	// Unbind the texture, make the caller bind it when they want to use it. More details:
	// http://stackoverflow.com/questions/15273674
	glBindTexture(GL_TEXTURE_2D, 0);
	return texName;
}


#ifdef KUHL_UTIL_USE_IMAGEMAGICK

/** Creates a texture from a string of text. For example, if you want
 * a texture that says "hello world" in red on a transparent
 * background, this method can easily create that texture directly
 * using ImageMagick. The text will be written in a normal font and
 * will be one line of text. The preprocessor variable
 * KUHL_UTIL_USE_IMAGEMAGICK must be defined to use this function.
 *
 * @param label The text that you want to render.
 *
 * @param texName A pointer that will be filled with the OpenGL texture name for the new texture.
 *
 * @param color The color of the text.
 *
 * @param bgcolor The background color for the texture (can be transparent).
 *
 * @param pointsize The size of the text in points. Use a larger value for higher resolution text.
 *
 * @return The aspect ratio of the texture. If an error occurs, the function prints a message and exits. */
float kuhl_make_label(const char *label, GLuint *texName, float color[3], float bgcolor[4], float pointsize)
{
	int width = 0;
	int height = 0;
	char *image = image_label(label, &width, &height, color, bgcolor, 10);
//	printf("Label texture dimensions: %d %d\n", width, height);
	*texName = kuhl_read_texture_rgba_array(image, width, height);
	free(image);
	
	if(*texName == 0)
	{
		fprintf(stderr, "Failed to create label: %s\n", label);
		exit(EXIT_FAILURE);
	}
	return width/(float)height;
}


/** Uses imageio to read in an image, and binds it to an OpenGL
 * texture name.  Requires OpenGL 2.0 or better. The preprocessor variable
 * KUHL_UTIL_USE_IMAGEMAGICK must be defined to use this function.
 *
 * @param filename name of file to load
 *
 * @param texName A pointer to where the OpenGL texture name should be stored.
 * (Remember that the "texture name" is really just some unsigned int).
 * @returns The aspect ratio of the image in the file. Since texture
 * coordinates range from 0 to 1, the caller doesn't really need to
 * know how large the image actually is.
 */
float kuhl_read_texture_file(const char *filename, GLuint *texName)
{
    /* It is generally best to just load images in RGBA8 format even
     * if we don't need the alpha component. ImageMagick will fill the
     * alpha component in correctly (opaque if there is no alpha
     * component in the file or with the actual alpha data. For more
     * information about why we use RGBA by default, see:
     * http://www.opengl.org/wiki/Common_Mistakes#Image_precision
     */
	imageio_info iioinfo;
	iioinfo.filename   = filename;
	iioinfo.type       = CharPixel;
	iioinfo.map        = (char*) "RGBA";
	iioinfo.colorspace = sRGBColorspace;
	char *image = (char*) imagein(&iioinfo);
	if(image == NULL)
	{
		fprintf(stderr, "\n%s: Unable to read image.\n", filename);
		return -1;
	}

	/* "image" is a 1D array of characters (unsigned bytes) with four
	 * bytes for each pixel (red, green, blue, alpha). The data in "image"
	 * is in row major order. The first 4 bytes are the color information
	 * for the lowest left pixel in the texture. */
	int width  = (int)iioinfo.width;
	int height = (int)iioinfo.height;
	float aspectRatio = (float)width/height;
    printf("%s: Finished reading, dimensions are %dx%d\n", filename, width, height);
	*texName = kuhl_read_texture_rgba_array(image, width, height);

	if(iioinfo.comment)
		free(iioinfo.comment);
	free(image);
	
	if(*texName == 0)
	{
		fprintf(stderr, "%s: Failed to read image.\n", filename);
		exit(EXIT_FAILURE);
	}

	return aspectRatio;
}
#endif


#ifdef KUHL_UTIL_USE_ASSIMP

/** This struct is used internally by kuhl_util.c to keep track of all textures that are associated with models that have been loaded. */
typedef struct {
	char *textureFileName; /**< The filename of a texture */
	GLuint textureID;      /**< The OpenGL texture name for that texture */
} textureIdMapStruct;
#define textureIdMapMaxSize 1024*32 /**< Maximum number of textures that can be loaded from models */
static textureIdMapStruct textureIdMap[textureIdMapMaxSize]; /**<List of textures for the models */
static int textureIdMapSize = 0; /**< Number of items in textureIdMap */

/** This struct is used internally by kuhl_util.c to keep track of all
 * of the models that we have loaded. */
typedef struct {
	char *modelFilename; /**< The filename of the loaded model */
	const struct aiScene *scene; /**< The scene information for the model */
	float bb_min[3]; /**< Smallest X,Y,Z coordinates out of all vertices */
	float bb_max[3]; /**< Largest X,Y,Z coordinates out of all vertices */
	float bb_center[3]; /**< Average of smallest and largest vertex coordinates */
} sceneMapStruct;
#define sceneMapMaxSize 1024 /**< Maximum number of scenes in sceneMap */
static sceneMapStruct sceneMap[sceneMapMaxSize]; /**< A list of scenes */
static int sceneMapSize = 0; /**< Number of items in the sceneMap list */


/** Looks for a model in the sceneMap list based on its filename.
 *
 * @param modelFilename The filename for the model.
 *
 * @return The index of the model in the sceneMap list or -1 if the
 * model does not exist in the sceneMap.
 */
static int kuhl_private_modelIndex(const char *modelFilename)
{
	for(int i=0; i<sceneMapSize; i++)
	{
		if(strcmp(modelFilename, sceneMap[i].modelFilename) == 0)
			return i;
	}
	return -1;
}



/** Recursively traverse a tree of ASSIMP nodes and updates the
 * bounding box information stored in our sceneMap list for that
 * model.
 *
 * @param nd A pointer to an ASSIMP aiNode struct.
 *
 * @param transform A pointer to an ASSIMP 4x4 transform matrix. When this function is called for the first time, set transform to NULL.
 *
 * @param scene An ASSIMP scene struct.
 *
 * @param modelIndex The index of the model in our sceneMap list.
 */
static void kuhl_private_calc_bbox(const struct aiNode* nd, struct aiMatrix4x4* transform, const struct aiScene *scene, const int modelIndex)
{
	// Get shorter names for our current min/max/center vectors.
	float *min = sceneMap[modelIndex].bb_min;
	float *max = sceneMap[modelIndex].bb_max;
	float *ctr = sceneMap[modelIndex].bb_center;
	
	/* When this method is called on the root node, the trafo matrix should be set to NULL. */
	if(transform == NULL)
	{
		// Reset our bounding box variables
		vec3f_set(min,  FLT_MAX,  FLT_MAX,  FLT_MAX);
		vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);
		vec3f_set(ctr, 0,0,0);

		// Set transform matrix to identity
		struct aiMatrix4x4 ident;
		aiIdentityMatrix4(&ident); 
		transform = &ident;
	}

	// Save the transformation before we process this node.
	struct aiMatrix4x4 previousTransform = *transform;
	// Apply this nodes transformation matrix
	aiMultiplyMatrix4(transform, &nd->mTransformation);

	/* For each mesh */
	for (unsigned int n=0; n < nd->mNumMeshes; ++n)
	{
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		/* For each vertex in mesh */
		for (unsigned int t=0; t < mesh->mNumVertices; t++)
		{
			// Transform the vertex based on the transformation matrix
			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp, transform);

			// Update our bounding box
			float coord[3];
			vec3f_set(coord, tmp.x, tmp.y, tmp.z);
			for(int i=0; i<3; i++)
			{
				if(coord[i] > max[i]) // found new max
					max[i] = coord[i];
				if(coord[i] < min[i]) // found new min
					min[i] = coord[i];
			}
			// Calculate new box center
			vec3f_add_new(ctr, min, max);
			vec3f_scalarDiv(ctr, 2);
		}
	}
	
	/* Process the children nodes using the current transformation. */
	for (unsigned int n=0; n < nd->mNumChildren; n++)
		kuhl_private_calc_bbox(nd->mChildren[n], transform, scene, modelIndex);

	/* Since we are done processing this node, we need to restore the
	* transformation matrix to whatever it was before we started
	* working on this node. */
	*transform = previousTransform;
}



/** Loads a model (if needed) and returns its index in the sceneMap.
 *
 * @param modelFilename The filename of a model to load.
 *
 * @param textureDirname The directory the textures for the model are
 * stored in. If textureDirname is NULL, we assume that the textures
 * are in the same directory as the model file.
 * 
 * @return Returns the index in the sceneMap array of this model.
 */
static int kuhl_private_load_model(const char *modelFilename, const char *textureDirname)
{
	int index = kuhl_private_modelIndex(modelFilename);
	if(index >= 0)
		return index;

	/* If we get here, we need to add the file to the sceneMap. */

	/* Write assimp messages to command line */
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);
	
	/* Try loading the model. We are using a postprocessing preset
	 * here so we don't have to set many options. */
	char *modelFilenameVarying = strdup(modelFilename); // aiImportFile doesn't declare filaname parameter as const!
	const struct aiScene* scene = aiImportFile(modelFilenameVarying, aiProcessPreset_TargetRealtime_MaxQuality);
	free(modelFilenameVarying);
	if(scene == NULL)
	{
		printf("%s: ASSIMP was unable to import the model file.\n", modelFilename);
		return -1;
	}

	/* Print warning message if model file contains embedded textures or animations since we don't support that (even if ASSIMP might). */
	if(scene->mNumTextures > 0)
		printf("%s: WARNING: This model has %d textures embedded in it. This program currently ignores embedded textures.\n", modelFilename, scene->mNumTextures);

	if(scene->mNumAnimations > 0)
		printf("%s: WARNING: This model has %d animations embedded in it. This program currently ignores embedded animations.\n", modelFilename, scene->mNumAnimations);

	/* For safety, zero out our texture ID map if it is supposed to be empty right now. */
	if(textureIdMapSize == 0)
	{
		for(int i=0; i<textureIdMapMaxSize; i++)
		{
			textureIdMap[i].textureFileName = NULL;
			textureIdMap[i].textureID = 0;
		}
	}

	/* For safety, zero our our sceneMap if it is supposed to be empty right now. */
	if(sceneMapSize == 0)
	{
		for(int i=0; i<sceneMapMaxSize; i++)
		{
			sceneMap[i].modelFilename = NULL;
			sceneMap[i].scene = NULL;
		}
	}

	/* For each material that has a texture in the scene, try to load the corresponding texture file. */
	for(unsigned int m=0; m < scene->mNumMaterials; m++)
	{
		struct aiString path;
		GLuint texIndex = 0;

		if(aiGetMaterialTexture(scene->mMaterials[m], aiTextureType_DIFFUSE, texIndex, &path, NULL, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
		{
			char fullpath[1024];
			if(textureDirname == NULL)
			{
				// Copy the directory that the model file is in into the fullpath variable
				char *filenameCopy = strdup(modelFilename);
				strncpy(fullpath, dirname(filenameCopy), 1024);
				free(filenameCopy);
			}
			else
				// Copy the user-specified texture directory into fullpath variable
				strncpy(fullpath, textureDirname, 1024);

			strncat(fullpath, "/", 1024-strlen(fullpath)); // make sure there is a slash between the directory and the texture's filename
			strncat(fullpath, path.data, 1024-strlen(fullpath));

			printf("%s: Model refers to a texture: %s\n", modelFilename, path.data);
			printf("%s: Looking for texture file: %s\n", modelFilename, fullpath);
			kuhl_read_texture_file(fullpath, &texIndex);

			/* Store the texture information in our list structure so
			 * we can find the textureID from the filename when we
			 * render the scene. */
			if(textureIdMapSize >= textureIdMapMaxSize)
			{
				printf("You have loaded more textures than the hardcoded limit. Exiting.\n");
				exit(EXIT_FAILURE);
			}
			textureIdMap[textureIdMapSize].textureFileName = strdup(path.data);
			textureIdMap[textureIdMapSize].textureID = texIndex;
			textureIdMapSize++;
		}
	}


	/* Store the scene information in our list structure so we can
	 * find the scene from the model filename again in the future. */
	if(sceneMapSize >= sceneMapMaxSize)
	{
		printf("You have loaded more scenes than the hardcoded limit. Exiting.\n");
		exit(EXIT_FAILURE);
	}

	index = sceneMapSize;
	sceneMapSize++;
	
	sceneMap[index].modelFilename = strdup(modelFilename);
	sceneMap[index].scene = scene;
	kuhl_private_calc_bbox(scene->mRootNode, NULL, scene, index);

	printf("%s: Bounding box min: ", modelFilename);
	vec3f_print(sceneMap[index].bb_min);
	printf("%s: Bounding box max: ", modelFilename);
	vec3f_print(sceneMap[index].bb_max);
	printf("%s: Bounding box ctr: ", modelFilename);
	vec3f_print(sceneMap[index].bb_center);

	return index;
}


/** Convert a aiColor4d struct into a plain 4 element array. Used by
 * kuhl_private_material_ogl2()
 *
 * @param c An aiColor4D struct used by ASSIMP to represent a color.
 * @param f A 4 element array to copy the color data into.
 */
static void kuhl_private_c4_to_f4(const struct aiColor4D *c, float f[4])
{	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}


/** Given an ASSIMP material, set up OpenGL 1/2 rendering settings so
 * that we can draw polygons with that material.
 *
 * @param mtl The material we want to render.
 */
static void kuhl_private_material_ogl2(const struct aiMaterial *mtl)
{
	struct aiString texPath;	//contains filename of texture
	int texIndex = 0;
	if(AI_SUCCESS == aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE, texIndex, &texPath,
	                                      NULL, NULL, NULL, NULL, NULL, NULL))
	{
		glEnable(GL_TEXTURE_2D);
		//bind texture
		for(int i=0; i<textureIdMapSize; i++)
			if(strcmp(textureIdMap[i].textureFileName, texPath.data) == 0)
				glBindTexture(GL_TEXTURE_2D, textureIdMap[i].textureID);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	float c[4];
	struct aiColor4D diffuse, specular, ambient, emission;
	vec4f_set(c, 0.8f, 0.8f, 0.8f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
		kuhl_private_c4_to_f4(&diffuse, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

	// specular
	vec4f_set(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
		kuhl_private_c4_to_f4(&specular, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

	// ambient
	vec4f_set(c, 0.2f, 0.2f, 0.2f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
		kuhl_private_c4_to_f4(&ambient, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

	// emission
	vec4f_set(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
		kuhl_private_c4_to_f4(&emission, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

	unsigned int max = 1;
	float shininess, strength;
	int ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
	if(ret1 == AI_SUCCESS) {
    	max = 1;
    	int ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
		if(ret2 == AI_SUCCESS)
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
        else
        	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
	else {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		vec4f_set(c, 0.0f, 0.0f, 0.0f, 0.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	/* Default to filling triangles, use wireframe if requested. */
	GLenum fill_mode = GL_FILL;
	int wireframe = GL_FILL;
	max = 1;
	if(aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME,
	                             &wireframe, &max) == AI_SUCCESS)
	   fill_mode = wireframe ? GL_LINE : GL_FILL;
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

	/* Default to culling faces. Draw both front and back faces if requested. */
	max = 1;
	glEnable(GL_CULL_FACE);
	int two_sided = 0;
	if(aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED,
	                             &two_sided, &max) == AI_SUCCESS)
		if(two_sided)
			glDisable(GL_CULL_FACE);
}

/** Recursively render the scene and apply materials appropriately using OpenGL 1 and 2 calls.
 *
 * @param sc The scene that we want to render.
 *
 * @param nd The current node that we are rendering.
 */
static void kuhl_private_recrend_ogl2(const struct aiScene *sc, const struct aiNode* nd)
{
	struct aiMatrix4x4 m = nd->mTransformation;

	// update transform
	aiTransposeMatrix4(&m);
	glPushMatrix();
	glMultMatrixf((float*)&m);

	// draw all meshes assigned to this node
	for(unsigned int n=0; n < nd->mNumMeshes; n++) {
		const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
		// Set up the material
		kuhl_private_material_ogl2(sc->mMaterials[mesh->mMaterialIndex]);

		/* Don't use lighting if no normals are provided */
		if(mesh->mNormals == NULL)
			glDisable(GL_LIGHTING);
		else
			glEnable(GL_LIGHTING);

		/* Colors are specified, use them */
		if(mesh->mColors == NULL || mesh->mColors[0] == NULL)
			glDisable(GL_COLOR_MATERIAL);
		else
			glEnable(GL_COLOR_MATERIAL);

		for(unsigned int t = 0; t < mesh->mNumFaces; t++)
		{
			const struct aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch(face->mNumIndices)
			{
				case 1: face_mode = GL_POINTS; break;
				case 2: face_mode = GL_LINES; break;
				case 3: face_mode = GL_TRIANGLES; break;
				default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode); /* begin drawing */
			for(unsigned int i=0; i < face->mNumIndices; i++)
			{
				int index = face->mIndices[i];
				/* Set color of vertex */
				if(mesh->mColors != NULL && mesh->mColors[0] != NULL)
					glColor4fv((GLfloat*)&mesh->mColors[0][index]);
				/* Set texture coordinate of vertex */
				if(mesh->mTextureCoords != NULL && mesh->mTextureCoords[0] != NULL)
				{
					glTexCoord2f(mesh->mTextureCoords[0][index].x,
					             mesh->mTextureCoords[0][index].y);  
				}
				/* Set the normal at this vertex */
				if(mesh->mNormals != NULL)
					glNormal3fv(&mesh->mNormals[index].x);
				/* Draw the vertex */
				glVertex3fv(&mesh->mVertices[index].x);
			}
			glEnd(); /* Finish drawing */
		}
	}

	// Draw all children nodes too.
	for (unsigned int i = 0; i < nd->mNumChildren; i++)
		kuhl_private_recrend_ogl2(sc, nd->mChildren[i]);

	glPopMatrix();
}

/** Given a model file, load the model (if it hasn't been loaded
 * already) and render that file using OpenGL2. The preprocessor
 * variable KUHL_UTIL_USE_ASSIMP must be defined to use this function.
 *
 * @param modelFilename The filename of the model.
 *
 * @param textureDirname The directory that the model's textures are saved in. If set to NULL, the textures are assumed to be in the same directory as the model is in.
 *
 * @return Returns 1 if successful and 0 if we failed to load the model.
 */
int kuhl_draw_model_file_ogl2(const char *modelFilename, const char *textureDirname)
{
	// Load the model if necessary and get its index in our sceneMap.
	int index = kuhl_private_load_model(modelFilename, textureDirname);
	if(index >= 0)
	{
		/* Save and restore OpenGL state so that any state that we set
		 * doesn't bleed over into other things that the caller draws
		 * later. */
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		// Draw the scene
		kuhl_private_recrend_ogl2(sceneMap[index].scene, sceneMap[index].scene->mRootNode);
		glPopAttrib();
		return 1;
	}
	return 0;
	
	/* TODO: Think about proving a way for a user to cleanup models
	   appropriately. We would call these two functions: */
	// aiReleaseImport(scene);
	// aiDetachAllLogStreams();
}

/** Returns the bounding box for a model file.
 *
 * @param modelFilename The 3D model file that you want the bounding box for.
 *
 * @param min An array to be filled with the smallest X,Y,Z values in the model.
 *
 * @param max An array to be filled with the largest X,Y,Z values in the model.
 *
 * @param center An array to be filled with the center coordinate of the bounding box.
 *
 * @return Returns 1 if successful or 0 if the model hasn't yet been loaded or drawn.
 */
int kuhl_model_bounding_box(const char *modelFilename, float min[3], float max[3], float center[3])
{
	int index = kuhl_private_modelIndex(modelFilename);
	if(index < 0)
		return 0;

	vec3f_copy(min,    sceneMap[index].bb_min);
	vec3f_copy(max,    sceneMap[index].bb_max);
	vec3f_copy(center, sceneMap[index].bb_center);
	return 1;
}
#endif // KUHL_UTIL_USE_ASSIMP


/** The time at which kuhl_limitfps() was last called. */
static struct timeval limitfps_last = { .tv_sec = 0, .tv_usec = 0 };
/** When called per frame, sleeps for a short period of time to limit
 * the frames per second. There are two potential uses for this: (1)
 * When FPS are far higher than the monitor refresh rate and CPU load
 * are high, this can reduce both of them to a more reasonable
 * value. (2) Allows you to test to see how your program might run if
 * it were running on hardware with a lower frame rate.
 *
 * kuhl_limitfps() does not reduce tearing. Tearing can eliminated on
 * one monitor connected to a machine via various options in drivers
 * or with special calls to glXSwapIntervalEXT() (not implemented here
 * because I've had difficulty reliably getting it to work/compile),
 * setting options in your video card driver, or setting an
 * environment variable (on Linux machines with NVIDIA cards):
 * http://us.download.nvidia.com/XFree86/Linux-x86/180.22/README/chapter-11.html
 *
 * @param fps Requested frames per second that we should not exceed.
 *
 * @see kuhl_getfps()
 */
void kuhl_limitfps(int fps)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	// How many microseconds have elapsed since last called? */
	suseconds_t elapsed_micro = (tv.tv_sec - limitfps_last.tv_sec)*1000000L + (tv.tv_usec - limitfps_last.tv_usec);
	// printf("elapsed_micro %ld\n", elapsed_micro);
	
	// How many microseconds should elapse per frame?
	float microspf = 1.0/fps * 1000000;
	// printf("microsec per frame %f\n", microspf);
	if(microspf > elapsed_micro)
	{
		suseconds_t microsec_sleep = (suseconds_t)microspf - elapsed_micro;
		// printf("sleeping %ld\n", microsec_sleep);
		usleep(microsec_sleep);
	}

	limitfps_last = tv;
}


static int fps_frame=0; /**< Number of frames in this second. Used by kuhl_getfps() */
static int fps_timebase=-1; /**< Time since we last updated FPS estimate. Used by kuhl_getfps() */
static float fps_now=-1; /**< Current estimate of FPS? Used by kuhl_getfps() */

/** When called every frame, estimates the frames per second.
 *
 * @param milliseconds The time in milliseconds relative to some fixed value. For example, if you are using GLUT, you can use glutGet(GLUT_ELAPSED_TIME) to get the time in milliseconds since your program started.
 *
 * @return An estimate of the frames per second (updated every second).
 *
 * @see kuhl_limitfps()
 */
float kuhl_getfps(int milliseconds)
{
	fps_frame++;

	/* If it is the first time we're called, keep track of the current
	 * time so we can calculate FPS once a second has elapsed. */
	if(fps_timebase == -1)
		fps_timebase = milliseconds;
	
	// If a second has elapsed since our last estimation
	if(milliseconds - fps_timebase > 1000)
	{
		// Calculate frames per second
		fps_now = fps_frame*1000.0/(milliseconds-fps_timebase);
		// Update the time that our estimation occured
		fps_timebase = milliseconds;
		// Reset our frame counter.
		fps_frame = 0;
	}
	return fps_now;
}
