#pragma once

#if !defined(MISSING_OVR) && defined(_WIN32)
#include "dispmode.h"

//#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"

#include <GL/glew.h>

struct DepthBuffer
{
	GLuint        texId;

	DepthBuffer(ovrSizei size, int sampleCount)
	{
	//		UNREFERENCED_PARAMETER(sampleCount);

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

	//		UNREFERENCED_PARAMETER(sampleCount);

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




class dispmodeOculusWindows : public dispmode
{
private:
	ovrEyeType dispmodeOculusWindows::get_ovrEye(int viewportID);

public:
	dispmodeOculusWindows();
	~dispmodeOculusWindows();
	virtual viewmat_eye eye_type(int viewportID);
	virtual int num_viewports(void);
	virtual void get_viewport(int viewportValue[4], int viewportId);
	virtual void get_frustum(float result[6], int viewportID);
	virtual void get_projmatrix(float result[6], int viewportID);
	virtual void begin_frame();
	virtual void end_frame();
	virtual void begin_eye(int viewportId);
	virtual void end_eye(int viewportId);
	virtual int get_framebuffer(int viewportID);
	void get_eyeoffset(float offset[3], viewmat_eye eye);


	TextureBuffer * eyeRenderTexture[2] = { nullptr, nullptr };
	DepthBuffer   * eyeDepthBuffer[2] = { nullptr, nullptr };
	ovrHmdDesc hmdDesc;
	ovrSession session;
	ovrPosef EyeRenderPose[2];
	ovrVector3f HmdToEyeOffset[2];
	double sensorSampleTime;
	long long frameIndex;

	// Mirroring
	ovrMirrorTexture mirrorTexture = nullptr;
	GLuint          mirrorFBO = 0;
};


#endif
