#pragma once

#include <string>

class AppConsole
{
public:
	void Clear();
	void Log(const std::string& msg);

	void Draw();
private:
	std::string m_TextBuffer;
};