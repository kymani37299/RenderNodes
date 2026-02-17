#include "FileDialog.h"

#include <windows.h>
#include <nfd.h>
#include <filesystem>

#include "FileUtils.h"

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
			path = FileUtils::GetRelativePath(path);
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
			path = FileUtils::GetRelativePath(path);
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

	bool SaveJavascriptFile(std::string& path)
	{
		return SaveFile(path, { { "Javascript file", "js" } });
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