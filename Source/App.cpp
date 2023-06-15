#include "App.h"

#include "Common.h"
#include "IDGen.h"

#include <filesystem>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Editor/RenderPipelineEditor.h"
#include "Execution/RenderPipelineExecutor.h"
#include "NodeGraph/NodeGraphCompiler.h"
#include "NodeGraph/NodeGraphSerializer.h"

#include "Util/FileDialog.h"

// From IDGen.h
std::atomic<UniqueID> IDGen::Allocator = 1;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

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
    m_Window = glfwCreateWindow(m_Width, m_Height, "RenderNodes", useFullscreen ? monitor : NULL, NULL);
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

    // Init imgui
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    FileDialog::Init();
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
    static const std::string RUN_STR = "   Run   ";
    static const std::string STOP_STR = "   Stop   ";

    bool editMode = true;
    bool editorInitialized = false;
    Ptr<RenderPipelineEditor> editor{ new RenderPipelineEditor{} };
    Ptr<RenderPipelineExecutor> executor{ new RenderPipelineExecutor{} };
    NodeGraphCompiler compiler;
    NodeGraphSerializer serializer;

    bool compilationSuccessful = false;

    // Render loop
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwSwapBuffers(m_Window);
        glfwPollEvents();

        // Imgui frame
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            ImGui::Begin("Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

            ImGui::SetWindowFontScale(m_Width / 1920.0f);

            // Render
            {
                if (ImGui::Button("New") || !editorInitialized)
                {
                    if (!editorInitialized)
                        editor->Unload();

                    std::filesystem::remove("NodeEditor.json");

                    IDGen::Init(1);
					editor->Load(NodeGraph::CreateDefaultNodeGraph());
					editor->InitializeDefaultNodePositions();

                    editorInitialized = true;
                }

                ImGui::SameLine();

                if (ImGui::Button("Load"))
                {
                    std::string path;
                    if (FileDialog::OpenRenderNodeFile(path))
                    {
                        NodeGraph* loadedNodeGraph = new NodeGraph{};
						const UniqueID firstID = serializer.Deserialize(path, *loadedNodeGraph);
						IDGen::Init(firstID);

						if (firstID)
						{
							editor->Unload();
							const std::string nodeEditorConfigPath = GetPathWithoutExtension(path) + ".json";
                            std::filesystem::copy(nodeEditorConfigPath, "NodeEditor.json", std::filesystem::copy_options::overwrite_existing);
							editor->Load(loadedNodeGraph);
						}
                        else
                        {
                            delete loadedNodeGraph;
                        }
                    }
                }

                ImGui::SameLine();

				if (ImGui::Button("Save"))
				{
					std::string path;
					if (FileDialog::SaveRenderNodeFile(path))
					{
                        serializer.Serialize(path, editor->GetNodeGraph());

                        const std::string jsonDest = GetPathWithoutExtension(path) + ".json";
                        std::filesystem::copy("NodeEditor.json", jsonDest, std::filesystem::copy_options::overwrite_existing);
					}
				}

                ImGui::Separator();
                ImGui::SetNextItemWidth(250.0f);
                if (ImGui::Button(editMode ? RUN_STR.c_str() : STOP_STR.c_str()))
                {
                    editMode = !editMode;

					if (!editMode)
					{
						CompiledPipeline pipeline = compiler.Compile(editor->GetNodeGraph());
						compilationSuccessful = compiler.GetErrorMessages().empty();
						if (compilationSuccessful)
						{
							executor->SetCompiledPipeline(pipeline);
							executor->OnStart();
						}
						else
						{
							for (const std::string& err : compiler.GetErrorMessages())
								std::cout << "[Compilation error] " << err << std::endl;
							editMode = true;
						}
					}
                }
                ImGui::Separator();

                if (editMode)
                    editor->Render();
                else
                {
                    ASSERT(compilationSuccessful);
                    const float dt = 1000.0f / ImGui::GetIO().Framerate;
                    executor->OnUpdate(dt);
                    executor->Render();
                }
                   
            }

            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }
}