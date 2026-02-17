#pragma once

#include <fstream>

#include "../Common.h"

struct CompiledPipeline;
class CodeGenerator;
class WebGLExecutorNodeVisitor;

class RenderPipelineCodeGenerator
{
public:
	bool GenerateCode(const std::string& projectName, const CompiledPipeline& pipeline);

private:
	void WriteImports(CodeGenerator& generator);
	void GenerateVariableInitialization(CodeGenerator& generator, const CompiledPipeline& pipeline);
	void LinkResources(const std::string& projectPath, CompiledPipeline& pipeline);

	void WriteInputCallbacks(CodeGenerator& generator, const CompiledPipeline& pipeline, WebGLExecutorNodeVisitor& visitor);
	void RegisterInputCallbacks(CodeGenerator& generator, const CompiledPipeline& pipeline);

private:
	std::ofstream m_Out;
};