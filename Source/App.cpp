#include "App.h"

#include "Common.h"
#include "IDGen.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Editor/RenderPipelineEditor.h"
#include "Execution/RenderPipelineExecutor.h"
#include "NodeGraph/NodeGraphCompiler.h"
#include "NodeGraph/NodeGraphSerializer.h"

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
}

App::~App()
{
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
    bool editMode = true;
    Ptr<RenderPipelineEditor> editor{ new RenderPipelineEditor{} };
    Ptr<RenderPipelineExecutor> executor{ new RenderPipelineExecutor{} };
    NodeGraphCompiler compiler;
    NodeGraphSerializer serializer;

    static const std::string FILE_PATH = "LastSession.rn";
    static const std::string JSON_FILE_PATH = GetPathWithoutExtension(FILE_PATH) + ".json";

    NodeGraph* nodeGraph = new NodeGraph{};

    const UniqueID firstID = serializer.Deserialize(FILE_PATH, *nodeGraph);
    IDGen::Init(firstID);

    if (!firstID)
    {
        delete nodeGraph;
        nodeGraph = NodeGraph::CreateDefaultNodeGraph(); 
    }
    editor->Load(JSON_FILE_PATH, nodeGraph);

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

            ImGui::SetWindowFontScale(m_Width / 1024.0f);

            // Render
            {
                if (ImGui::Checkbox("Edit mode", &editMode))
                {
                    if (!editMode)
                    {
                        CompiledPipeline pipeline = compiler.Compile(editor->GetNodeGraph());
                        executor->SetCompiledPipeline(pipeline);

                        executor->OnStart();
                    }   
                }
                ImGui::Separator();

                if (editMode)
                    editor->Render();
                else
                    executor->OnUpdate();
            }

            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }
    serializer.Serialize(FILE_PATH, editor->GetNodeGraph());
}