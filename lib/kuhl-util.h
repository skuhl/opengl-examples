/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 *
 * This file provides miscellaneous matrix and vector math operations
 * as well as helper functions for loading vertex/fragment shaders,
 * textures via imageio.h (if KUHL_UTIL_USE_IMAGEMAGICK preprocessor
 * variable is defined) and 3D models (if KUHL_UTIL_USE_ASSIMP
 * preprocessor variable is defined).
 *
 * See imageio.c and imageio.h for more information about texture
 * loading capabilities.
 *
 * ASSIMP is published under 3-clause BSD license (as is this
 * file). Some of the code involving ASSIMP in this file is based on
 * code in an example program in the ASSIMP source tree. ASSIMP code
 * is Copyright (c) 2006-2012 assimp team. All rights reserved.
 * For more information about ASSIMP, see: http://assimp.sourceforge.net/
 */

#pragma once

#include "windows-compat.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>


#if 0
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif


/* Windows does not have some functions, so we need to use the equivalent functions on Windows instead. */
#ifdef _MSC_VER 
 //not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define sleep(t)  Sleep((t) * 1000)
#endif

#include "kuhl-config.h"
#include "kuhl-nodep.h"
#include "msg.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef M_PI
/** M_PI is an approximation of pi. This variable is often available
 * in C without defining it yourself, but it isn't guaranteed to exist
 * according to the C standard. For more information, see:
 * http://c-faq.com/fp/mpi.html */
#define M_PI 3.14159265358979323846
#endif

/** Maximum number of bones that can be sent to GLSL program */
#define MAX_BONES 128
#define MAX_ATTRIBUTES 16
#define MAX_TEXTURES 8
	
#if KUHL_UTIL_USE_ASSIMP
typedef struct
{
	int count; /**< Number of bones in this struct */
	unsigned int mesh; /**< The bones in this struct are associated with this matrix index */
	const struct aiBone *boneList[MAX_BONES];
	float matrices[MAX_BONES][16]; /**< Transformation matrices for each bone */
} kuhl_bonemat;
#endif

/** This enum is used by some kuhl_geometry related functions */
enum
{ /* Options used for some kuhl_geometry functions */
	KG_NONE = 0,     /**< No options */
	KG_WARN = 1,     /**< Warn if GLSL variable is missing */
	KG_FULL_LIST = 2 /**< Apply to entire list of kuhl_geometry objects */
};

/** There is an array of kuhl_attrib structs inside of
 * kuhl_geometry to store all vertex attribute information */
typedef struct
{
	char*    name; /**< GLSL variable name the attribute information should be linked with. */
	GLuint   bufferobject; /**< OpenGL buffer the attribute is stored in */
} kuhl_attrib;

/** There is an array of kuhl_texture structs inside of
 * kuhl_geometry. */
typedef struct
{
	char* name; /**< GLSL variable name the texture should be linked with. */
	GLuint textureId; /**< OpenGL texture id/name of the texture */
} kuhl_texture;
	
/** The kuhl_geometry struct is used to quickly draw 3D objects in
 * OpenGL 3.0. For more information, see the example programs and the
 * documentation for kuhl_geometry_new() and kuhl_geometry_draw(). The
 * functions provide error checking and uses reasonable defaults. They
 * also provide significantly less flexibility than what OpenGL
 * provides. */
typedef struct _kuhl_geometry_
{
	GLuint vao;  /**< OpenGL Vertex Array Object - created by kuhl_geometry_new() */
	GLuint program; /**< OpenGL program object to use with this geometry - filled in by kuhl_geometry_new(). */
	GLuint vertex_count; /**< How many vertices are in this geometry? - Filled in by kuhl_geometry_new(). */
	GLenum primitive_type; /**< GL_TRIANGLES, GL_POINTS, etc. - Filled in by kuhl_geometry_new() */

	kuhl_attrib attribs[MAX_ATTRIBUTES]; /**< A list of attributes, to add or modify an attribute, use kuhl_geometry_attrib(). */
	unsigned int attrib_count; /**< Number of attributes in this geometry */

	kuhl_texture textures[MAX_TEXTURES];
	unsigned int texture_count;

	GLuint indices_len; /**< How many indices are there? - User should set this. */
	GLuint indices_bufferobject; /**< What is the OpenGL buffer object that holds the indices? - Set by kuhl_geometry_init(). */

	float matrix[16]; /**< A matrix that all of this geometry should be transformed by */
	int has_been_drawn; /**< Has this piece of geometry been drawn yet? */
	
#if KUHL_UTIL_USE_ASSIMP
	struct aiNode *assimp_node; /**< Assimp node that this kuhl_geometry object was created from. */
	struct aiScene *assimp_scene; /**< Assimp scene that this kuhl_geometry object is a part of. */
	kuhl_bonemat *bones; /**< Information about bones in the model */
#endif

	struct _kuhl_geometry_ *next; /**< A kuhl_geometry object can be a linked list. */
	
} kuhl_geometry;


/** Call kuhl_errorcheck() with no parameters frequently for easy
 * OpenGL error checking. OpenGL doesn't report errors by
 * default. Instead, we must periodically check for errors
 * ourselves. When you call kuhl_errorcheck() with no parameters. The
 * C preprocessor will replace those calls with
 * kuhl_errorcheckFileLine() and pass the source code filename and the
 * line in the source code file as parameters to it.
 *
 * One alternative way to carefully check for errors is to set up a
 * OpenGL context with debugging enabled and then use
 * glDebugMessageCallback() to ask OpenGL to call a function that you
 * write every time an error occurs. We aren't using this seemingly
 * easier callback approach because it doesn't make it easy to narrow
 * down the line(s) of code causing an error.
 */
#define kuhl_errorcheck() kuhl_errorcheckFileLine(__FILE__, __LINE__, __func__)



	
// kuhl_errorcheck() calls this C function:
int kuhl_errorcheckFileLine(const char *file, int line, const char *func);
// kuhl_malloc() calls this C function:
void* kuhl_mallocFileLine(size_t size, const char *file, int line);

GLFWwindow* kuhl_get_window();
void kuhl_ogl_init(int *argcp, char **argv, int width, int height, int oglProfile, int msaaSamples);

GLuint kuhl_create_shader(const char *filename, GLuint shader_type);
GLuint kuhl_create_program(const char *vertexFilename, const char *fragFilename);
void kuhl_delete_program(GLuint program);
void kuhl_print_program_log(GLuint program);
void kuhl_print_program_info(GLuint program);
GLint kuhl_get_uniform(const char *uniformName);
GLint kuhl_get_attribute(GLuint program, const char *attributeName);



void kuhl_bbox_transform(float bbox[6], float mat[16]);

#if 0
int kuhl_geometry_collide(kuhl_geometry *geom1, float mat1[16],
                          kuhl_geometry *geom2, float mat2[16]);
#endif

void kuhl_geometry_new(kuhl_geometry *geom, GLuint program, unsigned int vertexCount, GLint primitive_type);
void kuhl_geometry_draw(kuhl_geometry *geom);
void kuhl_geometry_delete(kuhl_geometry *geom);
unsigned int kuhl_geometry_count(const kuhl_geometry *geom);

void kuhl_geometry_program(kuhl_geometry *geom, GLuint program, int kg_options);
GLfloat* kuhl_geometry_attrib_get(kuhl_geometry *geom, const char *name, GLint *size);
void kuhl_geometry_indices(kuhl_geometry *geom, GLuint *indices, GLuint indexCount);
void kuhl_geometry_attrib(kuhl_geometry *geom, const GLfloat *data, GLuint components, const char* name, int kg_options);
void kuhl_geometry_texture(kuhl_geometry *geom, GLuint texture, const char* name, int kg_options);


GLuint kuhl_read_texture_array(const unsigned char* array, int width, int height, int components, GLuint wrapS, GLuint wrapT);
void kuhl_flip_texture_array(unsigned char *image, const int width, const int height, const int components);
GLuint kuhl_read_texture_rgba_array(const unsigned char *array, int width, int height);

float kuhl_make_label(const char *label, GLuint *texName, float color[3], float bgcolor[4], float pointsize);
kuhl_geometry* kuhl_label_geom(kuhl_geometry *geom, GLuint program, float *width,
                               const char *message, float color[3], float bgcolor[4], float pointsize);
float kuhl_read_texture_file_wrap(const char *filename, GLuint *texName, GLuint wrapS, GLuint wrapT);
float kuhl_read_texture_file(const char *filename, GLuint *texName);
void kuhl_screenshot(const char *outputImageFilename);
void kuhl_video_record(const char *fileLabel, int fps);

#ifdef KUHL_UTIL_USE_ASSIMP
void kuhl_update_model(kuhl_geometry *first_geom, unsigned int animationNum, float time);
kuhl_geometry* kuhl_load_model(const char *modelFilename, const char *textureDirname, GLuint program, float bbox[6]);
#endif // end use assimp

void kuhl_bbox_fit(float result[16], const float bbox[6], int sitOnXZPlane);
	
GLint kuhl_gen_framebuffer(int width, int height, GLuint *texture, GLuint *depthTexture);
GLint kuhl_gen_framebuffer_msaa(int width, int height, GLuint *texture, GLuint *depthTexture, GLint samples);

void kuhl_play_sound(const char *filename);

#ifdef __cplusplus
} // end extern "C"
#endif
