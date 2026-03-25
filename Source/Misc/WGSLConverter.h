#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace WGSLConverter
{
	bool Convert(const std::string& shaderPath, const std::string& baseTempPath, const std::string& baseOutputPath, std::unordered_map<std::string, int>& bindingMap, std::vector<std::string>& errorMessages);
}