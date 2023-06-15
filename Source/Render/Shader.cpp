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

static std::string GetPathWitoutFile(const std::string& path)
{
	return path.substr(0, 1 + path.find_last_of("\\/"));
}

Ptr<Shader> Shader::Compile(const std::string& path)
{
	const std::string includeRoot = GetPathWitoutFile(path);

	std::string code = "";
	GLenum type = 0;
	
	std::vector<std::string> shaderContent;

	if (!ReadFile(path, shaderContent))
	{
		return nullptr;
	}

	std::set<std::string> loadedFiles = {};

	std::vector<unsigned> shaderModules;
	for (size_t i = 0; i < shaderContent.size(); i++)
	{
		std::string& line = shaderContent[i];

		GLenum next_type = MacroToShaderType(line);
		if (next_type != 0)
		{
			if (!code.empty())
			{
				unsigned shaderModule = CompileShader(type, code.c_str());
				if (shaderModule == 0) return nullptr;
				shaderModules.push_back(shaderModule);
			}
			code = "#version 430\n";
			type = next_type;
			continue;
		}

		if (type == 0) continue; // TODO: Add common code

		if (line.find("#include") != std::string::npos)
		{
			std::string fileName = line;
			ReplaceAll(fileName, "#include", "");
			ReplaceAll(fileName, " ", "");
			ReplaceAll(fileName, "\"", "");

			if (loadedFiles.count(fileName)) continue;
			loadedFiles.insert(fileName);

			std::vector<std::string> _c;
			if (!ReadFile(includeRoot + fileName, _c))
			{
				std::cout << "[Shader compilation error] " << GetTag(type) << " " << "Failed to include " << fileName <<  " in " << path << std::endl;
			}
			shaderContent.insert((shaderContent.begin() + (i + 1)), _c.begin(), _c.end());
		}
		else
		{
			code.append(line + "\n");
		}
	}

	unsigned shaderModule = CompileShader(type, code.c_str());
	if (shaderModule == 0) return nullptr;
	shaderModules.push_back(shaderModule);

	if (shaderModules.size() == 0) return nullptr;

	Shader* shader = new Shader{};
	GL_CALL(shader->Handle = glCreateProgram());

	for (unsigned sm : shaderModules) { GL_CALL(glAttachShader(shader->Handle, sm)); }
	GL_CALL(glLinkProgram(shader->Handle));

	GLint validLinking;
	GL_CALL(glGetProgramiv(shader->Handle, GL_LINK_STATUS, (int*)&validLinking));
	GL_CALL(glValidateProgram(shader->Handle));

#ifdef _DEBUG
	for (unsigned sm : shaderModules) { GL_CALL(glDetachShader(shader->Handle, sm)); }
#else
	for (unsigned sm : shaderModules) { GL_CALL(glDeleteShader(sm)); }
#endif

	return Ptr<Shader>(shader);
}

Shader::~Shader()
{
	GL_CALL(glDeleteProgram(Handle));
}
