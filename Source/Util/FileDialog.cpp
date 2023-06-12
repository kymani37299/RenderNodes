#include "FileDialog.h"

#include <windows.h>
#include <nfd.h>
#include <filesystem>

static std::string GetWorkingDirectory() 
{
	if (IsDebuggerPresent())
	{
		return std::string(PROJECT_DIR);
	}
	else
	{
		CHAR buffer[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
		std::string::size_type pos = std::string(buffer).find_last_of("\\/");
		return std::string(buffer).substr(0, pos);
	}
}

static std::string GetRelativePath(const std::string& absolutePath)
{
	namespace fs = std::filesystem;

	const fs::path base{ absolutePath };
	const fs::path ref{ GetWorkingDirectory() };
	const fs::path relative = fs::relative(base, ref);
	return relative.string();
}

namespace FileDialog
{
	static bool s_Initialized = false;

	void Init()
	{
		s_Initialized = (NFD_Init() == NFD_OKAY);
	}

	void Destroy()
	{
		NFD_Quit();
	}

	bool OpenRenderNodeFile(std::string& path)
	{
		if (!s_Initialized) return false;

		nfdchar_t* outPath;
		nfdfilteritem_t filterItem[1] = { { "Render node file", "rn" } };
		nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
		if (result == NFD_OKAY)
		{
			path = std::string{ outPath };
			path = GetRelativePath(path);
			NFD_FreePath(outPath);
			return true;
		}
		return false;
	}

	bool SaveRenderNodeFile(std::string& path)
	{
		if (!s_Initialized) return false;

		nfdchar_t* outPath;
		nfdfilteritem_t filterItem[1] = { { "Render node file", "rn" } };
		nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, NULL, NULL);
		if (result == NFD_OKAY)
		{
			path = std::string{ outPath };
			path = GetRelativePath(path);
			NFD_FreePath(outPath);
			return true;
		}
		return false;
	}
}