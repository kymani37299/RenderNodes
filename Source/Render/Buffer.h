#pragma once

#include "../Common.h"

enum BufferFlags : unsigned
{
	BF_None = 0,


};

enum class BufferType
{
	Invalid,
	Vertex,
	Index,
	Uniform,
	Storage,
};

struct Buffer
{
	static Ptr<Buffer> Create(BufferType type, unsigned byteSize, unsigned stride, unsigned flags, const void* data = nullptr);
	
	~Buffer();

	BufferType Type = BufferType::Invalid;
	unsigned Flags = 0;
	unsigned ByteSize = 0;
	unsigned Stride = 0;

	unsigned Handle = 0;
};