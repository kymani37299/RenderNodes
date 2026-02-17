#pragma once

#include <unordered_map>
#include <unordered_set>

#include "../Common.h"
#include "../IDGen.h"
#include "../App/IInputListener.h"
#include "../Render/Texture.h"
#include "../Render/Shader.h"
#include "../Render/Buffer.h"
#include "../NodeGraph/VariablePool.h"
#include "ExecutorScene.h"

namespace ExecutionPrivate
{
	void Failure(const std::string& nodeName, const std::string& msg);
	void Warning(bool condition, const std::string& nodeName, const std::string& msg);
}

enum class ExecutorStaticResource
{
	CubeMesh,
};

struct ExecutorRenderResources
{
	std::unordered_map<VariableID, Ptr<Texture>> Textures;
	std::unordered_map<VariableID, Ptr<Buffer>> Buffers;
	std::unordered_map<VariableID, Ptr<Mesh>> Meshes;
	std::unordered_map<VariableID, Ptr<Shader>> Shaders;
	std::unordered_map<VariableID, Ptr<Scene>> Scenes;

	template<typename T, ExecutorStaticResource staticResource>  T GetStaticResource();
	template<> Mesh* GetStaticResource<Mesh*, ExecutorStaticResource::CubeMesh>() { return Meshes[VariablePool::ID_CubeMesh].get(); }

	template<ExecutorStaticResource staticResource>  std::string GetStaticResourceVariableName();
	template<> std::string GetStaticResourceVariableName<ExecutorStaticResource::CubeMesh>() { return "CubeMesh"; }

	template<typename T> T GetResource(VariableID id);
	template<> Texture* GetResource(VariableID id) { return Textures[id].get(); }
	template<> Buffer* GetResource(VariableID id) { return Buffers[id].get(); }
	template<> Mesh* GetResource(VariableID id) { return Meshes[id].get(); }
	template<> Shader* GetResource(VariableID id) { return Shaders[id].get(); }
	template<> Scene* GetResource(VariableID id) { return Scenes[id].get(); }
};

struct ExecutorIterators
{
	using IteratorType = std::variant<
		SceneObject*
	>;

	std::unordered_map<PinID, IteratorType> Data{};

	template<typename T>
	T& Get(PinID pinID)
	{
		return std::get<T>(Data[pinID]);
	}
};

struct ExecutorInputState
{
	std::unordered_set<KeyInput> ReleasedKeys;
	std::unordered_set<KeyInput> PressedKeys;
	std::unordered_set<KeyInput> DownKeys;
};

class ExecutorNode;

struct ExecuteContext
{
	Texture* RenderTarget = nullptr;
	VariablePool VariablePool;
	ExecutorIterators Iterators;
	ExecutorRenderResources RenderResources;
	ExecutorInputState InputState;
	
	std::unordered_map<ExecutorNode*, NodeID> EditorLinks;
	
	bool Failure = false;
	NodeID FailedNode = 0;
};

struct CompiledPipeline
{
	ExecutorNode* OnStartNode = nullptr;
	ExecutorNode* OnUpdateNode = nullptr;

	std::unordered_map<KeyInput, ExecutorNode*> OnKeyPressedNodes;
	std::unordered_map<KeyInput, ExecutorNode*> OnKeyDownNodes;
	std::unordered_map<KeyInput, ExecutorNode*> OnKeyReleasedNodes;

	VariablePool VariablePool;

	std::unordered_map<ExecutorNode*, NodeID> EditorLinks;
};
