#include "Shader.h"

#include <vector>
#include <fstream>
#include <string>
#include <set>

static bool ReadFile(const std::string& path, std::vector<std::string>& content)
{
	std::ifstream fileStream(path, std::ios::in);

	if (!fileStream.is_open()) {
		return false;
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.push_back(line);
	}

	fileStream.close();
	return true;
}

static GLenum MacroToShaderType(const std::string macro)
{
	if (macro.find("#start VERTEX") != std::string::npos) return GL_VERTEX_SHADER;
	if (macro.find("#start FRAGMENT") != std::string::npos) return GL_FRAGMENT_SHADER;
	if (macro.find("#start COMPUTE") != std::string::npos) return GL_COMPUTE_SHADER;
	if (macro.find("#start GEOMETRY") != std::string::npos) return GL_GEOMETRY_SHADER;
	return 0;
}

std::string GetTag(GLenum type)
{
	switch (type)
	{
	case GL_VERTEX_SHADER:
		return "[VS]";
	case GL_FRAGMENT_SHADER:
		return "[FS]";
	case GL_COMPUTE_SHADER:
	 	return "[CS]";
	case GL_GEOMETRY_SHADER:
		return "[GS]";
	default:
		return "[Unknown shader type]";
	}
}

static unsigned CompileShader(unsigned type, const char* source)
{
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
		std::cout << "[Shader compilation error] " << GetTag(type) << " " << message << std::endl;
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

static bool ReadShaderFile(std::string& outputCode, const std::string& includeRoot, const std::string includePath)
{
	std::vector<std::string> shaderContent;
	if (!ReadFile(includeRoot + includePath, shaderContent))
	{
		std::cout << "[Shader compiler] Failed to load shader file: " << includeRoot << includePath << std::endl;
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

			if (!ReadShaderFile(outputCode, includeRoot, fileName))
				return false;
		}
		else if (line.find("#version") != std::string::npos) // TODO: Read version from here
		{
			continue;
		}
		else
		{
			outputCode.append(line + "\n");
		}
	}
	return true;
}

Ptr<Shader> Shader::Compile(const std::string& path)
{
	std::string includeRoot, shaderFile;
	DecomposePath(path, includeRoot, shaderFile);

	std::string shaderCode;
	if (!ReadShaderFile(shaderCode, includeRoot, shaderFile))
	{
		std::cout << "[Shader compiler error] failed to compile shader" << std::endl;
		return nullptr;
	}

	const std::string vertexCode = "#version 430\n#define VERTEX\n" + shaderCode;
	unsigned vsModule = CompileShader(GL_VERTEX_SHADER, vertexCode.c_str());

	const std::string fragmentCode = "#version 430\n#define FRAGMENT\n" + shaderCode;
	unsigned fsModule = CompileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str());

	if (!vsModule || !fsModule)
	{
		std::cout << "[Shader compiler error] failed to compile shader" << std::endl;
		return nullptr;
	}

	Shader* shader = new Shader{};
	GL_CALL(shader->Handle = glCreateProgram());
	GL_CALL(glAttachShader(shader->Handle, vsModule));
	GL_CALL(glAttachShader(shader->Handle, fsModule));
	GL_CALL(glLinkProgram(shader->Handle));

	GLint validLinking;
	GL_CALL(glGetProgramiv(shader->Handle, GL_LINK_STATUS, (int*)&validLinking));
	if (!validLinking)
	{
		std::cout << "[Shader compiler error] failed to link shader" << std::endl;
		return nullptr;
	}

	GLint validShader;
	GL_CALL(glValidateProgram(shader->Handle));
	GL_CALL(glGetProgramiv(shader->Handle, GL_VALIDATE_STATUS, (int*)&validShader));
	if (!validShader)
	{
		std::cout << "[Shader compiler error] Shader not valid" << std::endl;
		return nullptr;
	}

#ifdef _DEBUG
	GL_CALL(glDetachShader(shader->Handle, vsModule));
	GL_CALL(glDetachShader(shader->Handle, fsModule));
#else
	GL_CALL(glDeleteShader(vsModule));
	GL_CALL(glDeleteShader(fsModule));
#endif

	return Ptr<Shader>{shader};
}

Shader::~Shader()
{
	GL_CALL(glDeleteProgram(Handle));
}
