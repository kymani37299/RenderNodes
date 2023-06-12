#pragma once

#include <string>

namespace FileDialog
{
	void Init();
	void Destroy();

	bool OpenRenderNodeFile(std::string& path);
	bool SaveRenderNodeFile(std::string& path);
}