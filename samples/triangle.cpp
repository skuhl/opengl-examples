/* Copyright (c) 2014-2015 Scott Kuhl. All rights reserved.
* License: This code is licensed under a 3-clause BSD license. See
* the file named "LICENSE" for a full copy of the license.
*/

/** @file Demonstrates drawing a 3D triangle.
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

//#define assert(expression) (void)(                                                       \
//            (!!(expression)) ||                                                              \
//            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
//        )

//Bochao's code------------------------------------
#include "OVR_CAPI_GL.h"
//#include "OVR_Math.h"

void upVector(float retVec[3], float matrix[16], float inVec[3])
{
	/*const T rcpW = T(1) / (M[3][0] * v.x + M[3][1] * v.y + M[3][2] * v.z + M[3][3]);
	return Vector3<T>((M[0][0] * v.x + M[0][1] * v.y + M[0][2] * v.z + M[0][3]) * rcpW,
		(M[1][0] * v.x + M[1][1] * v.y + M[1][2] * v.z + M[1][3]) * rcpW,
		(M[2][0] * v.x + M[2][1] * v.y + M[2][2] * v.z + M[2][3]) * rcpW);*/

	float rcpW;
	rcpW = 1.0 / (matrix[3] * inVec[0] + matrix[7] * inVec[1] + matrix[11] * inVec[2] + matrix[15]);
	retVec[0] = matrix[0] * inVec[0] + matrix[4] * inVec[1] + matrix[8] * inVec[2] + matrix[12] * rcpW;
	retVec[1] = matrix[1] * inVec[0] + matrix[5] * inVec[1] + matrix[9] * inVec[2] + matrix[13] * rcpW;
	retVec[2] = matrix[2] * inVec[0] + matrix[6] * inVec[1] + matrix[10] * inVec[2] + matrix[14] * rcpW;

	//float rcpW = 1.0 / matrix[12] * inVec[0] + matrix[13] * inVec[1] + matrix[14] * inVec[2] + matrix[15];
	//retVec[0] = matrix[0] * inVec[0] + matrix[1] * inVec[1] + matrix[2] * inVec[2] + matrix[3] * rcpW;
	//retVec[1] = matrix[4] * inVec[0] + matrix[5] * inVec[1] + matrix[6] * inVec[2] + matrix[7] * rcpW;
	//retVec[2] = matrix[8] * inVec[0] + matrix[9] * inVec[1] + matrix[10] * inVec[2] + matrix[11] * rcpW;

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

//-------------------end Bochao's code


GLuint program = 0; /**< id value for the GLSL program */

kuhl_geometry triangle;
kuhl_geometry quad;


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
	}

	/* Whenever any key is pressed, request that display() get
	* called. */
	glutPostRedisplay();
}

/* Called by GLUT whenever the window needs to be redrawn. This
* function should not be called directly by the programmer. Instead,
* we can call glutPostRedisplay() to request that GLUT call display()
* at some point. */
void display()
{
	/* If we are using DGR, send or receive data to keep multiple
	* processes/computers synchronized. */
	dgr_update();

	/* Render the scene once for each viewport. Frequently one
	* viewport will fill the entire screen. However, this loop will
	* run twice for HMDs (once for the left eye and once for the
	* right). */

	//viewmat_begin_frame();

	//ovrEyeRenderDesc eyeRenderDesc[2];
	//eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	//eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

	//ovrPosef EyeRenderPose[2];
	//ovrVector3f HmdToEyeOffset[2];
	//HmdToEyeOffset[0] = eyeRenderDesc[0].HmdToEyeOffset;
	//HmdToEyeOffset[1] = eyeRenderDesc[1].HmdToEyeOffset;

	//double sensorSampleTime;
	////set position and orientation
	//ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyeOffset, EyeRenderPose, &sensorSampleTime);

	//Bochao's code manually change
	//for(int viewportID=0; viewportID<viewmat_num_viewports(); viewportID++)

	ovrEyeRenderDesc eyeRenderDesc[2];
	eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

	ovrPosef                  EyeRenderPose[2];
	ovrVector3f               HmdToEyeOffset[2] = { eyeRenderDesc[0].HmdToEyeOffset,
		eyeRenderDesc[1].HmdToEyeOffset };

	double sensorSampleTime;    // sensorSampleTime is fed into the layer later
	ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyeOffset, EyeRenderPose, &sensorSampleTime);

	for (int viewportID = 0; viewportID < 2; viewportID++)
	{
		eyeRenderTexture[viewportID]->SetAndClearRenderSurface(eyeDepthBuffer[viewportID]);

		//matrix4f rollpitchyaw = matrix4f::rotationy(yaw);
		//matrix4f finalrollpitchyaw = rollpitchyaw * matrix4f(eyerenderpose[eye].orientation);
		//vector3f finalup = finalrollpitchyaw.transform(vector3f(0, 1, 0));
		//vector3f finalforward = finalrollpitchyaw.transform(vector3f(0, 0, -1));
		//vector3f shiftedeyepos = pos2 + rollpitchyaw.transform(eyerenderpose[eye].position);

		//matrix4f view = matrix4f::lookatrh(shiftedeyepos, shiftedeyepos + finalforward, finalup);
		//matrix4f proj = ovrmatrix4f_projection(hmddesc.defaulteyefov[eye], 0.2f, 1000.0f, ovrprojection_none);

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
		mat4f_rotateAxisVec_new(rollPitchYaw, Yaw/0.0174533, axes);
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

		//viewmat_begin_eye(viewportID);


		/* Where is the viewport that we are drawing onto and what is its size? */
		//int viewport[4]; // x,y of lower left corner, width, height
						 //viewmat_get_viewport(viewport, viewportID);
						 /* Tell OpenGL the area of the window that we will be drawing in. */
		//glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

		/* Clear the current viewport. Without glScissor(), glClear()
		* clears the entire screen. We could call glClear() before
		* this viewport loop---but in order for all variations of
		* this code to work (Oculus support, etc), we can only draw
		* after viewmat_begin_eye(). */
		//glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		glEnable(GL_SCISSOR_TEST);
		glClearColor(.2, .2, .2, 0); // set clear color to grey
		//glClearColor(1, 0, 0, 0); // set clear color to grey
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		glEnable(GL_DEPTH_TEST); // turn on depth testing
		kuhl_errorcheck();

		/* Get the view matrix and the projection matrix */
		//float viewMat[16], perspective[16];
		//viewmat_get(viewMat, perspective, viewportID);

		/* Calculate an angle to rotate the
		* object. glutGet(GLUT_ELAPSED_TIME) is the number of
		* milliseconds since glutInit() was called. */
		int count = glutGet(GLUT_ELAPSED_TIME) % 10000; // get a counter that repeats every 10 seconds
		float angle = count / 10000.0 * 360; // rotate 360 degrees every 10 seconds
											 /* Make sure all computers/processes use the same angle */
		dgr_setget("angle", &angle, sizeof(GLfloat));

		/* Create a 4x4 rotation matrix based on the angle we computed. */
		float rotateMat[16];
		mat4f_rotateAxis_new(rotateMat, angle, 0, 1, 0);

		/* Create a scale matrix. */
		float scaleMat[16];
		mat4f_scale_new(scaleMat, 3, 3, 3);

		/* Combine the scale and rotation matrices into a single model matrix.
		modelMat = scaleMat * rotateMat
		*/
		float modelMat[16];
		mat4f_mult_mat4f_new(modelMat, scaleMat, rotateMat);

		/* Construct a modelview matrix: modelview = viewMat * modelMat */
		float modelview[16];
		mat4f_mult_mat4f_new(modelview, viewMat, modelMat);

		/* Tell OpenGL which GLSL program the subsequent
		* glUniformMatrix4fv() calls are for. */
		kuhl_errorcheck();
		glUseProgram(program);
		kuhl_errorcheck();

		/* Send the perspective projection matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("Projection"),
			1, // number of 4x4 float matrices
			0, // transpose
			projMat); // value
						  /* Send the modelview matrix to the vertex program. */
		glUniformMatrix4fv(kuhl_get_uniform("ModelView"),
			1, // number of 4x4 float matrices
			0, // transpose
			modelview); // value
		kuhl_errorcheck();
		/* Draw the geometry using the matrices that we sent to the
		* vertex programs immediately above */
		kuhl_geometry_draw(&triangle);
		kuhl_geometry_draw(&quad);

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

	/* Check for errors. If there are errors, consider adding more
	* calls to kuhl_errorcheck() in your code. */
	kuhl_errorcheck();

	/* Ask GLUT to call display() again. We shouldn't call display()
	* ourselves recursively because it will not leave time for GLUT
	* to call other callback functions for when a key is pressed, the
	* window is resized, etc. */
	glutPostRedisplay();
}

void init_geometryTriangle(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog, 3, // num vertices
		GL_TRIANGLES); // primitive type

					   /* Vertices that we want to form triangles out of. Every 3 numbers
					   * is a vertex position. Since no indices are provided, every
					   * three vertex positions form a single triangle.*/
	GLfloat vertexPositions[] = { 0, 0, 0,
		1, 0, 0,
		1, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions, // data
		3, // number of components (x,y,z)
		"in_Position", // GLSL variable
		KG_WARN); // warn if attribute is missing in GLSL program?
}


/* This illustrates how to draw a quad by drawing two triangles and reusing vertices. */
void init_geometryQuad(kuhl_geometry *geom, GLuint prog)
{
	kuhl_geometry_new(geom, prog,
		4, // number of vertices
		GL_TRIANGLES); // type of thing to draw

					   /* Vertices that we want to form triangles out of. Every 3 numbers
					   * is a vertex position. Below, we provide indices to form
					   * triangles out of these vertices. */
	GLfloat vertexPositions[] = { 0 + 1.1, 0, 0,
		1 + 1.1, 0, 0,
		1 + 1.1, 1, 0,
		0 + 1.1, 1, 0 };
	kuhl_geometry_attrib(geom, vertexPositions,
		3, // number of components x,y,z
		"in_Position", // GLSL variable
		KG_WARN); // warn if attribute is missing in GLSL program?

				  /* A list of triangles that we want to draw. "0" refers to the
				  * first vertex in our list of vertices. Every three numbers forms
				  * a single triangle. */
	GLuint indexData[] = { 0, 1, 2,
		0, 2, 3 };
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



	// setup callbacks
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	/* Compile and link a GLSL program composed of a vertex shader and
	* a fragment shader. */
	program = kuhl_create_program("triangle.vert", "triangle.frag");

	/* Use the GLSL program so subsequent calls to glUniform*() send the variable to
	the correct program. */
	glUseProgram(program);
	kuhl_errorcheck();
	/* Set the uniform variable in the shader that is named "red" to the value 1. */
	glUniform1i(kuhl_get_uniform("red"), 0);
	kuhl_errorcheck();
	/* Good practice: Unbind objects until we really need them. */
	glUseProgram(0);

	/* Create kuhl_geometry structs for the objects that we want to
	* draw. */
	init_geometryTriangle(&triangle, program);
	init_geometryQuad(&quad, program);

	dgr_init();     /* Initialize DGR based on environment variables. */
	projmat_init(); /* Figure out which projection matrix we should use based on environment variables */

	float initCamPos[3] = { 0,0,10 }; // location of camera
	float initCamLook[3] = { 0,0,0 }; // a point the camera is facing at
	float initCamUp[3] = { 0,1,0 }; // a vector indicating which direction is up
									// Bochao's code
									//bochao_viewmat_init(initCamPos, initCamLook, initCamUp);
									//viewmat_init(initCamPos, initCamLook, initCamUp);


									/* Tell GLUT to start running the main loop and to call display(),
									* keyboard(), etc callback methods as needed. */
	glutMainLoop();
	/* // An alternative approach:
	while(1)
	glutMainLoopEvent();
	*/

	exit(EXIT_SUCCESS);
}
