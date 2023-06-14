#pragma once

#include "../Common.h"

struct Shader
{
	static Ptr<Shader> Compile(const std::string& path);

	~Shader();

	unsigned Handle;
};