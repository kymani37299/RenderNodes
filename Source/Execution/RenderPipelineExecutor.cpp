#include "RenderPipelineExecutor.h"

#include "../App/App.h"
#include "../Editor/EditorErrorHandler.h"
#include "../Render/SceneLoading.h"
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

	context.RenderResources.Meshes[VariablePool::ID_CubeMesh] = Ptr<Mesh>(cubeMesh);
}

void InitRenderResources(ExecuteContext& context)
{
	std::vector<std::string> errorMessages;
	SceneLoading::Loader loader{};
	const auto fn = [&context, &loader, &errorMessages](VariableID id, const Variable& variable) {
		switch (variable.Type)
		{
		case VariableType::Texture:
		{
			const TextureData& texData = variable.Get<TextureData>();
			if (texData.Path.empty())
			{
				unsigned int flags = TF_None;
				if (texData.Framebuffer) flags |= TF_Framebuffer;
				if (texData.DepthStencil) flags |= TF_DepthStencil;
				context.RenderResources.Textures[id] = Texture::Create(texData.Width, texData.Height, flags);
			}
			else
			{
				context.RenderResources.Textures[id] = loader.LoadTexture(texData.Path);
			}
		} break;
		case VariableType::Scene:
		{
			const SceneData& sceneData = variable.Get<SceneData>();
			if (sceneData.Path.empty())
			{
				errorMessages.push_back("Failed to load scene under variable " + variable.Name + " (empty path)");
				return;
			}
			const auto& loadedScene = loader.Load(sceneData.Path);
			if (loadedScene)
			{
				Scene* scene = new Scene{};
				for (auto& loadedObject : loadedScene->Objects)
				{
					auto& objectMesh = loadedObject.Mesh;

					SceneObject sceneObject;
					sceneObject.Albedo = std::move(loadedObject.Material.Albedo);
					sceneObject.ModelTransform = loadedObject.ModelTransform;

					Mesh& mesh = sceneObject.MeshData;
					mesh.NumPrimitives = objectMesh.PrimitiveCount;
					mesh.Positions = std::move(objectMesh.Positions);
					mesh.Texcoords = std::move(objectMesh.Texcoords);
					mesh.Normals = std::move(objectMesh.Normals);
					mesh.Tangents = std::move(objectMesh.Tangents);
					mesh.Indices = std::move(objectMesh.Indices);

					scene->SceneObjects.push_back(std::move(sceneObject));
				}
				context.RenderResources.Scenes[id] = Ptr<Scene>(scene);
			}
		} break;
		case VariableType::Shader:
		{
			const ShaderData& shaderData = variable.Get<ShaderData>();
			if (shaderData.Path.empty())
			{
				errorMessages.push_back("Failed to load shader under variable " + variable.Name + " (empty path)");
				return;
			}
			context.RenderResources.Shaders[id] = Shader::Compile(shaderData.Path);
		} break;
		}
	};
	context.VariablePool.ForEachVariable(fn);

	if (!errorMessages.empty())
	{
		for (const std::string& errorMessage : errorMessages)
		{
			App::Get()->GetConsole().Log("[InitVars error] " + errorMessage);
		}
		context.Failure = true;
	}

	if (loader.HasErrors())
	{
		loader.PrintErrors();
		context.Failure = true;
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
	m_Context.EditorLinks = m_Pipeline.EditorLinks;
	m_Context.VariablePool = m_Pipeline.VariablePool;
	InitStaticResources(m_Context);
	InitRenderResources(m_Context);

	// Clear console
	App::Get()->GetConsole().Clear();

	// Execute
	m_Pipeline.OnStartNode->ExecuteNodePath(m_Context);

	HandleErrors();
}

void RenderPipelineExecutor::OnUpdate(float dt)
{
	const std::string dtVar = "DT";
	m_Context.VariablePool.GetRefOrCreate(VariablePool::ID_DT, VariableType::Float, dtVar).Get<float>() = dt;
	
	// Update input
	const auto processInputNodes = [this](std::unordered_set<uint32_t>& keys, std::unordered_map<uint32_t, ExecutorNode*>& map)
	{
		for (const auto& key : keys)
		{
			if (map.find(key) != map.end())
			{
				map[key]->ExecuteNodePath(m_Context);
			}
		}
	};
	processInputNodes(m_Context.InputState.DownKeys, m_Pipeline.OnKeyDownNodes);
	processInputNodes(m_Context.InputState.ReleasedKeys, m_Pipeline.OnKeyReleasedNodes);
	processInputNodes(m_Context.InputState.PressedKeys, m_Pipeline.OnKeyPressedNodes);

	m_Pipeline.OnUpdateNode->ExecuteNodePath(m_Context);

	m_Context.InputState.PressedKeys.clear();
	m_Context.InputState.ReleasedKeys.clear();

	HandleErrors();
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
	const uint32_t inputHash = ExecutorInputState::GetInputHash(key, mods);
	m_Context.InputState.DownKeys.insert(inputHash);
	m_Context.InputState.PressedKeys.insert(inputHash);
}

void RenderPipelineExecutor::HandleKeyReleased(int key, int mods)
{
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

void RenderPipelineExecutor::HandleErrors()
{
	if (m_Context.Failure && m_Context.FailedNode != 0)
	{
		App::Get()->GetErrorHandler().MarkErrorNode(m_Context.FailedNode);
	}
}
