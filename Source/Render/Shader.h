#pragma once

#include <vector>

#include "../Common.h"

enum class ShaderStage
{
	Vertex,
	Fragment,
};

struct Shader
{
	static std::string GetShaderStageExtension(ShaderStage stage);
	static std::string FinalizeShaderCode(ShaderStage stage, const std::string& shaderVersion, const std::string& shaderCode);
	static bool ReadShaderFile(const std::string& path, std::string& outputCode, std::string& shaderVersion);
	
	static Ptr<Shader> Compile(const std::string& path, const std::vector<ShaderStage>& stages);

	~Shader();

	unsigned Handle;
};