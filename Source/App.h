#pragma once

struct GLFWwindow;

class App
{
public:
	App();
	~App();

	void Run();
private:
	int m_Width;
	int m_Height;
	GLFWwindow* m_Window;
};