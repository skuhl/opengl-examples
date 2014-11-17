#include <stdio.h>
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
extern inline void mat3d_from_mat3f(double dest[ 9], const float  src[ 9]);
extern inline void mat4d_from_mat4f(double dest[16], const float  src[16]);
extern inline void mat3f_from_mat3d(float  dest[ 9], const double src[ 9]);
extern inline void mat4f_from_mat4d(float  dest[16], const double src[16]);
