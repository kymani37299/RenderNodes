#include "RenderPipelineExecutor.h"

#include "../App/App.h"
#include "ExecutorNode.h"

void InitStaticResources(ExecuteContext& context)
{
	std::vector<GLuint> cubeIndices{
		//Top
		2, 6, 7,
		2, 3, 7,

		//Bottom
		0, 4, 5,
		0, 1, 5,

		//Left
		0, 2, 6,
		0, 4, 6,

		//Right
		1, 3, 7,
		1, 5, 7,

		//Front
		0, 2, 3,
		0, 1, 3,

		//Back
		4, 6, 7,
		4, 5, 7
	};


	std::vector<GLfloat> cubeVertices{
		-1, -1,  1, //0
		 1, -1,  1, //1
		-1,  1,  1, //2
		 1,  1,  1, //3
		-1, -1, -1, //4
		 1, -1, -1, //5
		-1,  1, -1, //6
		 1,  1, -1  //7
	};

	Mesh* cubeMesh = new Mesh{};
	cubeMesh->Positions = Buffer::Create(BufferType::Vertex, cubeVertices.size() * sizeof(float), sizeof(float), BF_None, cubeVertices.data());
	cubeMesh->Indices = Buffer::Create(BufferType::Index, cubeIndices.size() * sizeof(unsigned), sizeof(unsigned), BF_None, cubeIndices.data());
	cubeMesh->NumPrimitives = cubeIndices.size();

	context.RenderResources.Meshes.push_back(Ptr<Mesh>(cubeMesh));
	context.RenderResources.CubeMeshIndex = context.RenderResources.Meshes.size() - 1;
}

static void ExecuteNodePath(ExecuteContext& context, ExecutorNode* node)
{
	while (node)
	{
		node->Execute(context);
		node = node->GetNextNode();
	}
}

uint32_t ExecutorInputState::GetInputHash(int key, int mods)
{
	uint32_t hash = Hash::Crc32(key);
	hash = Hash::Crc32(hash, mods);
	return hash;
}

void RenderPipelineExecutor::OnStart()
{
	// Init context
	m_Context = ExecuteContext{};
	InitStaticResources(m_Context);

	// Clear console
	App::Get()->GetConsole().Clear();

	// Execute
	ExecuteNodePath(m_Context, m_Pipeline.OnStartNode);
}

void RenderPipelineExecutor::OnUpdate(float dt)
{
	if (m_Context.Failure) return;

	const std::string dtVar = "DT";
	m_Context.Variables.GetMapFromType<float>()[Hash::Crc32(dtVar)] = dt;

	// Update input
	const auto processInputNodes = [this](std::unordered_set<uint32_t>& keys, std::unordered_map<uint32_t, ExecutorNode*>& map)
	{
		for (const auto& key : keys)
		{
			if (map.find(key) != map.end())
			{
				ExecuteNodePath(m_Context, map[key]);
			}
		}
	};
	processInputNodes(m_Context.InputState.DownKeys, m_Pipeline.OnKeyDownNodes);
	processInputNodes(m_Context.InputState.ReleasedKeys, m_Pipeline.OnKeyReleasedNodes);
	processInputNodes(m_Context.InputState.PressedKeys, m_Pipeline.OnKeyPressedNodes);

	ExecuteNodePath(m_Context, m_Pipeline.OnUpdateNode);

	m_Context.InputState.PressedKeys.clear();
	m_Context.InputState.ReleasedKeys.clear();
}

void RenderPipelineExecutor::Render()
{
	if (m_Context.RenderTarget)
	{
		ImGui::Image((ImTextureID)m_Context.RenderTarget->TextureHandle, ImVec2{ (float) m_Context.RenderTarget->Width, (float) m_Context.RenderTarget->Height }, { 0, 1 }, { 1, 0 });
	}
}

void RenderPipelineExecutor::HandleKeyPressed(int key, int mods)
{
	if (m_Context.Failure) return;

	const uint32_t inputHash = ExecutorInputState::GetInputHash(key, mods);
	m_Context.InputState.DownKeys.insert(inputHash);
	m_Context.InputState.PressedKeys.insert(inputHash);
}

void RenderPipelineExecutor::HandleKeyReleased(int key, int mods)
{
	if (m_Context.Failure) return;

	const uint32_t inputHash = ExecutorInputState::GetInputHash(key, mods);
	m_Context.InputState.DownKeys.erase(inputHash);
	m_Context.InputState.ReleasedKeys.insert(inputHash);
}

void RenderPipelineExecutor::SetCompiledPipeline(CompiledPipeline pipeline)
{
	delete m_Pipeline.OnStartNode;
	delete m_Pipeline.OnUpdateNode;
	for (auto& it : m_Pipeline.OnKeyPressedNodes) delete it.second;
	for (auto& it : m_Pipeline.OnKeyReleasedNodes) delete it.second;
	for (auto& it : m_Pipeline.OnKeyDownNodes) delete it.second;

	m_Pipeline = pipeline;
}
