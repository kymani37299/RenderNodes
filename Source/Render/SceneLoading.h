#pragma once

#include <vector>

#include "../Common.h"
#include "Buffer.h"
#include "Texture.h"

struct cgltf_scene;
struct cgltf_node;
struct cgltf_primitive;
struct cgltf_material;
struct cgltf_texture;

namespace SceneLoading
{
	struct MeshData
	{
		unsigned PrimitiveCount = 0;

		Ptr<Buffer> Positions = nullptr;
		Ptr<Buffer> Texcoords = nullptr;
		Ptr<Buffer> Normals = nullptr;
		Ptr<Buffer> Tangents = nullptr;

		Ptr<Buffer> Indices = nullptr;
	};

	enum class MaterialType
	{
		Opaque,
		AlphaDiscard,
		AlphaBlend
	};

	struct MaterialData
	{
		MaterialType Type = MaterialType::Opaque;

		Float3 AlbedoFactor{ 1.0f, 1.0f, 1.0f };
		float MetallicFactor = 1.0f;
		float RoughnessFactor = 1.0f;

		Ptr<Texture> Albedo = nullptr;
		Ptr<Texture> Normal = nullptr;
		Ptr<Texture> MetallicRoughness = nullptr;
	};

	struct SceneObject
	{
		Float4x4 ModelTransform;

		MeshData Mesh;
		MaterialData Material;
	};

	struct Scene
	{
		std::vector<SceneObject> Objects;
	};

	class Loader
	{
	public:
		Ptr<Scene> Load(const std::string& scenePath);
		Ptr<Texture> LoadTexture(const std::string& texturePath, Float4 defaultColor = Float4{ 0.0f });

		bool HasErrors() const { return !m_ErrorMessages.empty(); }
		void PrintErrors()
		{
			for (const std::string& e : m_ErrorMessages)
			{
				std::cout << "[Loader error] " << e << std::endl;
			}
		}

	private:
		Scene* LoadScene(cgltf_scene* scene);
		void LoadNode(cgltf_node* nodeData, Scene* scene, const Float4x4& transform);

		MeshData LoadMesh(cgltf_primitive* meshData);
		MaterialData LoadMaterial(cgltf_material* materialData);

		Ptr<Texture> LoadTexture(cgltf_texture* textureData, Float4 defaultColor = Float4{0.0f});

	private:
		void Error(const std::string& msg)
		{
			m_ErrorMessages.push_back(msg);
		}

	private:
		std::string m_DirectoryPath = "";
		std::vector<std::string> m_ErrorMessages;
	};
}

