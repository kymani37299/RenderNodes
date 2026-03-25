#include "WGSLConverter.h"
#include <regex>
#include <algorithm>
#include "../Render/Shader.h"
#include "../NodeGraph/VariablePool.h"
#include "../Util/FileUtils.h"

namespace WGSLConverter
{
	namespace
	{
		std::string NagaCliPath = "./Tools/naga-cli/naga";
		uint32_t StaticSamplerIndex = 16; // Must be in sync with RenderContext.SAMPLER_BINDING_INDEX in js template
		uint32_t AutoBindingStartIndex = 17;

		std::string StripComments(const std::string& code)
		{
			std::string out;
			out.reserve(code.size());
			size_t i = 0;
			while (i < code.size())
			{
				if (i + 1 < code.size() && code[i] == '/' && code[i + 1] == '/')
				{
					while (i < code.size() && code[i] != '\n') ++i;
				}
				else if (i + 1 < code.size() && code[i] == '/' && code[i + 1] == '*')
				{
					i += 2;
					while (i + 1 < code.size() && !(code[i] == '*' && code[i + 1] == '/')) ++i;
					i += 2;
				}
				else
				{
					out += code[i++];
				}
			}
			return out;
		}

		std::string ParseUniformName(const std::string& rest)
		{
			auto brace = rest.find('{');
			if (brace != std::string::npos)
			{
				auto close = rest.rfind('}');
				if (close != std::string::npos)
				{
					std::string after = rest.substr(close + 1);
					std::regex nameRe(R"(\s*(\w+)\s*;)");
					std::smatch m;
					if (std::regex_search(after, m, nameRe)) return m[1].str();
					std::string before = rest.substr(0, brace);
					if (std::regex_search(before, m, nameRe)) return m[1].str();
				}
			}
			std::regex plainRe(R"(\b(\w+)\s*(?:\[\w*\])?\s*;?\s*$)");
			std::smatch m;
			if (std::regex_search(rest, m, plainRe)) return m[1].str();
			return "";
		}

		std::string ParseIOName(const std::string& rest, const std::string& keyword)
		{
			std::regex ioRe(R"(\b)" + keyword + R"(\s+\w+\s+(\w+)\s*;)");
			std::smatch m;
			if (std::regex_search(rest, m, ioRe)) return m[1].str();
			return "";
		}

		struct UniformEdit
		{
			size_t      Start;
			size_t      Len;
			std::string Replacement;
			std::string Name;
		};

		std::string AssignUniformBindings(const std::string& code, std::unordered_map<std::string, int>& outBindings)
		{
			std::string result = code;
			int nextBinding = AutoBindingStartIndex;
			
			std::vector<std::string> statements;
			std::string current;
			for (char c : code)
			{
				current += c;
				if (c == ';' || c == '\n')
				{
					if (!current.empty())
					{
						statements.push_back(current);
						current.clear();
					}
				}
			}
			if (!current.empty()) statements.push_back(current);

			for (auto& stmt : statements)
			{
				if (stmt.find("uniform") != std::string::npos)
				{
					bool hasBinding = false;
					int bindingVal = -1;
					std::regex bindRe(R"(binding\s*[=\s(]\s*(\d+))");
					std::smatch bm;
					if (std::regex_search(stmt, bm, bindRe))
					{
						hasBinding = true;
						bindingVal = std::stoi(bm[1].str());
					}

					std::string uName = ParseUniformName(stmt);
					if (!uName.empty())
					{
						int assignedBinding;
						if (hasBinding)
						{
							assignedBinding = bindingVal;
							if (assignedBinding >= nextBinding)
								nextBinding = assignedBinding + 1;
						}
						else
						{
							assignedBinding = nextBinding++;
						}
						outBindings[uName] = assignedBinding;

						if (!hasBinding)
						{
							size_t uPos = stmt.find("uniform");
							std::string insert = "layout(binding = " + std::to_string(assignedBinding) + ") ";
							stmt.insert(uPos, insert);
						}
					}
				}
			}

			result.clear();
			for (const auto& stmt : statements)
			{
				result += stmt;
			}
			return result;
		}

		std::string ConvertSamplers(const std::string& code)
		{
			std::string result = code;

			std::vector<std::string> samplerNames;
			{
				std::regex s2dRe(R"(uniform\s+sampler2D\s+(\w+)\s*;)");
				std::string stripped = StripComments(result);
				auto it = std::sregex_iterator(stripped.begin(), stripped.end(), s2dRe);
				auto end = std::sregex_iterator();
				for (; it != end; ++it)
					samplerNames.push_back((*it)[1].str());
			}

			result = std::regex_replace(result,
				std::regex(R"(\buniform\s+sampler2D\b)"), "uniform texture2D");

			bool staticSamplerAdded = false;
			for (const auto& name : samplerNames)
			{
				if (!staticSamplerAdded)
				{
					std::regex texDecl(R"(uniform\s+texture2D\s+)" + name + R"(\s*;)");
					std::smatch m;
					if (std::regex_search(result, m, texDecl))
					{
						result.insert(m.position() + m.length(),
							"\nlayout(binding = 16) uniform sampler _staticSampler;");
						staticSamplerAdded = true;
					}
				}
			}

			for (const auto& name : samplerNames)
			{
				const std::string replacement = "texture(sampler2D(" + name + ", _staticSampler),";
				// GLSL 1.20 style: texture2D(name, uv)
				result = std::regex_replace(result,
					std::regex(R"(\btexture2D\s*\(\s*)" + name + R"(\s*,)"), replacement);
				// GLSL 1.30+ style: texture(name, uv)
				result = std::regex_replace(result,
					std::regex(R"(\btexture\s*\(\s*)" + name + R"(\s*,)"), replacement);
			}

			return result;
		}

		std::string PreprocessForNaga(const std::string& code, std::unordered_map<std::string, int>& outBindings)
		{
			std::string result = AssignUniformBindings(code, outBindings);
			result = ConvertSamplers(result);
			return result;
		}

		bool RunCommand(const std::string& cmd, std::vector<std::string>& output)
		{
			FILE* pipe = _popen((cmd + " 2>&1").c_str(), "r");
			if (!pipe)
			{
				output.push_back("Failed to execute command.");
				return false;
			}
			char buffer[256];
			while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
			{
				std::string line = std::string{ buffer };
				if (!line.empty() && line.back() == '\n')
					line.pop_back();
				output.push_back(line);
			}
			int exitCode = _pclose(pipe);
			return exitCode == 0;
		}

		std::string Stringify(const std::string& input)
		{
			return "\"" + input + "\"";
		}
	}

	bool Convert(const std::string& shaderPath, const std::string& baseTempPath, const std::string& baseOutputPath, std::unordered_map<std::string, int>& bindingMap, std::vector<std::string>& errorMessages)
	{
		std::string shaderCode, shaderVersion;
		if (!Shader::ReadShaderFile(shaderPath, shaderCode, shaderVersion))
		{
			errorMessages.push_back("Error while reading file: " + shaderPath);
			return false;
		}

		if (std::stoi(shaderVersion) < 450)
		{
			shaderVersion = "450";
		}
		
		const std::string preprocessedCode = PreprocessForNaga(shaderCode, bindingMap);

		std::vector<ShaderStage> stages = { ShaderStage::Vertex, ShaderStage::Fragment };
		for (ShaderStage stage : stages)
		{
			const std::string tmpPath = baseTempPath + Shader::GetShaderStageExtension(stage) + ".glsl";
			const std::string convertedPath = baseOutputPath + Shader::GetShaderStageExtension(stage) + ".wgsl";

			const std::string finalizedCode = Shader::FinalizeShaderCode(stage, shaderVersion, preprocessedCode);

			std::ofstream outStream;
			outStream.open(tmpPath);
			outStream << finalizedCode;
			outStream.close();

			bool commandSuccess = RunCommand(Stringify(Stringify(NagaCliPath) + " " + Stringify(tmpPath) + " " + Stringify(convertedPath)), errorMessages);

			FileUtils::DeleteFile(tmpPath);

			if (!commandSuccess)
			{
				errorMessages.push_back("Error while compiling shader: " + shaderPath);
				return false;
			}
		}
		return true;
	}
}