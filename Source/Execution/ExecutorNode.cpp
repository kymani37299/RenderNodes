#include "ExecutorNode.h"

#include "../Common.h"
#include "../Render/Texture.h"
#include "../Render/Shader.h"
#include "../Render/SceneLoading.h"

namespace ExecutionPrivate
{
	void Failure(const std::string& nodeName, const std::string& msg)
	{
		std::cout << "[FAILURE] " << "[" << nodeName << "] " << msg << std::endl;
	}

	void Warning(bool condition, const std::string& nodeName, const std::string& msg)
	{
		if (!condition)
		{
			std::cout << "[" << nodeName << "] " << msg << std::endl;
		}
	}
}

namespace
{
	unsigned CreateVAO(Mesh* mesh, const ValueNodeExtraInfo& extraInfo)
	{
		unsigned vao;
		GL_CALL(glGenVertexArrays(1, &vao));
		GL_CALL(glBindVertexArray(vao));

		unsigned nextAttribArray = 0;
		unsigned nextOffset = 0;

		const auto& vertexBits = extraInfo.MeshVertexBits;

		if (vertexBits.Position)
		{
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->Positions->Handle));
			GL_CALL(glVertexAttribPointer(nextAttribArray, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nextOffset));
			GL_CALL(glEnableVertexAttribArray(nextAttribArray++));
			nextOffset += 3 * sizeof(float);
		}

		if (vertexBits.Texcoord)
		{
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->Texcoords->Handle));
			GL_CALL(glVertexAttribPointer(nextAttribArray, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)nextOffset));
			GL_CALL(glEnableVertexAttribArray(nextAttribArray++));
			nextOffset += 2 * sizeof(float);
		}

		if (vertexBits.Normal)
		{
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->Normals->Handle));
			GL_CALL(glVertexAttribPointer(nextAttribArray, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nextOffset));
			GL_CALL(glEnableVertexAttribArray(nextAttribArray++));
			nextOffset += 3 * sizeof(float);
		}

		if (vertexBits.Tangent)
		{
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->Tangents->Handle));
			GL_CALL(glVertexAttribPointer(nextAttribArray, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)nextOffset));
			GL_CALL(glEnableVertexAttribArray(nextAttribArray++));
			nextOffset += 4 * sizeof(float);
		}

		return vao;
	}

	bool GetUniformIndex(Shader* shader, const std::string& name, unsigned& uniformIndex)
	{
		uniformIndex = glGetUniformLocation(shader->Handle, name.c_str());
		return uniformIndex != -1;
	}

	bool TableBind(ExecuteContext& context, Shader* shader, BindTable* bindTable)
	{
		if (!bindTable) return false;

		for (unsigned i = 0; i < bindTable->Textures.size(); i++)
		{
			const auto& binding = bindTable->Textures[i];
			Texture* texture = binding.Value.get() ? binding.Value->GetValue(context) : (Texture*) nullptr;
			if (!texture) return false;

			unsigned textureSlot;
			if (GetUniformIndex(shader, binding.Name, textureSlot))
			{
				GL_CALL(glActiveTexture(GL_TEXTURE0 + textureSlot));
				GL_CALL(glBindTexture(GL_TEXTURE_2D, texture->TextureHandle));
			}
		}
		return true;
	}

	void TableUnbind(Shader* shader, BindTable* bindTable)
	{
		if (!bindTable) return;

		for (unsigned i = 0; i < bindTable->Textures.size(); i++)
		{
			const auto& binding = bindTable->Textures[i];
			unsigned textureSlot;
			if (GetUniformIndex(shader, binding.Name, textureSlot))
			{
				GL_CALL(glActiveTexture(GL_TEXTURE0 + textureSlot));
				GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
			}
		}
	}
}

using namespace ExecutionPrivate;

void IfExecutorNode::Execute(ExecuteContext& context)
{
	m_PassedCondition = m_Condition->GetValue(context);
}

std::string ToString(const Float2& value)
{
	return "(" + std::to_string(value.x) + ",  " + std::to_string(value.y) + ")";
}

std::string ToString(const Float3& value)
{
	return "(" + std::to_string(value.x) + ",  " + std::to_string(value.y) + ",  " + std::to_string(value.z) + ")";
}

std::string ToString(const Float4& value)
{
	return "(" + std::to_string(value.x) + ",  " + std::to_string(value.y) + ",  " + std::to_string(value.z) + ",  " + std::to_string(value.w) + ")";
}

void PrintExecutorNode::Execute(ExecuteContext& context)
{
	if(m_FloatNode)  std::cout << m_FloatNode->GetValue(context) << std::endl;
	if(m_Float2Node)  std::cout << ToString(m_Float2Node->GetValue(context)) << std::endl;
	if(m_Float3Node)  std::cout << ToString(m_Float3Node->GetValue(context)) << std::endl;
	if(m_Float4Node)  std::cout << ToString(m_Float4Node->GetValue(context)) << std::endl;
}

void ClearRenderTargetExecutorNode::Execute(ExecuteContext& context)
{
	const Float4 clearColor = m_ClearColorNode->GetValue(context);
	const Texture* texture = m_TextureNode->GetValue(context);
	Warning(texture, "ClearRenderTargetExecutorNode", "Input texture is null");

	if (texture)
	{
		Warning(texture->FrameBufferHandle, "ClearRenderTargetExecutorNode", "Input texture is not framebuffer");

		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, texture->FrameBufferHandle));
		GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
}

void PresentTextureExecutorNode::Execute(ExecuteContext& context)
{
	Texture* texture = m_Texture->GetValue(context);
	Warning(texture, "PresentTextureExecutorNode", "Input texture is null");
	context.RenderTarget = texture;
}

template<typename T>
static T* AddToPtrVector(std::vector<Ptr<T>>& ptrVector, Ptr<T>&& ptrValue)
{
	ptrVector.push_back(std::move(ptrValue));
	return ptrVector[ptrVector.size() - 1].get();
}

void LoadTextureExecutorNode::Execute(ExecuteContext& context)
{
	SceneLoading::Loader l{};
	context.Variables.Textures[m_TextureKey] = AddToPtrVector(context.RenderResources.Textures, l.LoadTexture(m_TexturePath, Float4{ 1.0, 0.0f, 0.25f, 1.0f }));
}

void LoadShaderExecutorNode::Execute(ExecuteContext& context)
{
	Shader* shader = AddToPtrVector(context.RenderResources.Shaders, Shader::Compile(m_ShaderPath));
	if (!shader)
	{
		Failure("LoadShaderExecutorNode", "Failed to compile shader");
		context.Failure = true;
		return;
	}
	context.Variables.Shaders[m_ShaderKey] = shader;
}

void CreateTextureExecutorNode::Execute(ExecuteContext& context)
{
	context.Variables.Textures[m_TextureKey] = AddToPtrVector(context.RenderResources.Textures, Texture::Create(m_Width, m_Height, m_IsFramebuffer ? TF_Framebuffer : TF_None));
}

DrawMeshExecutorNode::~DrawMeshExecutorNode()
{
	if (m_VAO) GL_CALL(glDeleteVertexArrays(1, &m_VAO));
}

void DrawMeshExecutorNode::Execute(ExecuteContext& context)
{
	if (!m_FramebufferNode || !m_ShaderNode || !m_MeshNode)
	{
		Failure("DrawMeshExecutorNode", "Missing inputs");
		context.Failure = true;
		return;
	}

	Texture* framebuffer = m_FramebufferNode->GetValue(context);
	Shader* shader = m_ShaderNode->GetValue(context);
	Mesh* mesh = m_MeshNode->GetValue(context);

	if (!framebuffer || !shader || !mesh)
	{
		Failure("DrawMeshExecutorNode", "Invalid inputs");
		context.Failure = true;
		return;
	}

	if ((framebuffer->Flags & TF_Framebuffer) == 0)
	{
		Failure("DrawMeshExecutorNode", "Input framebuffer texture isn't created with framebuffer flag");
		context.Failure = true;
		return;
	}

	if (!m_VAO) m_VAO = CreateVAO(mesh, m_MeshNode->GetExtraInfo());

	// Framebuffer
	GL_CALL(glViewport(0, 0, framebuffer->Width, framebuffer->Height));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FrameBufferHandle));

	// Shader
	GL_CALL(glUseProgram(shader->Handle));

	// Mesh
	GL_CALL(glBindVertexArray(m_VAO));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->Indices->Handle));

	// Bindings
	BindTable* bindTable = nullptr;
	if (m_BindTable) bindTable = m_BindTable->GetValue(context);
	
	if (!TableBind(context, shader, bindTable))
	{
		Failure("DrawMeshExecutorNode", "Failed to bind BindTable!");
		context.Failure = true;
	}

	GL_CALL(glDrawElements(GL_TRIANGLES, mesh->NumPrimitives, GL_UNSIGNED_INT, 0));

	// ~Bindings
	TableUnbind(shader, bindTable);

	// ~Mesh
	GL_CALL(glBindVertexArray(0));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	// ~Shader
	GL_CALL(glUseProgram(0));
	
	// ~Framebuffer
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void LoadMeshExecutorNode::Execute(ExecuteContext& context)
{
	SceneLoading::Loader l{};
	Ptr<SceneLoading::Scene> scene = l.Load(m_MeshPath);

	if (scene->Objects.empty() || l.HasErrors())
	{
		l.PrintErrors();
		Failure("LoadMeshExecutorNode", "Failed to load scene");
		context.Failure = true;
		return;
	}

	auto& objectMesh = scene->Objects[0].Mesh;

	Mesh* mesh = new Mesh{};
	mesh->NumPrimitives = objectMesh.PrimitiveCount;
	mesh->Positions = std::move(objectMesh.Positions);
	mesh->Texcoords = std::move(objectMesh.Texcoords);
	mesh->Normals = std::move(objectMesh.Normals);
	mesh->Tangents = std::move(objectMesh.Tangents);
	mesh->Indices = std::move(objectMesh.Indices);

	context.Variables.Meshes[m_MeshKey] = AddToPtrVector(context.RenderResources.Meshes, Ptr<Mesh>(mesh));
}
