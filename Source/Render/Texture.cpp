#include "Texture.h"

#include "../Common.h"

Ptr<Texture> Texture::Create(unsigned width, unsigned height, unsigned flags)
{
	Texture* texture = new Texture{};
	texture->Width = width;
	texture->Height = height;

	if(flags & TF_Framebuffer)
	{
		// GL: If texture is not for reading we can use renderbuffer instead

		glGenFramebuffers(1, &texture->FrameBufferHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, texture->FrameBufferHandle); // GL: Not sure if needed, maybe delete ?
	}

	glGenTextures(1, &texture->TextureHandle);
	glBindTexture(GL_TEXTURE_2D, texture->TextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// Probably need some sampler struct
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (texture->FrameBufferHandle)
	{
		// GL: Probably bind framebuffer here instead
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->TextureHandle, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	return Ptr<Texture>(texture);
}

Texture::~Texture()
{
	if (FrameBufferHandle) glDeleteFramebuffers(1, &FrameBufferHandle);
}