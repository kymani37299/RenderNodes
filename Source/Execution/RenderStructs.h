#pragma once

#include <vector>

#include "../Common.h"
#include "ValueNode.h"

struct Texture;

struct BindTable
{
	template<typename T>
	struct Binding
	{
		std::string Name;
		Ptr<ValueNode<T>> Value;
	};
	std::vector<Binding<Texture*>> Textures;
	std::vector<Binding<float>> Floats;
	std::vector<Binding<Float2>> Float2s;
	std::vector<Binding<Float3>> Float3s;
	std::vector<Binding<Float4>> Float4s;
	std::vector<Binding<Float4x4>> Float4x4s;
};

struct RenderState
{
	bool DepthWrite = false;
	GLenum DepthTest = GL_ALWAYS;
};