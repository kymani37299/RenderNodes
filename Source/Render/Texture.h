#pragma once

#include "../Common.h"

enum TextureFlags : unsigned
{
	TF_None = 0,

	TF_Framebuffer = 1,
};

struct Texture
{
	static Ptr<Texture> Create(unsigned width, unsigned height, unsigned flags = TF_None);

	~Texture();

	unsigned Width = 0;
	unsigned Height = 0;
	unsigned Flags = TF_None;

	unsigned FrameBufferHandle = 0;
	unsigned TextureHandle = 0;
};
