/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
* License: This code is licensed under a 3-clause BSD license. See
* the file named "LICENSE" for a full copy of the license.
*/

/** @file Loads 3D model files displays them.
*
* @author Scott Kuhl
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#ifdef FREEGLUT
#include <GL/freeglut.h>
#else
#include <GLUT/glut.h>
#endif

#include "kuhl-util.h"
#include "vecmat.h"
#include "dgr.h"
#include "projmat.h"
#include "viewmat.h"

//Bochao's code------------------------------------
#include "OVR_CAPI_GL.h"
//#include "OVR_Math.h"

void upVector(float retVec[3], float matrix[16], float inVec[3])
{
	float rcpW;
	rcpW = 1.0 / (matrix[3] * inVec[0] + matrix[7] * inVec[1] + matrix[11] * inVec[2] + matrix[15]);
	retVec[0] = matrix[0] * inVec[0] + matrix[4] * inVec[1] + matrix[8] * inVec[2] + matrix[12] * rcpW;
	retVec[1] = matrix[1] * inVec[0] + matrix[5] * inVec[1] + matrix[9] * inVec[2] + matrix[13] * rcpW;
	retVec[2] = matrix[2] * inVec[0] + matrix[6] * inVec[1] + matrix[10] * inVec[2] + matrix[14] * rcpW;
}

void mat4fToArray(float matrix[16], ovrMatrix4f mat4) {
	int index = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			matrix[index] = mat4.M[j][i]; // needed double check
			index++;
		}
	}
}

struct DepthBuffer
{
	GLuint        texId;

	DepthBuffer(ovrSizei size, int sampleCount)
	{
		UNREFERENCED_PARAMETER(sampleCount);

		//		assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLenum internalFormat = GL_DEPTH_COMPONENT24;
		GLenum type = GL_UNSIGNED_INT;
		//		if (GLE_ARB_depth_buffer_float)
		//		{
		internalFormat = GL_DEPTH_COMPONENT32F;
		type = GL_FLOAT;
		//		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h, 0, GL_DEPTH_COMPONENT, type, NULL);
	}
	~DepthBuffer()
	{
		if (texId)
		{
			glDeleteTextures(1, &texId);
			texId = 0;
		}
	}
};


struct TextureBuffer
{
	ovrSession          Session;
	ovrTextureSwapChain  TextureChain;
	GLuint              texId;
	GLuint              fboId;
	ovrSizei            texSize;

	TextureBuffer(ovrSession session, bool rendertarget, bool displayableOnHmd, ovrSizei size, int mipLevels, unsigned char * data, int sampleCount) :
		Session(session),
		TextureChain(0),
		texId(0),
		fboId(0)
	{
		texSize.w = 0;
		texSize.h = 0;

		UNREFERENCED_PARAMETER(sampleCount);

		//		assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

		texSize = size;

		if (displayableOnHmd)
		{
			// This texture isn't necessarily going to be a rendertarget, but it usually is.
			//			assert(session); // No HMD? A little odd.
			//			assert(sampleCount == 1); // ovr_CreateSwapTextureSetD3D11 doesn't support MSAA.

			ovrTextureSwapChainDesc desc = {};
			desc.Type = ovrTexture_2D;
			desc.ArraySize = 1;
			desc.Width = size.w;
			desc.Height = size.h;
			desc.MipLevels = 1;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.SampleCount = 1;
			desc.StaticImage = ovrFalse;

			ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &TextureChain);

			int length = 0;
			ovr_GetTextureSwapChainLength(session, TextureChain, &length);

			if (OVR_SUCCESS(result))
			{
				for (int i = 0; i < length; ++i)
				{
					GLuint chainTexId;
					ovr_GetTextureSwapChainBufferGL(Session, TextureChain, i, &chainTexId);
					glBindTexture(GL_TEXTURE_2D, chainTexId);

					if (rendertarget)
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					}
				}
			}
		}
		else
		{
			glGenTextures(1, &texId);
			glBindTexture(GL_TEXTURE_2D, texId);

			if (rendertarget)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texSize.w, texSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		if (mipLevels > 1)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glGenFramebuffers(1, &fboId);
	}

	~TextureBuffer()
	{
		if (TextureChain)
		{
			ovr_DestroyTextureSwapChain(Session, TextureChain);
			TextureChain = nullptr;
		}
		if (texId)
		{
			glDeleteTextures(1, &texId);
			texId = 0;
		}
		if (fboId)
		{
			glDeleteFramebuffers(1, &fboId);
			fboId = 0;
		}
	}

	ovrSizei GetSize() const
	{
		return texSize;
	}

	void SetAndClearRenderSurface(DepthBuffer* dbuffer)
	{
		GLuint curTexId;
		if (TextureChain)
		{
			int curIndex;
			ovr_GetTextureSwapChainCurrentIndex(Session, TextureChain, &curIndex);
			ovr_GetTextureSwapChainBufferGL(Session, TextureChain, curIndex, &curTexId);
		}
		else
		{
			curTexId = texId;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbuffer->texId, 0);

		glViewport(0, 0, texSize.w, texSize.h);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	void UnsetRenderSurface()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	}

	void Commit()
	{
		if (TextureChain)
		{
			ovr_CommitTextureSwapChain(Session, TextureChain);
		}
	}
};
//for oculus rendering
TextureBuffer * eyeRenderTexture[2] = { nullptr, nullptr };
DepthBuffer   * eyeDepthBuffer[2] = { nullptr, nullptr };
ovrMirrorTexture mirrorTexture = nullptr;
GLuint          mirrorFBO = 0;
ovrHmdDesc hmdDesc;
ovrSizei windowSize;
static float Yaw(3.141592f);

long long frameIndex = 0;

ovrSession session;
ovrGraphicsLuid luid;
ovrResult result;

//for mirror rendering
//ovrMirrorTexture MirrorTexture;

void cleanUpOculus() {
	if (mirrorFBO) glDeleteFramebuffers(1, &mirrorFBO);
	if (mirrorTexture) ovr_DestroyMirrorTexture(session, mirrorTexture);
	for (int eye = 0; eye < 2; ++eye)
	{
		delete eyeRenderTexture[eye];
		delete eyeDepthBuffer[eye];
	}
	//Platform.ReleaseDevice();
	ovr_Destroy(session);
}

//-------------------end Bochao's initialization code


static kuhl_fps_state fps_state;
GLuint fpsLabel = 0;
float fpsLabelAspectRatio = 0;
kuhl_geometry labelQuad;
int renderStyle = 2;

GLuint program = 0; // id value for the GLSL program
kuhl_geometry *modelgeom = NULL;
kuhl_geometry *origingeom = NULL;
float bbox[6];

int fitToView = 0;  // was --fit option used?

					/** The following variable toggles the display an "origin+axis" marker
					* which draws a small box at the origin and draws lines of length 1
					* on each axis. Depending on which matrices are applied to the
					* marker, the marker will be in object, world, etc coordinates. */
int showOrigin = 0; // was --origin option used?


					/** Initial position of the camera. 1.55 is a good approximate
					* eyeheight in meters.*/
const float initCamPos[3] = { 0,1.55,0 };

/** A point that the camera should initially be looking at. If
* fitToView is set, this will also be the position that model will be
* translated to. */
const float initCamLook[3] = { 0,0,-5 };

/** A vector indicating which direction is up. */
const float initCamUp[3] = { 0,1,0 };


/** SketchUp produces files that older versions of ASSIMP think 1 unit
* is 1 inch. However, all of this software assumes that 1 unit is 1
* meter. So, we need to convert some models from inches to
* meters. Newer versions of ASSIMP correctly read the same files and
* give us units in meters. */
#define INCHES_TO_METERS 0

#define GLSL_VERT_FILE "assimp.vert"
#define GLSL_FRAG_FILE "assimp.frag"

/** Called by GLUT whenever a key is pressed. */
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
	case 'Q':
	case 27: // ASCII code for Escape key
		dgr_exit();
		exit(EXIT_SUCCESS);
		break;
	case 'f': // full screen
		glutFullScreen();
		break;
	case 'F': // switch to window from full screen mode
		glutPositionWindow(0, 0);
		break;
	case 'r':
	{
		// Reload GLSL program from disk
		kuhl_delete_program(program);
		program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);
		/* Apply the program to the model geometry */
		kuhl_geometry_program(modelgeom, program, KG_FULL_LIST);
		/* and the fps label*/
		kuhl_geometry_program(&labelQuad, program, KG_FULL_LIST);

		break;
	}
	case 'w':
	{
		// Toggle between wireframe and solid
		int polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
		if (polygonMode == GL_LINE)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	}
	case 'p':
	{
		// Toggle between points and solid
		int polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
		if (polygonMode == GL_POINT)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	}
	case 'c':
	{
		// Toggle front, back, and no culling
		int cullMode;
		glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
		if (glIsEnabled(GL_CULL_FACE))
		{
			if (cullMode == GL_FRONT)
			{
				glCullFace(GL_BACK);
				printf("Culling: Culling back faces; drawing front faces\n");
			}
			else
			{
				glDisable(GL_CULL_FACE);
				printf("Culling: No culling; drawing all faces.\n");
			}
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			printf("Culling: Culling front faces; drawing back faces\n");
		}
		kuhl_errorcheck();
		break;
	}
	case 'd': // toggle depth clamping
	{
		if (glIsEnabled(GL_DEPTH_CLAMP))
		{
			printf("Depth clamping disabled\n");
			glDisable(GL_DEPTH_CLAMP); // default
			glDepthFunc(GL_LESS);      // default
		}
		else
		{
			/* With depth clamping, vertices beyond the near and
			far planes are clamped to the near and far
			planes. Since multiple layers of vertices will be
			clamped to the same depth value, depth testing
			beyond the near and far planes won't work. */
			printf("Depth clamping enabled\n");
			glEnable(GL_DEPTH_CLAMP);
			glDepthFunc(GL_LEQUAL); // makes far clamping work.
		}
		break;
	}
	case '+': // increase size of points and width of lines
	{
		GLfloat currentPtSize;
		GLfloat sizeRange[2];
		glGetFloatv(GL_POINT_SIZE, &currentPtSize);
		glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, sizeRange);
		GLfloat temp = currentPtSize + 1;
		if (temp > sizeRange[1])
			temp = sizeRange[1];
		glPointSize(temp);
		printf("Point size is %f (can be between %f and %f)\n", temp, sizeRange[0], sizeRange[1]);
		kuhl_errorcheck();

		GLfloat currentLineWidth;
		GLfloat widthRange[2];
		glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
		glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, widthRange);
		temp = currentLineWidth + 1;
		if (temp > widthRange[1])
			temp = widthRange[1];
		glLineWidth(temp);
		printf("Line width is %f (can be between %f and %f)\n", temp, widthRange[0], widthRange[1]);
		kuhl_errorcheck();
		break;
	}
	case '-': // decrease size of points and width of lines
	{
		GLfloat currentPtSize;
		GLfloat sizeRange[2];
		glGetFloatv(GL_POINT_SIZE, &currentPtSize);
		glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, sizeRange);
		GLfloat temp = currentPtSize - 1;
		if (temp < sizeRange[0])
			temp = sizeRange[0];
		glPointSize(temp);
		printf("Point size is %f (can be between %f and %f)\n", temp, sizeRange[0], sizeRange[1]);
		kuhl_errorcheck();

		GLfloat currentLineWidth;
		GLfloat widthRange[2];
		glGetFloatv(GL_LINE_WIDTH, &currentLineWidth);
		glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, widthRange);
		temp = currentLineWidth - 1;
		if (temp < widthRange[0])
			temp = widthRange[0];
		glLineWidth(temp);
		printf("Line width is %f (can be between %f and %f)\n", temp, widthRange[0], widthRange[1]);
		kuhl_errorcheck();
		break;
	}

	case ' ': // Toggle different sections of the GLSL fragment shader
		renderStyle++;
		if (renderStyle > 9)
			renderStyle = 0;
		switch (renderStyle)
		{
		case 0: printf("Render style: Diffuse (headlamp light)\n"); break;
		case 1: printf("Render style: Texture (color is used on non-textured geometry)\n"); break;
		case 2: printf("Render style: Texture+diffuse (color is used on non-textured geometry)\n"); break;
		case 3: printf("Render style: Vertex color\n"); break;
		case 4: printf("Render style: Vertex color + diffuse (headlamp light)\n"); break;
		case 5: printf("Render style: Normals\n"); break;
		case 6: printf("Render style: Texture coordinates\n"); break;
		case 7: printf("Render style: Front (green) and back (red) faces based on winding\n"); break;
		case 8: printf("Render style: Front (green) and back (red) based on normals\n"); break;
		case 9: printf("Render style: Depth (white=far; black=close)\n"); break;
		}
		break;
	}

	/* Whenever any key is pressed, request that display() get
	* called. */
	glutPostRedisplay();
}


/** Gets a model matrix which is appropriate for the model that we have loaded. */
void get_model_matrix(float result[16])
{
	mat4f_identity(result);

	if (fitToView == 1) // if --fit option was provided.
	{
		float fitMat[16];
		float transMat[16];

		/* Get a matrix to scale+translate the model based on the bounding
		* box. If the last parameter is 1, the bounding box will sit on
		* the XZ plane. If it is set to 0, the bounding box will be
		* centered at the specified point. */
		kuhl_bbox_fit(fitMat, bbox, 1);

		/* Translate the model to the point the camera is looking at. */
		mat4f_translateVec_new(transMat, initCamLook);

		mat4f_mult_mat4f_new(result, transMat, fitMat);
		return;
	}
	else  // if NOT fitting to view.
	{
		if (INCHES_TO_METERS)
		{
			float inchesToMeters = 1 / 39.3701;
			mat4f_scale_new(result, inchesToMeters, inchesToMeters, inchesToMeters);
		}
		return;
	}
}



/** Called by GLUT whenever the window needs to be redrawn. This
* function should not be called directly by the programmer. Instead,
* we can call glutPostRedisplay() to request that GLUT call display()
* at some point. */
void display()
{
	/* If we are using DGR, send or receive data to keep multiple
	* processes/computers synchronized. */
	dgr_update();

	/* Get current frames per second calculations. */
	float fps = kuhl_getfps(&fps_state);

	if (dgr_is_enabled() == 0 || dgr_is_master())
	{
		// If DGR is being used, only display FPS info if we are
		// the master process.

		// Check if FPS value was just updated by kuhl_getfps()
		if (fps_state.frame == 0)
		{
			char label[1024];
			snprintf(label, 1024, "FPS: %0.1f", fps);

			/* Delete old label if it exists */
			if (fpsLabel != 0)
				glDeleteTextures(1, &fpsLabel);

			/* Make a new label */
			float labelColor[3] = { 1,1,1 };
			float labelBg[4] = { 0,0,0,.3 };

			/* Change the last parameter (point size) to adjust the
			* size of the texture that the text is rendered in to. */
			fpsLabelAspectRatio = kuhl_make_label(label,
				&fpsLabel,
				labelColor, labelBg, 24);

			if (fpsLabel != 0)
				kuhl_geometry_texture(&labelQuad, fpsLabel, "tex", 1);
		}
	}

	/* Ensure the slaves use the same render style as the master
	* process. */
	dgr_setget("style", &renderStyle, sizeof(int));


	/* Render the scene once for each viewport. Frequently one
	* viewport will fill the entire screen. However, this loop will
	* run twice for HMDs (once for the left eye and once for the
	* right. */
	//viewmat_begin_frame();
	ovrEyeRenderDesc eyeRenderDesc[2];
	eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

	ovrPosef                  EyeRenderPose[2];
	ovrVector3f               HmdToEyeOffset[2] = { eyeRenderDesc[0].HmdToEyeOffset,
		eyeRenderDesc[1].HmdToEyeOffset };

	double sensorSampleTime;    // sensorSampleTime is fed into the layer later
	ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyeOffset, EyeRenderPose, &sensorSampleTime);

	for (int viewportID = 0; viewportID<2; viewportID++)
	{
		//viewmat_begin_eye(viewportID);
		//Bochao's Oculus Eye renders
		eyeRenderTexture[viewportID]->SetAndClearRenderSurface(eyeDepthBuffer[viewportID]);

		float rollPitchYaw[16];
		float finalRollPitchYaw[16];
		float quatMatrix[16];
		float axes[3] = { 0, 1, 0 };
		float axesForward[3] = { 0, 0, -1 };
		float pos2[3] = { 0.0f ,0.0f, -5.0f };
		float orientationQuat[4];
		float posVec[3];
		float tempVec[3];
		float finalUp[3];
		float finalForward[3];
		float shiftedEyePos[3];
		float eyeCenter[3];
		float viewMat[16];
		ovrMatrix4f tempProj;
		float projMat[16];
		//get oculus orientation
		orientationQuat[0] = EyeRenderPose[viewportID].Orientation.x;
		orientationQuat[1] = EyeRenderPose[viewportID].Orientation.y;
		orientationQuat[2] = EyeRenderPose[viewportID].Orientation.z;
		orientationQuat[3] = EyeRenderPose[viewportID].Orientation.w;
		//get oculus predicted position
		posVec[0] = EyeRenderPose[viewportID].Position.x;
		posVec[1] = EyeRenderPose[viewportID].Position.y;
		posVec[2] = EyeRenderPose[viewportID].Position.z;

		mat4f_rotateQuatVec_new(quatMatrix, orientationQuat);
		mat4f_rotateAxisVec_new(rollPitchYaw, Yaw / 0.0174533, axes);
		mat4f_mult_mat4f_new(finalRollPitchYaw, rollPitchYaw, quatMatrix);
		upVector(finalUp, finalRollPitchYaw, axes);
		upVector(finalForward, finalRollPitchYaw, axesForward);
		upVector(tempVec, rollPitchYaw, posVec);
		//caculate the shifted eye position
		shiftedEyePos[0] = pos2[0] + tempVec[0];
		shiftedEyePos[1] = pos2[1] + tempVec[1];
		shiftedEyePos[2] = pos2[2] + tempVec[2];
		//calculate center vector
		eyeCenter[0] = shiftedEyePos[0] + finalForward[0];
		eyeCenter[1] = shiftedEyePos[1] + finalForward[1];
		eyeCenter[2] = shiftedEyePos[2] + finalForward[2];
		//construct view matrix
		mat4f_lookatVec_new(viewMat, shiftedEyePos, eyeCenter, finalUp);
		//construct projection matrix
		tempProj = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[viewportID], 0.2f, 1000.0f, ovrProjection_None);
		mat4fToArray(projMat, tempProj);

		/* Where is the viewport that we are drawing onto and what is its size? */
		//int viewport[4]; // x,y of lower left corner, width, height
		//viewmat_get_viewport(viewport, viewportID);
		//glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Clear the current viewport. Without glScissor(), glClear()
		* clears the entire screen. We could call glClear() before
		* this viewport loop---but on order for all variations of
		* this code to work (Oculus support, etc), we can only draw
		* after viewmat_begin_eye(). */
		//glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(.2, .2, .2, 0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		/* Turn on blending (note, if you are using transparent textures,
		the transparency may not look correct unless you draw further
		items before closer items.). */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		/* Get the view or camera matrix; update the frustum values if needed. */
		//float viewMat[16], perspective[16];
		//viewmat_get(viewMat, perspective, viewportID);

		glUseProgram(program);
		kuhl_errorcheck();
		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			projMat); // value

		float modelMat[16];
		get_model_matrix(modelMat);
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, modelMat); // modelview = view * model

															/* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			modelview); // value

		glUniform1i(kuhl_get_uniform("renderStyle"), renderStyle);

		kuhl_errorcheck();
		kuhl_geometry_draw(modelgeom); /* Draw the model */
		kuhl_errorcheck();
		if (showOrigin && origingeom != NULL)
		{
			/* Save current line width */
			GLfloat origLineWidth;
			glGetFloatv(GL_LINE_WIDTH, &origLineWidth);
			glLineWidth(4); // make lines thick

							/* Object coordinate system origin */
			kuhl_geometry_draw(origingeom); /* Draw the origin marker */

											/* World coordinate origin */
			mat4f_copy(modelview, viewMat);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
				1, // number of 4x4 float matrices
				0, // transpose
				modelview); // value
			kuhl_geometry_draw(origingeom); /* Draw the origin marker */

											/* Restore line width */
			glLineWidth(origLineWidth);
		}

		// aspect ratio will be zero when the program starts (and FPS hasn't been computed yet)
		if ((dgr_is_enabled() == 0 || dgr_is_master()) && fpsLabelAspectRatio != 0)
		{
			/* The shape of the frames per second quad depends on the
			* aspect ratio of the label texture and the aspect ratio of
			* the window (because we are placing the quad in normalized
			* device coordinates). */
			int windowWidth, windowHeight;
			viewmat_window_size(&windowWidth, &windowHeight);
			float windowAspect = windowWidth / (float)windowHeight;

			float stretchLabel[16];
			mat4f_scale_new(stretchLabel, 1 / 8.0 * fpsLabelAspectRatio / windowAspect, 1 / 8.0, 1);

			/* Position label in the upper left corner of the screen */
			float transLabel[16];
			mat4f_translate_new(transLabel, -.9, .8, 0);
			mat4f_mult_mat4f_new(modelview, transLabel, stretchLabel);
			glUniformMatrix4fv(kuhl_get_uniform("ModelView"), 1, 0, modelview);

			/* Make sure we don't use a projection matrix */
			float identity[16];
			mat4f_identity(identity);
			glUniformMatrix4fv(kuhl_get_uniform("Projection"), 1, 0, identity);

			/* Don't use depth testing and make sure we use the texture
			* rendering style */
			glDisable(GL_DEPTH_TEST);
			glUniform1i(kuhl_get_uniform("renderStyle"), 1);
			kuhl_geometry_draw(&labelQuad); /* Draw the quad */
			glEnable(GL_DEPTH_TEST);
			kuhl_errorcheck();
		}

		// Avoids an error when calling SetAndClearRenderSurface during next iteration.
		// Without this, during the next while loop iteration SetAndClearRenderSurface
		// would bind a framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
		// associated with COLOR_ATTACHMENT0 had been unlocked by calling wglDXUnlockObjectsNV.
		eyeRenderTexture[viewportID]->UnsetRenderSurface();

		// Commit changes to the textures so they get picked up frame
		eyeRenderTexture[viewportID]->Commit();

		glUseProgram(0); // stop using a GLSL program.

	} // finish viewport loop
	//viewmat_end_frame();
	// Do distortion rendering, Present and flush/sync

	ovrLayerEyeFov ld;
	ld.Header.Type = ovrLayerType_EyeFov;
	ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

	for (int eye = 0; eye < 2; ++eye)
	{
		ld.ColorTexture[eye] = eyeRenderTexture[eye]->TextureChain;
		ld.Viewport[eye].Pos.x = 0;
		ld.Viewport[eye].Pos.y = 0;
		ld.Viewport[eye].Size = eyeRenderTexture[eye]->GetSize();
		ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
		ld.RenderPose[eye] = EyeRenderPose[eye];
		ld.SensorSampleTime = sensorSampleTime;
	}

	ovrLayerHeader* layers = &ld.Header;
	ovrResult result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	GLint w = windowSize.w;
	GLint h = windowSize.h;
	glBlitFramebuffer(0, h, w, 0,
		0, 0, w, h,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	//SwapBuffers(Platform.hDC);
	glutSwapBuffers();

	frameIndex++;

	/* Update the model for the next frame based on the time. We
	* convert the time to seconds and then use mod to cause the
	* animation to repeat. */
	int time = glutGet(GLUT_ELAPSED_TIME);
	dgr_setget("time", &time, sizeof(int));
	kuhl_update_model(modelgeom, 0, ((time % 10000) / 1000.0));

	/* Check for errors. If there are errors, consider adding more
	* calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	//kuhl_video_record("videoout", 30);

	/* Ask GLUT to call display() again. We shouldn't call display()
	* ourselves recursively because it will not leave time for GLUT
	* to call other callback functions for when a key is pressed, the
	* window is resized, etc. */
	glutPostRedisplay();
}

/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuad(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog,
		4, // number of vertices
		GL_TRIANGLES); // type of thing to draw

					   /* The data that we want to draw */
	GLfloat vertexPositions[] = { 0, 0, 0,
		1, 0, 0,
		1, 1, 0,
		0, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions,
		3, // number of components x,y,z
		"in_Position", // GLSL variable
		KG_WARN); // warn if attribute is missing in GLSL program?

	GLfloat texcoord[] = { 0, 0,
		1, 0,
		1, 1,
		0, 1 };
	kuhl_geometry_attrib(geom, texcoord,
		2, // number of components x,y,z
		"in_TexCoord", // GLSL variable
		KG_WARN); // warn if attribute is missing in GLSL program?




	GLuint indexData[] = { 0, 1, 2,  // first triangle is index 0, 1, and 2 in the list of vertices
		0, 2, 3 }; // indices of second triangle.
	kuhl_geometry_indices(geom, indexData, 6);

	kuhl_errorcheck();
}


void main(int argc, char** argv)
{
	//Bochao's code ------------------------------
	result = ovr_Initialize(nullptr);
	//Scene         * roomScene = nullptr;
	bool isVisible = true;
	frameIndex = 0;
	result = ovr_Create(&session, &luid);
	if (!OVR_SUCCESS(result))
		return;

	hmdDesc = ovr_GetHmdDesc(session);
	windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
	//--------------------------------end Bochao's code

	/* Initialize GLUT and GLEW */
	kuhl_ogl_init(&argc, argv, windowSize.w, windowSize.h, 32,
		GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE, 4);

	for (int eye = 0; eye < 2; ++eye)
	{
		ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
		eyeRenderTexture[eye] = new TextureBuffer(session, true, true, idealTextureSize, 1, NULL, 1);
		eyeDepthBuffer[eye] = new DepthBuffer(eyeRenderTexture[eye]->GetSize(), 0);
		/*if (!eyeRenderTexture[eye]->TextureChain)
		{
		if (retryCreate) goto Done;
		VALIDATE(false, "Failed to create texture.");
		}*/
	}

	//mirror texture
	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = windowSize.w;
	desc.Height = windowSize.h;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	result = ovr_CreateMirrorTextureGL(session, &desc, &mirrorTexture);
	if (!OVR_SUCCESS(result))
	{
		printf("error: failed to create mirror texture.\n");
		return;
	}

	GLuint texId;
	ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &texId);

	glGenFramebuffers(1, &mirrorFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	//Dr. Kuhl's code
	//--------------------

	ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);


	char *modelFilename = NULL;
	char *modelTexturePath = NULL;

	int currentArgIndex = 1; // skip program name
	int usageError = 0;
	while (argc > currentArgIndex)
	{
		if (strcmp(argv[currentArgIndex], "--fit") == 0)
			fitToView = 1;
		else if (strcmp(argv[currentArgIndex], "--origin") == 0)
			showOrigin = 1;
		else if (modelFilename == NULL)
		{
			modelFilename = argv[currentArgIndex];
			modelTexturePath = NULL;
		}
		else if (modelTexturePath == NULL)
			modelTexturePath = argv[currentArgIndex];
		else
		{
			usageError = 1;
		}
		currentArgIndex++;
	}

	// If we have no model to load or if there were too many arguments.
	if (modelFilename == NULL || usageError)
	{
		printf("Usage:\n"
			"%s [--fit] [--origin] modelFile     - Textures are assumed to be in the same directory as the model.\n"
			"- or -\n"
			"%s [--fit] [--origin] modelFile texturePath\n"
			"If the optional --fit parameter is included, the model will be scaled and translated to fit within the approximate view of the camera\n"
			"If the optional --origin parameter is included, a box will is drawn at the origin and unit-length lines are drawn down each axis.\n",
			argv[0], argv[0]);
		exit(EXIT_FAILURE);
	}


	// setup callbacks
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	/* Compile and link a GLSL program composed of a vertex shader and
	* a fragment shader. */
	program = kuhl_create_program(GLSL_VERT_FILE, GLSL_FRAG_FILE);

	dgr_init();     /* Initialize DGR based on environment variables. */
	projmat_init(); /* Figure out which projection matrix we should use based on environment variables */

	//viewmat_init(initCamPos, initCamLook, initCamUp);

	// Clear the screen while things might be loading
	glClearColor(.2, .2, .2, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// Load the model from the file
	modelgeom = kuhl_load_model(modelFilename, modelTexturePath, program, bbox);
	init_geometryQuad(&labelQuad, program);

	kuhl_getfps_init(&fps_state);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	/* Tell GLUT to start running the main loop and to call display(),
	* keyboard(), etc callback methods as needed. */
	glutMainLoop();
	/* // An alternative approach:
	while(1)
	glutMainLoopEvent();
	*/

	cleanUpOculus();

	exit(EXIT_SUCCESS);
}
