#include "SceneLoading.h"

#pragma warning(disable : 4996)
#ifndef CGLTF_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define CGTF_CALL(X) { cgltf_result result = X; ASSERT(result == cgltf_result_success); }

namespace SceneLoading
{
	struct NodeTransform
	{
		Float3 Position{ 0.0f, 0.0f, 0.0f };
		Quaternion Rotation{ 0.0f, 0.0f, 0.0f, 0.0f };
		Float3 Scale{ 1.0f, 1.0f, 1.0f };
	};

	static std::string GetPathWitoutFile(const std::string& path)
	{
		return path.substr(0, 1 + path.find_last_of("\\/"));
	}

	static Float3 ToFloat3(cgltf_float color[3])
	{
		return Float3{ color[0], color[1], color[2] };
	}

	static Quaternion ToQuaternion(cgltf_float color[4])
	{
		return Quaternion{ color[0], color[1], color[2], color[3] };
	}

	static Float4 ToFloat4(cgltf_float color[4])
	{
		return Float4{ color[0], color[1], color[2], color[3] };
	}

	static Float4x4 ToFloat4x4(cgltf_float matrix[16])
	{
		return Float4x4(matrix[0], matrix[1], matrix[2], matrix[3],
						matrix[4], matrix[5], matrix[6], matrix[7], 
						matrix[8], matrix[9], matrix[10], matrix[11],
						matrix[12], matrix[13], matrix[14], matrix[15]);
	}

	static void DecomposeMatrix(const glm::mat4& m, glm::vec3& pos, glm::quat& rot, glm::vec3& scale)
	{
		pos = m[3];
		for (int i = 0; i < 3; i++)
			scale[i] = glm::length(glm::vec3(m[i]));
		const glm::mat3 rotMtx(
			glm::vec3(m[0]) / scale[0],
			glm::vec3(m[1]) / scale[1],
			glm::vec3(m[2]) / scale[2]);
		rot = glm::quat_cast(rotMtx);
	}

	NodeTransform GetTransform(cgltf_node* node)
	{
		NodeTransform t;
		t.Position = node->has_translation ? ToFloat3(node->translation) : Float3{ 0.0f, 0.0f, 0.0f };
		t.Rotation = node->has_rotation ? ToQuaternion(node->rotation) : Quaternion{ 0.0f, 0.0f, 0.0f, 0.0f };
		t.Scale = node->has_scale ? ToFloat3(node->scale) : Float3{ 1.0f, 1.0f, 1.0f };

		if (node->has_matrix)
		{
			const Float4x4 modelMatrix = ToFloat4x4(node->matrix);

			NodeTransform matrixData;
			DecomposeMatrix(modelMatrix, matrixData.Position, matrixData.Rotation, matrixData.Scale);

			t.Position += matrixData.Position;
			t.Rotation *= matrixData.Rotation;
			t.Scale *= matrixData.Scale;
		}
		return t;
	}

	template<cgltf_type TYPE, cgltf_component_type COMPONENT_TYPE>
	static void ValidateVertexAttribute(cgltf_attribute* attribute)
	{
		ASSERT_M(attribute->data->type == TYPE, "[SceneLoading] ASSERT FAILED: attributeAccessor->type == TYPE");
		ASSERT_M(attribute->data->component_type == COMPONENT_TYPE, "[SceneLoading] ASSERT FAILED: attributeAccessor->component_type == COMPONENT_TYPE");
	}

	template<typename T>
	static T* GetBufferData(cgltf_accessor* accessor)
	{
		unsigned char* buffer = (unsigned char*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset;
		void* data = buffer + accessor->offset;
		return static_cast<T*>(data);
	}

	template<typename T>
	static void FetchIndexData(cgltf_accessor* indexAccessor, std::vector<uint32_t>& buffer)
	{
		T* indexData = GetBufferData<T>(indexAccessor);
		for (size_t i = 0; i < indexAccessor->count; i++)
		{
			buffer[i] = indexData[i];
		}
	}

	static void LoadIB(cgltf_accessor* indexAccessor, std::vector<uint32_t>& buffer)
	{
		ASSERT_M(indexAccessor, "[SceneLoading] Trying to read indices from empty accessor");
		ASSERT_M(indexAccessor->type == cgltf_type_scalar, "[SceneLoading] Indices of a mesh arent scalar.");

		buffer.resize(indexAccessor->count);

		switch (indexAccessor->component_type)
		{
		case cgltf_component_type_r_16u: FetchIndexData<uint16_t>(indexAccessor, buffer); break;
		case cgltf_component_type_r_32u: FetchIndexData<uint32_t>(indexAccessor, buffer); break;
		default: NOT_IMPLEMENTED;
		}
	}

	struct VertexAttributesData
	{
		uint32_t NumVertices = 0;

		Float3* Positions = nullptr;
		Float2* Texcoords = nullptr;
		Float3* Normals = nullptr;
		Float4* Tangents = nullptr;
	};

	VertexAttributesData LoadAttributes(cgltf_attribute* attributes, cgltf_size attributesNumber)
	{
		if (attributesNumber == 0) return VertexAttributesData{};

		VertexAttributesData attributesData;
		attributesData.NumVertices = (uint32_t)attributes->data->count;
		for (size_t i = 0; i < attributesNumber; i++)
		{
			cgltf_attribute* vertexAttribute = (attributes + i);
			switch (vertexAttribute->type)
			{
			case cgltf_attribute_type_position:
				ValidateVertexAttribute<cgltf_type_vec3, cgltf_component_type_r_32f>(vertexAttribute);
				attributesData.Positions = GetBufferData<Float3>(vertexAttribute->data);
				break;
			case cgltf_attribute_type_texcoord:
				ValidateVertexAttribute<cgltf_type_vec2, cgltf_component_type_r_32f>(vertexAttribute);
				attributesData.Texcoords = GetBufferData<Float2>(vertexAttribute->data);
				break;
			case cgltf_attribute_type_normal:
				ValidateVertexAttribute<cgltf_type_vec3, cgltf_component_type_r_32f>(vertexAttribute);
				attributesData.Normals = GetBufferData<Float3>(vertexAttribute->data);
				break;
			case cgltf_attribute_type_tangent:
				if (vertexAttribute->type != cgltf_type_vec4) continue; // Tmp hack: Find a way to fix this
				ValidateVertexAttribute<cgltf_type_vec4, cgltf_component_type_r_32f>(vertexAttribute);
				attributesData.Tangents = GetBufferData<Float4>(vertexAttribute->data);
				break;
			default:
				NOT_IMPLEMENTED;
				break;
			}
		}
		return attributesData;
	}

	Ptr<Scene> Loader::Load(const std::string& scenePath)
	{
		m_ErrorMessages.clear();
		m_DirectoryPath = GetPathWitoutFile(scenePath);

		cgltf_options options = {};
		cgltf_data* data = NULL;
		CGTF_CALL(cgltf_parse_file(&options, scenePath.c_str(), &data));
		CGTF_CALL(cgltf_load_buffers(&options, data, scenePath.c_str()));

		if (data == nullptr || data->scene == nullptr)
		{
			Error("Unable to load object file");
			return Ptr<Scene>(nullptr);
		}

		Scene* scene = LoadScene(data->scene);

		cgltf_free(data);

		return Ptr<Scene>(scene);
	}

	Scene* Loader::LoadScene(cgltf_scene* scene)
	{
		Scene* loadedScene = new Scene{};
		for (unsigned i = 0; i < scene->nodes_count; i++)
			LoadNode(*(scene->nodes + i), loadedScene, Float4x4{1.0f});
		return loadedScene;
	}

	void Loader::LoadNode(cgltf_node* nodeData, Scene* scene, const Float4x4& parentMatrix)
	{
		// TODO: Determine order of operations since glm is column major
		// Probably transforms are not working right now

		const NodeTransform nodeTranform = GetTransform(nodeData);
		Float4x4 localMatrix = glm::scale(Float4x4{}, nodeTranform.Scale);
		localMatrix = localMatrix * glm::toMat4(nodeTranform.Rotation);
		localMatrix = glm::translate(localMatrix, nodeTranform.Position);

		const Float4x4 nodeMatrix = localMatrix * parentMatrix;

		if (nodeData->mesh)
		{
			cgltf_mesh* mesh = nodeData->mesh;
			for (cgltf_size i = 0; i < mesh->primitives_count; i++)
			{
				cgltf_primitive* primitive = mesh->primitives + i;

				SceneObject object{};
				object.ModelTransform = nodeMatrix;
				object.Mesh = LoadMesh(primitive);
				object.Material = LoadMaterial(primitive->material);
				scene->Objects.push_back(std::move(object));
			}
		}

		for (uint32_t i = 0; i < nodeData->children_count; i++)
		{
			LoadNode(*(nodeData->children + i), scene, nodeMatrix);
		}

	}

	MeshData Loader::LoadMesh(cgltf_primitive* meshData)
	{
		if (meshData->type != cgltf_primitive_type_triangles)
		{
			Error("Scene contains quad meshes. Only triangle meshes are supported.");
			return MeshData{};
		}

		MeshData mesh;

		const uint32_t vertCount = (uint32_t)meshData->attributes[0].data->count;

		const VertexAttributesData vertices = LoadAttributes(meshData->attributes, meshData->attributes_count);

		std::vector<uint32_t> indices;
		if (meshData->indices)
			LoadIB(meshData->indices, indices);

		const auto createBuffer = [this](const void* data, uint32_t stride, uint32_t numElements, BufferType bufferType = BufferType::Vertex)
		{
			std::vector<uint8_t> defaultData{};
			if (!data)
			{
				defaultData.resize(stride * numElements);
				memset(defaultData.data(), 0, stride * numElements);
				data = defaultData.data();
			}
			return Buffer::Create(bufferType, stride * numElements, stride, BF_None, data);
		};
		mesh.Positions = createBuffer(vertices.Positions, sizeof(Float3), vertCount);
		mesh.Texcoords = createBuffer(vertices.Texcoords, sizeof(Float2), vertCount);
		mesh.Normals = createBuffer(vertices.Normals, sizeof(Float3), vertCount);
		mesh.Tangents = createBuffer(vertices.Tangents, sizeof(Float4), vertCount);
		mesh.Indices = createBuffer(indices.empty() ? nullptr : indices.data(), sizeof(uint32_t), (uint32_t)indices.size(), BufferType::Index);
		mesh.PrimitiveCount = mesh.Indices ? (uint32_t)indices.size() : vertices.NumVertices;

		return mesh;
	}

	MaterialData Loader::LoadMaterial(cgltf_material* materialData)
	{
		MaterialData material{};

		if (!materialData || !materialData->has_pbr_metallic_roughness)
		{
			material.Albedo = LoadTexture(nullptr);
			material.Normal = LoadTexture(nullptr, Float4(0.5f, 0.5f, 1.0f, 1.0f));
			material.MetallicRoughness = LoadTexture(nullptr);
			return material;
		}

		cgltf_pbr_metallic_roughness& mat = materialData->pbr_metallic_roughness;

		switch (materialData->alpha_mode)
		{
		case cgltf_alpha_mode_blend:
			material.Type = MaterialType::AlphaBlend; break;
		case cgltf_alpha_mode_mask:
			material.Type = MaterialType::AlphaDiscard; break;
		case cgltf_alpha_mode_opaque:
			material.Type = MaterialType::Opaque; break;
		default:
			NOT_IMPLEMENTED;
		}

		material.AlbedoFactor = ToFloat3(mat.base_color_factor);
		material.MetallicFactor = mat.metallic_factor;
		material.RoughnessFactor = mat.roughness_factor;

		material.Albedo = LoadTexture(mat.base_color_texture.texture);
		material.Normal = LoadTexture(materialData->normal_texture.texture, Float4(0.5f, 0.5f, 1.0f, 1.0f));
		material.MetallicRoughness = LoadTexture(mat.metallic_roughness_texture.texture);

		return material;
	}

	Ptr<Texture> Loader::LoadTexture(cgltf_texture* textureData, Float4 defaultColor)
	{
		if (!textureData || !textureData->image) return LoadTexture("", defaultColor);

		const std::string textureURI = textureData->image->uri;
		const std::string texturePath = m_DirectoryPath + "/" + textureURI;
		return LoadTexture(texturePath);
	}

	Ptr<Texture> Loader::LoadTexture(const std::string& texturePath, Float4 defaultColor)
	{
		const auto toU8 = [](float x) { return (uint8_t)(255.0f * x); };
		const auto toUnorm = [&toU8](Float4 rgba)
		{
			return (uint32_t)(toU8(rgba.a) | (toU8(rgba.b) << 8) | (toU8(rgba.g) << 16) | (toU8(rgba.r) << 24));
		};

		const uint32_t defaultColorUnorm = toUnorm(defaultColor);

		if (texturePath.empty()) return Texture::Create(1, 1, TF_None, &defaultColorUnorm);

		int width, height, bpp;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = stbi_load(texturePath.c_str(), &width, &height, &bpp, 4);

		if(!data) return Texture::Create(1, 1, TF_None, &defaultColorUnorm);

		Ptr<Texture> loadedTexture = Texture::Create(width, height, TF_None, data);

		stbi_image_free(data);

		return std::move(loadedTexture);
	}

}


