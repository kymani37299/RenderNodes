#include "AppConsole.h"

#include <regex>
#include <sstream>
#include <unordered_map>

#include "../Common.h"

namespace
{
	static const std::unordered_map<std::string, std::string> s_TagToAnsi = {
	{ "red",     "\x1B[31m" },
	{ "green",   "\x1B[32m" },
	{ "yellow",  "\x1B[33m" },
	{ "blue",    "\x1B[34m" },
	{ "magenta", "\x1B[35m" },
	{ "cyan",    "\x1B[36m" },
	{ "white",   "\x1B[37m" },
	{ "bold",    "\x1B[1m"  },
	{ "dim",     "\x1B[2m"  },
	};

	static ImVec4 AnsiCodeToColor(int code)
	{
		switch (code)
		{
		case 30: return ImVec4(0.2f, 0.2f, 0.2f, 1.0f); // Black
		case 31: return ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red
		case 32: return ImVec4(0.3f, 1.0f, 0.3f, 1.0f); // Green
		case 33: return ImVec4(1.0f, 1.0f, 0.3f, 1.0f); // Yellow
		case 34: return ImVec4(0.3f, 0.5f, 1.0f, 1.0f); // Blue
		case 35: return ImVec4(1.0f, 0.3f, 1.0f, 1.0f); // Magenta
		case 36: return ImVec4(0.3f, 1.0f, 1.0f, 1.0f); // Cyan
		case 37: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
		default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Reset/unknown
		}
	}

	static std::string ProcessTags(const std::string& input)
	{
		std::string result = input;
		for (const auto& [tag, ansi] : s_TagToAnsi)
		{
			std::string openTag = "<" + tag + ">";
			size_t pos;
			while ((pos = result.find(openTag)) != std::string::npos)
				result.replace(pos, openTag.size(), ansi);

			std::string closeTag = "</" + tag + ">";
			while ((pos = result.find(closeTag)) != std::string::npos)
				result.replace(pos, closeTag.size(), "\x1B[0m");
		}
		return result;
	}

	static std::string StripAnsiCodes(const std::string& input)
	{
		return std::regex_replace(input, std::regex("\x1B\\[[0-9;]*[a-zA-Z]"), "");
	}
}

struct ConsoleSegment
{
	std::string text;
	ImVec4 color;
};

static std::vector<ConsoleSegment> ParseAnsiString(const std::string& input)
{
	std::vector<ConsoleSegment> segments;
	ImVec4 currentColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	std::regex ansiRegex("\x1B\\[([0-9;]*)([a-zA-Z])");

	auto it = std::sregex_iterator(input.begin(), input.end(), ansiRegex);
	auto end = std::sregex_iterator();
	size_t lastPos = 0;

	for (auto match = it; match != end; ++match)
	{
		size_t matchPos = match->position();
		if (matchPos > lastPos)
			segments.push_back({ input.substr(lastPos, matchPos - lastPos), currentColor });

		if ((*match)[2].str() == "m")
		{
			const std::string params = (*match)[1].str();
			if (params.empty() || params == "0")
			{
				currentColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else
			{
				std::stringstream ss(params);
				std::string token;
				while (std::getline(ss, token, ';'))
				{
					int code = std::stoi(token);
					if (code >= 30 && code <= 37)
						currentColor = AnsiCodeToColor(code);
					else if (code == 0)
						currentColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
		}

		lastPos = matchPos + match->length();
	}

	if (lastPos < input.size())
		segments.push_back({ input.substr(lastPos), currentColor });

	return segments;
}

void AppConsole::Draw()
{
	ImGui::Begin("Console");
	if (ImGui::Button("Clear")) Clear();
	ImGui::SameLine();
	if (ImGui::Button("Copy"))
	{
		std::string plainText;
		for (const std::string& line : m_Lines)
			plainText += StripAnsiCodes(line) + "\n";
		ImGui::SetClipboardText(plainText.c_str());
	}
	ImGui::Separator();

	ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	for (const std::string& line : m_Lines)
	{
		auto segments = ParseAnsiString(line);
		for (size_t i = 0; i < segments.size(); i++)
		{
			if (i > 0) ImGui::SameLine(0.0f, 0.0f);
			ImGui::TextColored(segments[i].color, "%s", segments[i].text.c_str());
		}
	}

	ImGui::EndChild();
	ImGui::End();
}

void AppConsole::Clear()
{
	m_Lines.clear();
}

void AppConsole::Log(const std::string& msg)
{
	m_Lines.push_back(ProcessTags(msg));
}