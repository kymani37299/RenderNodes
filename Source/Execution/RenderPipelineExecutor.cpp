#include "RenderPipelineExecutor.h"

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

void RenderPipelineExecutor::OnStart()
{
	// Init context
	m_Context = ExecuteContext{};
	InitStaticResources(m_Context);

	ExecutorNode* node = m_Pipeline.OnStartNode;
	while (node)
	{
		node->Execute(m_Context);
		node = node->GetNextNode();
	}
}

void RenderPipelineExecutor::OnUpdate(float dt)
{
	if (m_Context.Failure) return;

	ExecutorNode* node = m_Pipeline.OnUpdateNode;
	const std::string dtVar = "dt";
	m_Context.Variables.GetMapFromType<float>()[Hash::Crc32(dtVar)] = dt;
	while (node)
	{
		node->Execute(m_Context);
		node = node->GetNextNode();
	}
}

void RenderPipelineExecutor::Render()
{
	if (m_Context.RenderTarget)
	{
		ImGui::Image((ImTextureID)m_Context.RenderTarget->TextureHandle, ImVec2{ (float) m_Context.RenderTarget->Width, (float) m_Context.RenderTarget->Height }, { 0, 1 }, { 1, 0 });
	}
}

void RenderPipelineExecutor::SetCompiledPipeline(CompiledPipeline pipeline)
{
	delete m_Pipeline.OnStartNode;
	delete m_Pipeline.OnUpdateNode;
	m_Pipeline = pipeline;
}
