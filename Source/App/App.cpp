#include "App.h"

#include "../Common.h"
#include "../IDGen.h"

#include <filesystem>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "../Editor/EditorErrorHandler.h"
#include "../Editor/RenderPipelineEditor.h"
#include "../Editor/Drawing/EditorWidgets.h"
#include "../Execution/RenderPipelineExecutor.h"
#include "../NodeGraph/NodeGraphCompiler.h"
#include "../NodeGraph/NodeGraphSerializer.h"
#include "../NodeGraph/NodeGraphCommands.h"

#include "../Util/FileDialog.h"

// From IDGen.h
std::atomic<UniqueID> IDGen::Allocator = 1;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	App::Get()->HandleInputAction(key, mods, action);
}

App* App::s_Instance = nullptr;

App::App()
{
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Determine window size
    static constexpr bool useFullscreen = false;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    m_Width = 1024;
    m_Height = 768;
    if (const GLFWvidmode* videoMode = glfwGetVideoMode(monitor))
    {
        m_Width = videoMode->width;
        m_Height = videoMode->height;
    }
    if (!useFullscreen)
    {
        m_Width = static_cast<int>(m_Width * 0.8f);
        m_Height = static_cast<int>(m_Height * 0.8f);
    }
    
    // Create window
    m_Window = glfwCreateWindow(m_Width, m_Height, "Render Nodes", useFullscreen ? monitor : NULL, NULL);
    ASSERT_M(m_Window, "Failed to create window!");
    if (m_Window == NULL)
    {
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    // Init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        ASSERT_M(0, "Failed to initialize GLAD");
        return;
    }
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
	glfwSetKeyCallback(m_Window, key_callback);

    // Init imgui
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	ImGui::GetIO().FontGlobalScale = m_Width / 1920.0f;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Init file dialog
    FileDialog::Init();

    // Init render nodes objects
    m_Editor = Ptr<RenderPipelineEditor>(new RenderPipelineEditor{ new NodeGraphCommandExecutor{} });
    m_Executor = Ptr<RenderPipelineExecutor>(new RenderPipelineExecutor{});
    m_Compiler = Ptr<NodeGraphCompiler>(new NodeGraphCompiler{});
    m_Serializer = Ptr<NodeGraphSerializer>(new NodeGraphSerializer{});
	m_ErrorHandler = Ptr<EditorErrorHandler>(new EditorErrorHandler{});

	NewDocument();
}

App::~App()
{
    FileDialog::Destroy();

    // Deinit imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Deinit glfw
    glfwTerminate();
}

inline std::string GetPathWithoutExtension(const std::string path)
{
    size_t i = path.rfind('.', path.length());
    if (i != std::string::npos)
    {
        return path.substr(0, i);
    }
    return "";
}

void App::Run()
{
    // Render loop
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwSwapBuffers(m_Window);
        glfwPollEvents();

		ProcessRequests();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		const std::string windowTitle = GetWindowTitle();
		if (windowTitle != m_WindowTitle)
		{
			m_WindowTitle = windowTitle;
			glfwSetWindowTitle(m_Window, m_WindowTitle.c_str());
		}

		ImGui::NewFrame();

		RenderMenuBar();

		const ImVec2 menuBarOffset = ImVec2{ 0.0f, ImGui::ConstantSize(20.0f) };

		ImGui::SetNextWindowPos(menuBarOffset);
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize - menuBarOffset);
		ImGui::Begin("Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
		
		RenderFrame();

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void App::RenderMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")) NewDocument();
			if (ImGui::MenuItem("Load...")) LoadDocument();
			if (ImGui::MenuItem("Save...", "Ctrl + S")) SaveDocument();
			if (ImGui::MenuItem("Save As...")) SaveAsDocument();
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Actions"))
		{
			if (ImGui::MenuItem("Undo", "Ctrl + Z"))
			{
				if(m_Mode == AppMode::Editor) m_Editor->GetCommandExecutor()->UndoCommand();
				if(m_Mode == AppMode::CustomNode) m_CustomNodeEditor->GetCommandExecutor()->UndoCommand();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Nodes"))
		{
			if (ImGui::MenuItem("Add custom node"))
			{
				std::filesystem::remove("CustomNodeEditor.json");
				AddRequest(AppRequest::ChangeModeRequest(AppMode::CustomNode, ""));
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void App::RenderFrame()
{
	if (m_Mode == AppMode::Editor)
	{
		ImGui::SetNextItemWidth(ImGui::ConstantSize(250.0f));
		if (ImGui::Button("   Run   ")) CompileAndRun();
		ImGui::Separator();

		m_Editor->Render();
	}
	else if (m_Mode == AppMode::Run)
	{
		ImGui::SetNextItemWidth(ImGui::ConstantSize(250.0f));
		if (ImGui::Button("   Stop   ")) 
			AddRequest(AppRequest::ChangeModeRequest(AppMode::Editor));
		ImGui::Separator();

		const float dt = 1000.0f / ImGui::GetIO().Framerate;
		m_Executor->OnUpdate(dt);
		m_Executor->Render();
	}
	else if (m_Mode == AppMode::CustomNode)
	{
		ASSERT(m_CustomNodeEditor);

		m_CustomNodeEditor->LoadPositionsIfNeeded();

		ImGui::BeginHorizontal("Custom node actions");

		EditorWidgets::InputText("Name", m_CustomNodeEditor->GetNameRef(), m_CustomNodeEditor->EditMode());

		ImGui::SetNextItemWidth(ImGui::ConstantSize(250.0f));

		const bool canBeCreated = m_CustomNodeEditor->CanCreateNode();
		const bool editNode = m_CustomNodeEditor->EditMode();
		if (!canBeCreated && !editNode)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::Button("   Finish    "))
		{
			if (!editNode) AddCustomNode(m_CustomNodeEditor->CreateNode());
			else m_CustomNodeEditor->RewriteNode();

			AddRequest(AppRequest::ChangeModeRequest(AppMode::Editor));
			
			m_NodeGraph->RefreshCustomNodes();
		}

		if (!canBeCreated && !editNode)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (ImGui::Button("   Abort    "))
			AddRequest(AppRequest::ChangeModeRequest(AppMode::Editor));

		ImGui::EndHorizontal();

		ImGui::Separator();

		if(m_CustomNodeEditor) m_CustomNodeEditor->Render();
	}
	else
	{
		NOT_IMPLEMENTED;
	}

	m_Console.Draw();
}

void App::HandleInputAction(int key, int mods, int action)
{
	// TODO: Use IInputListeners
	if (action == GLFW_PRESS)
	{
		if (m_Mode == AppMode::Editor)
		{
			m_Editor->HandleKeyPressed(key, mods);

			if ((mods & GLFW_MOD_CONTROL) && key == GLFW_KEY_S)
				SaveDocument();
		}
		else if (m_Mode == AppMode::CustomNode)
		{
			m_CustomNodeEditor->HandleKeyPressed(key, mods);
		}
		else if (m_Mode == AppMode::Run)
		{
			m_Executor->HandleKeyPressed(key, mods);
		}
	}

	if (action == GLFW_RELEASE)
	{
		m_Executor->HandleKeyReleased(key, mods);
	}
	
	if (action == GLFW_PRESS)
	{
		for (IInputListener* listener : m_InputListeners)
			listener->OnKeyPressed(key, mods);
	}

	if (action == GLFW_RELEASE)
	{
		for (IInputListener* listener : m_InputListeners)
			listener->OnKeyReleased(key, mods);
	}
}

void App::AddCustomNode(CustomEditorNode* node)
{
	m_CustomNodeList.push_back(Ptr<CustomEditorNode>(node));
}

void App::OpenCustomNode(const std::string& name)
{
	AddRequest(AppRequest::ChangeModeRequest(AppMode::CustomNode, name));
}

void App::SubscribeToInput(IInputListener* listener)
{
	m_InputListeners.push_back(listener);
}

void App::UnsubscribeToInput(IInputListener* lisnener)
{
	uint32_t listenerIndex = -1;
	for (uint32_t i = 0; i < m_InputListeners.size(); i++)
	{
		if (lisnener == m_InputListeners[i])
		{
			listenerIndex = i;
			break;
		}
	}

	ASSERT(listenerIndex != -1);
	if (listenerIndex != -1)
	{
		m_InputListeners.erase(m_InputListeners.begin() + listenerIndex);
	}
}

void App::NewDocument()
{
	std::filesystem::remove("NodeEditor.json");

	IDGen::Init(1);
	m_NodeGraph = Ptr<NodeGraph>(NodeGraph::CreateDefaultNodeGraph());
	m_Editor->Load(m_NodeGraph.get());
	m_Editor->InitializeDefaultNodePositions();
	
	m_CurrentLoadedFile = "";
	m_LastExecutedCommandCount = 0;
	AddRequest(AppRequest::ChangeModeRequest(AppMode::Editor));

	m_CustomNodeList.clear();
}

void App::LoadDocument()
{
	std::string path;
	if (FileDialog::OpenRenderNodeFile(path))
	{
		NodeGraph* loadedNodeGraph = new NodeGraph{};
		std::vector<CustomEditorNode*> customNodes{};
		const UniqueID firstID = m_Serializer->Deserialize(path, *loadedNodeGraph, customNodes);
		IDGen::Init(firstID);

		if (firstID)
		{
			m_Editor->Unload();

			m_CustomNodeList.clear();
			for (const auto& customNode : customNodes) AddCustomNode(customNode);

			const std::string nodeEditorConfigPath = GetPathWithoutExtension(path) + ".json";
			std::filesystem::copy(nodeEditorConfigPath, "NodeEditor.json", std::filesystem::copy_options::overwrite_existing);
			m_Editor->Load(loadedNodeGraph);
			m_CurrentLoadedFile = path;
			m_NodeGraph = Ptr<NodeGraph>(loadedNodeGraph);
			m_LastExecutedCommandCount = 0;
			AddRequest(AppRequest::ChangeModeRequest(AppMode::Editor));
		}
		else
		{
			delete loadedNodeGraph;
		}
	}
}

void App::SaveDocument()
{
	bool hasPath = false;
	std::string path = "";
	if (!m_CurrentLoadedFile.empty())
	{
		hasPath = true;
		path = m_CurrentLoadedFile;
	}
	else
	{
		hasPath = FileDialog::SaveRenderNodeFile(path);
	}

	if (hasPath)
	{
		m_Serializer->Serialize(path, *m_NodeGraph);

		const std::string jsonDest = GetPathWithoutExtension(path) + ".json";
		std::filesystem::copy("NodeEditor.json", jsonDest, std::filesystem::copy_options::overwrite_existing);

		m_CurrentLoadedFile = path;
		m_LastExecutedCommandCount = m_Editor->GetCommandExecutor()->GetExecutedCommandCount();
	}
}

void App::SaveAsDocument()
{
	std::string path;
	if (FileDialog::SaveRenderNodeFile(path))
	{
		m_Serializer->Serialize(path, *m_NodeGraph);

		const std::string jsonDest = GetPathWithoutExtension(path) + ".json";
		std::filesystem::copy("NodeEditor.json", jsonDest, std::filesystem::copy_options::overwrite_existing);
		m_CurrentLoadedFile = path;
		m_LastExecutedCommandCount = m_Editor->GetCommandExecutor()->GetExecutedCommandCount();
	}
}

void App::CompileAndRun()
{
	m_ErrorHandler->Clear();

	CompiledPipeline pipeline = m_Compiler->Compile(*m_NodeGraph);
	bool compilationSuccessful = m_Compiler->GetCompileErrors().empty();
	if (compilationSuccessful)
	{
		AddRequest(AppRequest::ChangeModeRequest(AppMode::Run));
		m_Executor->SetCompiledPipeline(pipeline); // TODO: This should also be in handle app request
		m_Executor->OnStart();
	}
	else
	{
		AddRequest(AppRequest::ChangeModeRequest(AppMode::Editor));
		
		// Mark error nodes
		for (const auto& err : m_Compiler->GetCompileErrors())
		{
			m_Console.Log("[Compilation error] " + err.Message);
			m_ErrorHandler->MarkErrorNode(err.Node);
		}
	}
}

std::string App::GetWindowTitle()
{
	std::string windowTitle = "Render Nodes";
	if (!m_CurrentLoadedFile.empty())
		windowTitle += " - " + m_CurrentLoadedFile;
	
	if (m_Editor->GetCommandExecutor()->GetExecutedCommandCount() != m_LastExecutedCommandCount)
		windowTitle += "*";

	return windowTitle;
}

void App::ProcessRequests()
{
	while (!m_Requests.empty())
	{
		const auto& request = m_Requests.front();
		switch (request.Type)
		{
		case AppRequestType::ChangeMode:
			ProcessChangeModeRequest(request);
			break;
		}
		m_Requests.pop();
	}
}

void App::ProcessChangeModeRequest(const AppRequest& request)
{
	m_Mode = request.ChangeMode.Mode;
	if (m_Mode == AppMode::CustomNode)
	{
		CustomEditorNode* customNode = nullptr;
		
		// Is name is empty, this is new node
		if (request.ChangeMode.CustomNodeName != "")
		{
			for (const auto& cn : m_CustomNodeList)
			{
				if (cn->GetName() == request.ChangeMode.CustomNodeName)
				{
					customNode = cn.get();
					break;
				}
			}
		}

		if (customNode != nullptr)
		{
			customNode->GetNodeGraph()->RefreshCustomNodes();
			m_CustomNodeEditor.reset(new CustomNodePipelineEditor{ customNode });
		}
		else if (request.ChangeMode.CustomNodeName == "")
		{
			m_CustomNodeEditor.reset(new CustomNodePipelineEditor{ });
		}
		else
		{
			m_Console.Log("[Internal error][App::ProcessChangeModeRequest] Cannot find custom node by name " + request.ChangeMode.CustomNodeName);
		}
	}
	else if(m_Mode == AppMode::Editor)
	{
		m_CustomNodeEditor = nullptr;
		m_NodeGraph->RefreshCustomNodes();
	}
	else if (m_Mode == AppMode::Run)
	{
		m_CustomNodeEditor = nullptr;
	}
}

