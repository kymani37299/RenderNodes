#pragma once

#include <string>
#include <vector>

#include "../Common.h"
#include "AppConsole.h"
#include "IInputListener.h"

class RenderPipelineEditor;
class RenderPipelineExecutor;
class CustomNodePipelineEditor;
class NodeGraphCompiler;
class NodeGraphSerializer;
class NodeGraphCommandExecutor;
class NodeGraph;
class CustomEditorNode;

struct GLFWwindow;

enum class AppMode
{
	Editor,
	CustomNode,
	Run
};

using CustomNodeList = std::vector<Ptr<CustomEditorNode>>;

class App
{
	static App* s_Instance;

public:
	static App* Get()
	{
		if (!s_Instance)
			s_Instance = new App{};
		return s_Instance;
	}

	static void Destroy()
	{
		delete s_Instance;
	}

private:
	App();
	~App();

public:
	void Run();

	void RenderMenuBar();
	void RenderFrame();
	void HandleInputAction(int key, int mods, int action);

	void AddCustomNode(CustomEditorNode* node);
	void OpenCustomNode(CustomEditorNode* node);

	CustomNodeList* GetCustomNodes() { return &m_CustomNodeList; }

	void SubscribeToInput(IInputListener* listener);
	void UnsubscribeToInput(IInputListener* lisnener);

	AppConsole& GetConsole() { return m_Console; }
private:
	void NewDocument();
	void LoadDocument();
	void SaveDocument();
	void SaveAsDocument();

	void CompileAndRun();

	std::string GetWindowTitle();

private:

	// Window
	int m_Width;
	int m_Height;
	GLFWwindow* m_Window;
	std::string m_WindowTitle = "Render Nodes";

	// Save/Load
	std::string m_CurrentLoadedFile = "";
	unsigned m_LastExecutedCommandCount = 0;

	// Input
	std::vector<IInputListener*> m_InputListeners;

	AppMode m_Mode = AppMode::Editor;
	
	// Workers
	Ptr<NodeGraphCompiler> m_Compiler;
	Ptr<RenderPipelineExecutor> m_Executor;
	Ptr<NodeGraphSerializer> m_Serializer;

	Ptr<RenderPipelineEditor> m_Editor;
	Ptr<NodeGraph> m_NodeGraph;

	Ptr<CustomNodePipelineEditor> m_CustomNodeEditor;

	CustomNodeList m_CustomNodeList;

	AppConsole m_Console;
};