#pragma once

#include <unordered_map>
#include <unordered_set>

#include "../Common.h"
#include "../Render/Texture.h"
#include "../Render/Shader.h"
#include "../Render/Buffer.h"

namespace ExecutionPrivate
{
	void Failure(const std::string& nodeName, const std::string& msg);
	void Warning(bool condition, const std::string& nodeName, const std::string& msg);
}

struct Mesh
{
	unsigned NumPrimitives = 0;
	Ptr<Buffer> Positions;
	Ptr<Buffer> Texcoords;
	Ptr<Buffer> Normals;
	Ptr<Buffer> Tangents;
	Ptr<Buffer> Indices;
};

struct ExecutorVariableBlock
{
	std::unordered_map<uint32_t, bool> Bools;
	std::unordered_map<uint32_t, int> Ints;
	std::unordered_map<uint32_t, float> Floats;
	std::unordered_map<uint32_t, Float2> Float2s;
	std::unordered_map<uint32_t, Float3> Float3s;
	std::unordered_map<uint32_t, Float4> Float4s;
	std::unordered_map<uint32_t, Float4x4> Float4x4s;
	std::unordered_map<uint32_t, Texture*> Textures;
	std::unordered_map<uint32_t, Buffer*> Buffers;
	std::unordered_map<uint32_t, Mesh*> Meshes;
	std::unordered_map<uint32_t, Shader*> Shaders;

	template<typename T> std::unordered_map<uint32_t, T>& GetMapFromType();
	template<> std::unordered_map<uint32_t, bool>& GetMapFromType<bool>() { return Bools; }
	template<> std::unordered_map<uint32_t, int>& GetMapFromType<int>() { return Ints; }
	template<> std::unordered_map<uint32_t, float>& GetMapFromType<float>() { return Floats; }
	template<> std::unordered_map<uint32_t, Float2>& GetMapFromType<Float2>() { return Float2s; }
	template<> std::unordered_map<uint32_t, Float3>& GetMapFromType<Float3>() { return Float3s; }
	template<> std::unordered_map<uint32_t, Float4>& GetMapFromType<Float4>() { return Float4s; }
	template<> std::unordered_map<uint32_t, Float4x4>& GetMapFromType<Float4x4>() { return Float4x4s; }
	template<> std::unordered_map<uint32_t, Texture*>& GetMapFromType<Texture*>() { return Textures; }
	template<> std::unordered_map<uint32_t, Shader*>& GetMapFromType<Shader*>() { return Shaders; }
	template<> std::unordered_map<uint32_t, Buffer*>& GetMapFromType<Buffer*>() { return Buffers; }
	template<> std::unordered_map<uint32_t, Mesh*>& GetMapFromType<Mesh*>() { return Meshes; }
};

enum class ExecutorStaticResource
{
	CubeMesh,
};

struct ExecutorRenderResources
{
	unsigned CubeMeshIndex = 0;

	std::vector<Ptr<Texture>> Textures;
	std::vector<Ptr<Buffer>> Buffers;
	std::vector<Ptr<Mesh>> Meshes;
	std::vector<Ptr<Shader>> Shaders;

	template<typename T, ExecutorStaticResource staticResource>  T GetStaticResource();
	template<> Mesh* GetStaticResource<Mesh*, ExecutorStaticResource::CubeMesh>() { return Meshes[CubeMeshIndex].get(); }
};

struct ExecutorInputState
{
	std::unordered_set<uint32_t> ReleasedKeys;
	std::unordered_set<uint32_t> PressedKeys;
	std::unordered_set<uint32_t> DownKeys;

	static uint32_t GetInputHash(int key, int mods);
};

struct ExecuteContext
{
	Texture* RenderTarget = nullptr;
	ExecutorVariableBlock Variables;
	ExecutorRenderResources RenderResources;
	ExecutorInputState InputState;

	bool Failure = false;
};

class ExecutorNode;

struct CompiledPipeline
{
	ExecutorNode* OnStartNode = nullptr;
	ExecutorNode* OnUpdateNode = nullptr;

	std::unordered_map<uint32_t, ExecutorNode*> OnKeyPressedNodes;
	std::unordered_map<uint32_t, ExecutorNode*> OnKeyDownNodes;
	std::unordered_map<uint32_t, ExecutorNode*> OnKeyReleasedNodes;
};
