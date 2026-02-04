#include "FileDialog.h"

#include <windows.h>
#include <nfd.h>
#include <filesystem>

static std::string GetWorkingDirectory() 
{
	CHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
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

	bool OpenFile(std::string& path, const std::vector<nfdfilteritem_t>& filters)
	{
		if (!s_Initialized) return false;

		nfdchar_t* outPath;
		nfdresult_t result = NFD_OpenDialog(&outPath, filters.data(), filters.size(), NULL);
		if (result == NFD_OKAY)
		{
			path = std::string{ outPath };
			path = GetRelativePath(path);
			NFD_FreePath(outPath);
			return true;
		}
		return false;
	}

	bool SaveFile(std::string& path, const std::vector<nfdfilteritem_t>& filters)
	{
		if (!s_Initialized) return false;

		nfdchar_t* outPath;
		nfdresult_t result = NFD_SaveDialog(&outPath, filters.data(), filters.size(), NULL, NULL);
		if (result == NFD_OKAY)
		{
			path = std::string{ outPath };
			path = GetRelativePath(path);
			NFD_FreePath(outPath);
			return true;
		}
		return false;
	}

	bool OpenRenderNodeFile(std::string& path)
	{
		return OpenFile(path, { {{ "Render node file", "rn" }} });
	}

	bool SaveRenderNodeFile(std::string& path)
	{
		return SaveFile(path, { { "Render node file", "rn" } });
	}

	bool OpenTextureFile(std::string& path)
	{
		return OpenFile(path, { { "Texture file", "jpg,jpeg,png,hdr,bmp,gif,psd,pic,pnm,tga" } });
	}

	bool OpenShaderFile(std::string& path)
	{
		return OpenFile(path, { { "GLSL shader", "glsl"} });
	}

	bool OpenSceneFile(std::string& path)
	{
		return OpenFile(path, { { "GLTF scene", "gltf"} });
	}

}