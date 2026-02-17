#include "RenderPipelineCodeGenerator.h"

#include "CodeGenerator.h"
#include "../Execution/ExecuteContext.h"
#include "../Execution/ExecutorNodeVisitor.h"
#include "../Util/FileUtils.h"
#include "../Util/GLFWUtils.h"

static std::string CodegenFileName = "nodegraph-codegen.js";
static std::string TemplateLocation = "./Data/WebGPU_Template";
static std::string GeneratedProjectsLocation = "./Generated";
static std::string ResourcesDirectoryName = "Resources";

bool RenderPipelineCodeGenerator::GenerateCode(const std::string& projectName, const CompiledPipeline& pipeline)
{
	const std::string projectDirectoryLocation = GeneratedProjectsLocation + "/" + projectName;
	const std::string codegenFileLocation = projectDirectoryLocation + "/" + CodegenFileName;

	FileUtils::DeleteDirectoryIfExists(projectDirectoryLocation);
	FileUtils::MakeDirectory(projectDirectoryLocation);

	LinkResources(projectDirectoryLocation, const_cast<CompiledPipeline&>(pipeline)); // TODO: TMP: Find a better way than const_cast

	m_Out.open(codegenFileLocation);

	ExecuteContext executeContext{};
	executeContext.EditorLinks = pipeline.EditorLinks;
	executeContext.VariablePool = pipeline.VariablePool;

	WebGPUCodeGenerator codeGenerator{ m_Out };
	WebGLExecutorNodeVisitor visitor{ executeContext, codeGenerator };

	WriteImports(codeGenerator);

	codeGenerator.WriteKeyword("export");
	codeGenerator.ClassDeclaration("NodeGraphCodeGen");
	codeGenerator.BeginBlock();
	{
		GenerateVariableInitialization(codeGenerator, pipeline);

		codeGenerator.FunctionDeclaration("OnStart", "void", false, true);
		codeGenerator.FunctionArgumentsBegin();
		codeGenerator.FunctionArgumentsEnd();
		codeGenerator.BeginBlock();
		if (!executeContext.Failure) visitor.ExecuteNodes(pipeline.OnStartNode);
		codeGenerator.EndBlock();

		codeGenerator.FunctionDeclaration("OnUpdate", "void", false, true);
		codeGenerator.FunctionArgumentsBegin();
		codeGenerator.FunctionArgumentsEnd();
		codeGenerator.BeginBlock();
		if (!executeContext.Failure) visitor.ExecuteNodes(pipeline.OnUpdateNode);
		codeGenerator.EndBlock();
	}

	RegisterInputCallbacks(codeGenerator, pipeline);
	WriteInputCallbacks(codeGenerator, pipeline, visitor);

	// TODO: Mark error execution nodes

	codeGenerator.EndBlock();

	m_Out.close();

	if (executeContext.Failure)
	{
		FileUtils::DeleteDirectoryIfExists(projectDirectoryLocation);
	}
	else
	{
		FileUtils::CopyDirectory(TemplateLocation, projectDirectoryLocation);
	}

	return !executeContext.Failure;
}

void RenderPipelineCodeGenerator::WriteImports(CodeGenerator& generator)
{
	const auto writeImport = [&generator](const std::string& elements, const std::string& file) {
		generator.WriteKeyword("import");
		generator.WriteKeyword(elements);
		generator.WriteKeyword("from");
		generator.WriteConstant(file);
		generator.EndInstruction();
		};

	writeImport("{ RenderNodeAPI }", "./render-api.js");
	writeImport("{ Float2, Float3, Float4, Float4x4, DepthTest, RenderState }", "./math-types.js");

}

void RenderPipelineCodeGenerator::GenerateVariableInitialization(CodeGenerator& generator, const CompiledPipeline& pipeline)
{
	const auto fn = [&generator](VariableID id, const Variable& variable)
		{
			generator.WriteVariable(variable);
			generator.WriteOperator(CodeGenerator::Operator::Asign);
			switch (variable.Type)
			{
			case VariableType::Bool:
				generator.WriteConstant(variable.Get<bool>());
				break;
			case VariableType::Int:
				generator.WriteConstant(variable.Get<int>());
				break;
			case VariableType::Float:
				generator.WriteConstant(variable.Get<float>());
				break;
			case VariableType::Float2:
				generator.WriteConstant(variable.Get<Float2>());
				break;
			case VariableType::Float3:
				generator.WriteConstant(variable.Get<Float3>());
				break;
			case VariableType::Float4:
				generator.WriteConstant(variable.Get<Float4>());
				break;
			case VariableType::Float4x4:
				generator.WriteConstant(variable.Get<Float4x4>());
				break;
			case VariableType::Shader:
			{
				const auto& data = variable.Get<ShaderData>();
				generator.WriteKeyword("await");
				generator.FunctionCall("RenderNodeAPI.LoadShader");
				generator.FunctionArgumentsBegin();
				generator.WriteConstant(data.Path);
				generator.FunctionArgumentsEnd();
			} break;
			case VariableType::Texture:
			{
				const auto& data = variable.Get<TextureData>();
				if (data.Path.empty())
				{
					generator.WriteKeyword("await");
					generator.FunctionCall("RenderNodeAPI.CreateFramebuffer");
					generator.FunctionArgumentsBegin();
					generator.WriteConstant(data.Width);
					generator.ArgumentsSeparator();
					generator.WriteConstant(data.Height);
					generator.ArgumentsSeparator();
					generator.WriteConstant(data.Framebuffer);
					generator.ArgumentsSeparator();
					generator.WriteConstant(data.DepthStencil);
					generator.FunctionArgumentsEnd();
				}
				else
				{
					NOT_IMPLEMENTED;
				}

			} break;
			case VariableType::Scene:
			{
				const auto& data = variable.Get<SceneData>();
				generator.WriteKeyword("await");
				generator.FunctionCall("RenderNodeAPI.LoadScene");
				generator.FunctionArgumentsBegin();
				generator.WriteConstant(data.Path);
				generator.FunctionArgumentsEnd();
			} break;
			case VariableType::Invalid:
			case VariableType::Count:
				break;
			default:
				NOT_IMPLEMENTED;
				break;

			}
			generator.EndInstruction();
		};

	generator.FunctionDeclaration("InitializeVariables", "void", false, true);
	generator.FunctionArgumentsBegin();
	generator.FunctionArgumentsEnd();
	generator.BeginBlock();
	pipeline.VariablePool.ForEachVariable(fn);
	generator.EndBlock();
}

void RenderPipelineCodeGenerator::LinkResources(const std::string& projectPath, CompiledPipeline& pipeline)
{
	const std::string resourceDir = projectPath + "/" + ResourcesDirectoryName;

	FileUtils::DeleteDirectoryIfExists(resourceDir);
	FileUtils::MakeDirectory(resourceDir);
	FileUtils::MakeDirectory(resourceDir + "/Textures");
	FileUtils::MakeDirectory(resourceDir + "/Shaders");
	FileUtils::MakeDirectory(resourceDir + "/Scenes");

	const auto fn = [&resourceDir](VariableID id, Variable& variable) {
		switch (variable.Type)
		{
		case VariableType::Shader:
		{
			auto& data = variable.Get<ShaderData>();
			std::string fileName, fileExt;
			if (FileUtils::GetFileNameAndExtension(data.Path, fileName, fileExt))
			{
				const std::string newPath = resourceDir + "/Shaders/" + fileName + ".wgsl";
				FileUtils::CopyFile(data.Path, newPath);
				data.Path = newPath;
			}
		} break;
		case VariableType::Scene:
		{
			auto& data = variable.Get<SceneData>();
			std::string fileName, fileExt;
			if (FileUtils::GetFileNameAndExtension(data.Path, fileName, fileExt))
			{
				const std::string newPath = resourceDir + "/Scenes/" + fileName + fileExt;
				FileUtils::CopyFile(data.Path, newPath);
				data.Path = newPath;
			}
		} break;
		case VariableType::Texture:
		{
			auto& data = variable.Get<TextureData>();
			if (!data.Path.empty())
			{
				std::string fileName, fileExt;
				if (FileUtils::GetFileNameAndExtension(data.Path, fileName, fileExt))
				{
					const std::string newPath = resourceDir + "/Textures/" + fileName + fileExt;
					FileUtils::CopyFile(data.Path, newPath);
					data.Path = newPath;
				}
			}
		} break;
		}
		};
	pipeline.VariablePool.ForEachVariable(fn);
}

void RenderPipelineCodeGenerator::WriteInputCallbacks(CodeGenerator& generator, const CompiledPipeline& pipeline, WebGLExecutorNodeVisitor& visitor)
{
	const auto writeInputs = [&generator, &visitor](const std::unordered_map<KeyInput, ExecutorNode*>& inputs, const std::string& callbackPrefix) {
		for (const auto& it : inputs)
		{
			const KeyInput input = it.first;
			const std::string callbackFunctionName = callbackPrefix + "_" + std::to_string(std::hash<KeyInput>{}(input));

			generator.FunctionDeclaration(callbackFunctionName, "void", false, true);
			generator.FunctionArgumentsBegin();
			generator.FunctionArgumentsEnd();
			generator.BeginBlock();
			visitor.ExecuteNodes(it.second);
			generator.EndBlock();
		}
		};

	writeInputs(pipeline.OnKeyDownNodes, "OnKeyDown");
	writeInputs(pipeline.OnKeyReleasedNodes, "OnKeyReleased");
	writeInputs(pipeline.OnKeyPressedNodes, "OnKeyPressed");
}

void RenderPipelineCodeGenerator::RegisterInputCallbacks(CodeGenerator& generator, const CompiledPipeline& pipeline)
{
	const auto registerInputs = [&generator](const std::unordered_map<KeyInput, ExecutorNode*>& inputs, const std::string& registerFunctionName, const std::string& callbackPrefix) {
		for (const auto& it : inputs) 
		{
			const KeyInput input = it.first;
			const std::string callbackFunctionName = callbackPrefix + "_" + std::to_string(std::hash<KeyInput>{}(input));

			generator.FunctionCall(registerFunctionName);
			generator.FunctionArgumentsBegin();
			generator.WriteConstant(GLFWUtils::ToString(input.Key, input.Mods, GLFWUtils::StringRepresentation::Javascript));
			generator.ArgumentsSeparator();
			generator.WriteKeyword("this." + callbackFunctionName + ".bind(this)");
			generator.FunctionArgumentsEnd();
			generator.EndInstruction();
		}
		};

	generator.FunctionDeclaration("RegisterInputs", "void", false, false);
	generator.FunctionArgumentsBegin();
	generator.FunctionArgumentsEnd();
	generator.BeginBlock();

	registerInputs(pipeline.OnKeyDownNodes, "RenderNodeAPI.RegisterKeyDownEvent", "OnKeyDown");
	registerInputs(pipeline.OnKeyReleasedNodes, "RenderNodeAPI.RegisterKeyReleasedEvent", "OnKeyReleased");
	registerInputs(pipeline.OnKeyPressedNodes, "RenderNodeAPI.RegisterKeyPressedEvent", "OnKeyPressed");

	generator.EndBlock();
}