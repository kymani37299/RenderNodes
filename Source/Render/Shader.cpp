#include "Shader.h"

#include <vector>
#include <fstream>
#include <string>
#include <set>

#include "../App/App.h"
#include "../Util/FileUtils.h"

std::string GetShaderDefine(ShaderStage stage)
{
	switch(stage)
	{
	case ShaderStage::Vertex: return "VERTEX";
	case ShaderStage::Fragment: return "FRAGMENT";
	default:
		NOT_IMPLEMENTED;
	}
	return "UNKOWN";
}

unsigned GetShaderGLEnum(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex: return GL_VERTEX_SHADER;
	case ShaderStage::Fragment: return GL_FRAGMENT_SHADER;
	default:
		NOT_IMPLEMENTED;
	}
	return 0;
}

std::string GetTag(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex: return "[VS]";
	case ShaderStage::Fragment: return "[FS]";
	default:
		NOT_IMPLEMENTED;
	}
	return "[Unknown shader type]";
}

// TODO: Make it threadsafe
static std::vector<std::string> s_Errors;

static void LogErrors(const std::string& path)
{
	App::Get()->GetConsole().Log("<red>----------------[Shader compiler errors]------------------</red> ");
	App::Get()->GetConsole().Log("File: " + path);
	for (const auto& err : s_Errors)
	{
		App::Get()->GetConsole().Log(err);
	}
	App::Get()->GetConsole().Log("<red>----------------------------------------------------------</red> ");
}

static unsigned CompileShader(ShaderStage stage, const char* source)
{
	unsigned type = GetShaderGLEnum(stage);
	GL_CALL(unsigned id = glCreateShader(type));
	GL_CALL(glShaderSource(id, 1, &source, nullptr));
	GL_CALL(glCompileShader(id));

	int result;
	GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (!result)
	{
		int length;
		GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GL_CALL(glGetShaderInfoLog(id, length, &length, message));
		s_Errors.push_back(GetTag(stage) + " " + message);
		GL_CALL(glDeleteShader(id));
		return 0;
	}

	return id;
}

static void ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

static void DecomposePath(const std::string& path, std::string& pathRoot, std::string& fileName)
{
	pathRoot = path.substr(0, 1 + path.find_last_of("\\/"));
	fileName = path.substr(path.find_last_of("/\\") + 1);
}

bool ReadShaderFile(std::string& outputCode, std::string& shaderVersion, const std::string& includeRoot, const std::string includePath)
{
	std::vector<std::string> shaderContent;
	if (!FileUtils::ReadFile(includeRoot + includePath, shaderContent))
	{
		s_Errors.push_back("Failed to load shader file: " + includeRoot + includePath);
		return false;
	}

	for (const std::string& line : shaderContent)
	{
		// Include directive
		if (line.find("#include") != std::string::npos)
		{
			// TODO: Rework finding file name in path
			std::string fileName = line;
			ReplaceAll(fileName, "#include", "");
			ReplaceAll(fileName, " ", "");
			ReplaceAll(fileName, "\"", "");

			if (ReadShaderFile(outputCode, shaderVersion, includeRoot, fileName))
				return false;
		}
		else if (line.find("#version") != std::string::npos)
		{
			shaderVersion = line;
			ReplaceAll(shaderVersion, "#version", "");
			ReplaceAll(shaderVersion, " ", "");
		}
		else
		{
			outputCode.append(line + "\n");
		}
	}
	return true;
}

std::string Shader::GetShaderStageExtension(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex: return ".vert";
	case ShaderStage::Fragment: return ".frag";
	default:
		NOT_IMPLEMENTED;
	}
	return "";
}

std::string Shader::FinalizeShaderCode(ShaderStage stage, const std::string& shaderVersion, const std::string& shaderCode)
{
	return "#version " + shaderVersion + "\n#define " + GetShaderDefine(stage) + "\n" + shaderCode;
}

bool Shader::ReadShaderFile(const std::string& path, std::string& outputCode, std::string& shaderVersion)
{
	std::string includeRoot, shaderFile;
	DecomposePath(path, includeRoot, shaderFile);
	return ::ReadShaderFile(outputCode, shaderVersion, includeRoot, shaderFile);
}

Ptr<Shader> Shader::Compile(const std::string& path, const std::vector<ShaderStage>& stages)
{
	std::vector<std::string> errors;

	std::string shaderCode, shaderVersion;
	if (!Shader::ReadShaderFile(path, shaderCode, shaderVersion))
	{
		errors.push_back("Failed to compile shader!");
		s_Errors.push_back("Failed to compile shader!");
		LogErrors(path);
		return nullptr;
	}

	std::vector<unsigned> shaderModules{};
	for (ShaderStage stage : stages)
	{
		const std::string moduleCode = FinalizeShaderCode(stage, shaderVersion, shaderCode);
		unsigned module = CompileShader(stage, moduleCode.c_str());
		if (!module)
		{
			s_Errors.push_back("Failed to compile shader!");
			LogErrors(path);
			return nullptr;
		}
		shaderModules.push_back(module);
	}

	Shader* shader = new Shader{};
	GL_CALL(shader->Handle = glCreateProgram());
	for (unsigned module : shaderModules)
	{
		GL_CALL(glAttachShader(shader->Handle, module));
	}
	GL_CALL(glLinkProgram(shader->Handle));

	GLint validLinking;
	GL_CALL(glGetProgramiv(shader->Handle, GL_LINK_STATUS, (int*)&validLinking));
	if (!validLinking)
	{
		s_Errors.push_back("Failed to link shader!");
		LogErrors(path);
		return nullptr;
	}

	GLint validShader;
	GL_CALL(glValidateProgram(shader->Handle));
	GL_CALL(glGetProgramiv(shader->Handle, GL_VALIDATE_STATUS, (int*)&validShader));
	if (!validShader)
	{
		s_Errors.push_back("Shader is not valid!");
		LogErrors(path);
		return nullptr;
	}

	for (unsigned module : shaderModules)
	{
#ifdef _DEBUG
		GL_CALL(glDetachShader(shader->Handle, module));
#else
		GL_CALL(glDeleteShader(module));
#endif
	}

	return Ptr<Shader>{shader};
}

Shader::~Shader()
{
	GL_CALL(glDeleteProgram(Handle));
}
