#pragma once

#include <string>

namespace FileDialog
{
	void Init();
	void Destroy();

	bool OpenRenderNodeFile(std::string& path);
	bool OpenTextureFile(std::string& path);
	bool OpenShaderFile(std::string& path);

	bool SaveRenderNodeFile(std::string& path);
}