#pragma once

#include <fstream>
#include <vector>

#include "../Common.h"

struct CompiledPipeline;
struct ShaderData;
class CodeGenerator;
class WebGLExecutorNodeVisitor;

class RenderPipelineCodeGenerator
{
public:
	bool GenerateCode(const std::string& projectName, const CompiledPipeline& pipeline, std::string& absoluteOutputPath);

private:
	void WriteImports(CodeGenerator& generator);
	void GenerateVariableInitialization(CodeGenerator& generator, const CompiledPipeline& pipeline);
	void LinkResources(const std::string& projectPath, CompiledPipeline& pipeline);

	void WriteInputCallbacks(CodeGenerator& generator, const CompiledPipeline& pipeline, WebGLExecutorNodeVisitor& visitor);
	void RegisterInputCallbacks(CodeGenerator& generator, const CompiledPipeline& pipeline);
private:
	std::ofstream m_Out;

	bool m_CompilationSuccess = true;
	std::vector<std::string> m_ErrorMessages;
};