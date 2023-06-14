#pragma once

#include <unordered_map>

#include "../Common.h"
#include "Render/Texture.h"
#include "Render/Shader.h"
#include "Render/Buffer.h"

namespace ExecutionPrivate
{
	void Failure(const std::string& nodeName, const std::string& msg);
	void Warning(bool condition, const std::string& nodeName, const std::string& msg);
}

struct Mesh
{
	unsigned NumPrimitives = 0;
	Ptr<Buffer> VertexBuffer;
	Ptr<Buffer> IndexBuffer;
};

struct ExecutorVariableBlock
{
	std::unordered_map<uint32_t, float> Floats;
	std::unordered_map<uint32_t, glm::vec2> Float2s;
	std::unordered_map<uint32_t, glm::vec3> Float3s;
	std::unordered_map<uint32_t, glm::vec4> Float4s;
	std::unordered_map<uint32_t, Texture*> Textures;
	std::unordered_map<uint32_t, Buffer*> Buffers;
	std::unordered_map<uint32_t, Mesh*> Meshes;
	std::unordered_map<uint32_t, Shader*> Shaders;

	template<typename T> std::unordered_map<uint32_t, T>& GetMapFromType();
	template<> std::unordered_map<uint32_t, float>& GetMapFromType<float>() { return Floats; }
	template<> std::unordered_map<uint32_t, glm::vec2>& GetMapFromType<glm::vec2>() { return Float2s; }
	template<> std::unordered_map<uint32_t, glm::vec3>& GetMapFromType<glm::vec3>() { return Float3s; }
	template<> std::unordered_map<uint32_t, glm::vec4>& GetMapFromType<glm::vec4>() { return Float4s; }
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

struct ExecuteContext
{
	Texture* RenderTarget = nullptr;
	ExecutorVariableBlock Variables;
	ExecutorRenderResources RenderResources;

	bool Failure = false;
};

class ExecutorNode;

struct CompiledPipeline
{
	ExecutorNode* OnStartNode = nullptr;
	ExecutorNode* OnUpdateNode = nullptr;
};
