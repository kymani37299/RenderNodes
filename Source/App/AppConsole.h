#pragma once

#include <string>
#include <vector>

class AppConsole
{
public:
	void Draw();
	void Clear();
	void Log(const std::string& msg);

private:
	std::vector<std::string> m_Lines;
};