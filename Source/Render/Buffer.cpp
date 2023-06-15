#include "Buffer.h"

GLenum GetGLBufferType(BufferType type)
{
	switch (type)
	{
	case BufferType::Vertex: return GL_ARRAY_BUFFER;
	case BufferType::Index: return GL_ELEMENT_ARRAY_BUFFER;
	case BufferType::Uniform: return GL_UNIFORM_BUFFER;
	case BufferType::Storage: return GL_SHADER_STORAGE_BUFFER;
	case BufferType::Invalid:
	default:
		NOT_IMPLEMENTED;
		break;
	}
	return GL_INVALID_ENUM;
}

Ptr<Buffer> Buffer::Create(BufferType type, unsigned byteSize, unsigned stride, unsigned flags, const void* data)
{
	Buffer* buffer = new Buffer{};
	buffer->Type = type;
	buffer->Flags = flags;
	buffer->ByteSize = byteSize;
	buffer->Stride = stride;

	GL_CALL(glGenBuffers(1, &buffer->Handle));
	GL_CALL(glBindBuffer(GetGLBufferType(type), buffer->Handle));
	GL_CALL(glBufferData(GetGLBufferType(type), byteSize, data, GL_STATIC_DRAW));

	return Ptr<Buffer>{buffer};
}

Buffer::~Buffer()
{
	GL_CALL(glDeleteBuffers(1, &Handle));
}
