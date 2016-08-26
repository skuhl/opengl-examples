#include "windows-compat.h"
#include "font-helper.h"
#include <GLFW/glfw3.h>
#include "kuhl-util.h"

//#define min(x, y) (x < y) ? x : y
//#define max(x, y) (x > y) ? x : y

#ifdef KUHL_UTIL_USE_FREETYPE
static FT_Library lib;

static int font_load(FT_Face* face, const char* fileName, const unsigned int pointSize) {
	char *newFilename = kuhl_find_file(fileName);
	if (newFilename == NULL || pointSize < 1) {
		fprintf(stderr, "Font: Could not open font - either a NULL file path, or invalid point size\n");
		free(newFilename);
		return 0;
	}
	if(FT_New_Face(lib, newFilename, 0, face)) {
		fprintf(stderr, "Font: Could not open font '%s'\n", fileName);
		free(newFilename);
		return 0;
	}
	free(newFilename);
	
	if(FT_Set_Pixel_Sizes(*face, 0, pointSize)) {
		fprintf(stderr, "Font: Could not resize font '%s'\n", (*face)->family_name);
		return 0;
	}
	if(FT_Load_Char(*face, 'X', FT_LOAD_RENDER)) {
		fprintf(stderr, "Font: Could not load character for font '%s'\n", (*face)->family_name);
		return 0;
	}
	printf("Font: Loaded %s successfully\n", (*face)->family_name);
	return 1;
}
#endif

/** Initializes the vertex array object (VAO), vertex buffer object (VBO) and texture
	associated with this font_info and initializes other variables inside of the struct.

    @param info A font_info struct populated with the information necessary to draw text.

    @param program A GLSL program to be used when rendering this font.

    @param fontFile The file path to the TTF file you wish to load.

    @param pointSize The point size of the font.

    @param pixelsPerPoint The number of screen pixels per glyph pixel (2 is usually good).
	
	@return ok Returns 1 if there were no problems, 0 if something went wrong (i.e. bad 
	shader program, invalid font file, etc.).
*/
int font_info_new(font_info* info, const GLuint program, const char* fontFile, const unsigned int pointSize, const unsigned int pixelsPerPoint)
{	
	#ifndef KUHL_UTIL_USE_FREETYPE
	return 0;
	#else
	// Set up the shader program
	glUseProgram(program);
	kuhl_errorcheck();
	
	// Make sure it's got the right variables
	info->uniform_tex = kuhl_get_uniform("tex");
	kuhl_errorcheck();
	if (info->uniform_tex < 0) {
		fprintf(stderr, "Could not bind texture uniform\n");
		return 0;
	}
	info->attribute_coord = kuhl_get_attribute(program, "coord");
	kuhl_errorcheck();
	if (info->attribute_coord < 0) {
		fprintf(stderr, "Could not bind coord attribute\n");
		return 0;
	}

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &info->tex);
	glBindTexture(GL_TEXTURE_2D, info->tex);
	glUniform1i(info->uniform_tex, 0);
	kuhl_errorcheck();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenVertexArrays(1, &info->vao);
	kuhl_errorcheck();
	glBindVertexArray(info->vao);
	kuhl_errorcheck();
	
	glGenBuffers(1, &info->vbo);
	kuhl_errorcheck();
	glEnableVertexAttribArray(info->attribute_coord);
	kuhl_errorcheck();
	glBindBuffer(GL_ARRAY_BUFFER, info->vbo);
	kuhl_errorcheck();
	glVertexAttribPointer(info->attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);
	kuhl_errorcheck();
	
	FT_Face face;
	if (!font_load(&face, fontFile, pointSize))
		return 0;
	
	info->face = face;
	info->program = program;
	info->pointSize = pointSize;
	info->pixelsPerPoint = pixelsPerPoint;
	
	return 1;
	#endif
}


/** Initializes FreeType.
	@return ok Returns 1 if it succeeded, 0 if it didn't.
*/
int font_init() {
	#ifndef KUHL_UTIL_USE_FREETYPE
	return 0;
	#else
	// Init FreeType
	if (FT_Init_FreeType(&lib)) {
		fprintf(stderr, "Could not initialize the FreeType library\n");
		return 0;
	}
	return 1;
	#endif
}

void font_info_release(font_info* info) 
{
	glDeleteTextures(1, &info->tex);
	glDeleteVertexArrays(1, &info->vao);
	glDeleteBuffers(1, &info->vbo);
}

void font_release() {
	#ifdef KUHL_UTIL_USE_FREETYPE
	FT_Done_FreeType(lib);
	#endif
}

static void render_char(font_info* info, const char ch, float* x, float* y, float sx, float sy, float startX, float startY) {
	#ifdef KUHL_UTIL_USE_FREETYPE
	FT_GlyphSlot g = info->face->glyph;
	if(FT_Load_Char(info->face, ch, FT_LOAD_RENDER))
		return;
	
	if (ch == '\n') {
		*y -= info->pointSize * sy;
		*x = startX;
		return;
	} else if (ch == '\r') {
		*x = startX;
		return;
	}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		g->bitmap.width,
		g->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		g->bitmap.buffer
	);
	kuhl_errorcheck();
	
	float x2 = *x + g->bitmap_left * sx;
	float y2 = -*y - g->bitmap_top * sy;
	float w = g->bitmap.width * sx;
	float h = g->bitmap.rows * sy;
	
	GLfloat box[4][4] = {
		{x2,     -y2    , 0, 0},
		{x2 + w, -y2    , 1, 0},
		{x2,     -y2 - h, 0, 1},
		{x2 + w, -y2 - h, 1, 1},
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
	kuhl_errorcheck();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	kuhl_errorcheck();
	
	*x += (g->advance.x >> 6) * sx;
	*y += (g->advance.y >> 6) * sy;
	#endif
}

void font_draw(font_info* info, const char *text, float x, float y) {
	if (info == NULL || text == NULL)
		return;
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, info->tex);
	glBindVertexArray(info->vao);
	glBindBuffer(GL_ARRAY_BUFFER, info->vbo);
	glEnableVertexAttribArray(info->attribute_coord);
	
	y += info->pointSize; // Bitmaps start at bottom-left corner.

	int windowWidth=0, windowHeight=0;
	glfwGetFramebufferSize(kuhl_get_window(), &windowWidth, &windowHeight);
	float aspect = 1.0; //min((float) glutGet(GLUT_WINDOW_WIDTH) / width, (float) glutGet(GLUT_WINDOW_HEIGHT) / height);
	float sx = aspect * (float)info->pixelsPerPoint / windowWidth;
	float sy = aspect * (float)info->pixelsPerPoint / windowHeight;

	x = -1 + x * sx;
	y = 1 - y * sy;
	float startX = x, startY = y;
	const char *p;
	for(p = text; *p; p++)
		render_char(info, *p, &x, &y, sx, sy, startX, startY);
}
