#pragma once

#include <GL/glew.h>



// Freetype
#ifdef KUHL_UTIL_USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

typedef struct _font_info_ {
	#ifdef KUHL_UTIL_USE_FREETYPE
	FT_Face face;
	#endif
	unsigned int pointSize;
	unsigned int pixelsPerPoint;
	float color[4];
	//float colorBG[4];
	GLuint program;
	GLuint tex;
	GLuint vbo;
	GLuint vao;
	GLint uniform_tex;
	GLint attribute_coord;
} font_info;

int font_init();

int font_info_new(font_info* info, const GLuint program, const char* fontFile, const unsigned int pointSize, const unsigned int pixelsPerPoint);

void font_draw(font_info* info, const char *text, float x, float y);

void font_info_release(font_info* info);

void font_release();
