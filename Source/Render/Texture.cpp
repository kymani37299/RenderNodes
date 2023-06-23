#include "Texture.h"

#include "../Common.h"

Ptr<Texture> Texture::Create(unsigned width, unsigned height, unsigned flags, const void* textureData)
{
	unsigned numMips = 1;

	Texture* texture = new Texture{};
	texture->Width = width;
	texture->Height = height;
	texture->NumMips = numMips;
	texture->Flags = flags;

	GL_CALL(glGenTextures(1, &texture->TextureHandle));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, texture->TextureHandle));
	GL_CALL(glTexStorage2D(GL_TEXTURE_2D, numMips, GL_RGBA8, width, height));
	if (textureData)
	{
		GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, textureData));
	}

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, numMips-1));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMips-1));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT));

	if (flags & TF_Framebuffer)
	{
		GL_CALL(glGenFramebuffers(1, &texture->FrameBufferHandle));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, texture->FrameBufferHandle));
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->TextureHandle, 0));

		if (flags & TF_DepthStencil)
		{
			unsigned int depthStencilHandle;
			GL_CALL(glGenRenderbuffers(1, &depthStencilHandle));
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depthStencilHandle));
			GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
			GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilHandle));
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

			// The handle will still remain because of framebuffer reference, it will be deleted when framebuffer is deleted
			GL_CALL(glDeleteRenderbuffers(1, &depthStencilHandle)); 
		}
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

	return Ptr<Texture>(texture);
}

Texture::~Texture()
{
	GL_CALL(glDeleteTextures(1, &TextureHandle));
	if (FrameBufferHandle) GL_CALL(glDeleteFramebuffers(1, &FrameBufferHandle));
}