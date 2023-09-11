#include "ExecutorNode.h"

#include <glm/gtc/type_ptr.hpp>

#include "../App/App.h"
#include "../Common.h"
#include "../Render/Texture.h"
#include "../Render/Shader.h"
#include "../Render/SceneLoading.h"

namespace ExecutionPrivate
{
	void Failure(const std::string& nodeName, const std::string& msg)
	{
		App::Get()->GetConsole().Log("[FAILURE] [" + nodeName + "] " + msg);
	}

	void Warning(bool condition, const std::string& nodeName, const std::string& msg)
	{
		if (!condition)
		{
			App::Get()->GetConsole().Log("[WARN] [" + nodeName + "] " + msg);
		}
	}

	void Warning(const std::string& nodeName, const std::string& msg)
	{
		Warning(false, nodeName, msg);
	}
}

using namespace ExecutionPrivate;

namespace
{
	unsigned CreateVAO(Mesh* mesh, const ValueNodeExtraInfo& extraInfo)
	{
		unsigned vao;
		GL_CALL(glGenVertexArrays(1, &vao));
		GL_CALL(glBindVertexArray(vao));

		unsigned nextAttribArray = 0;
		const auto& vertexBits = extraInfo.MeshVertexBits;

		if (vertexBits.Position)
		{
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->Positions->Handle));
			GL_CALL(glVertexAttribPointer(nextAttribArray, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
			GL_CALL(glEnableVertexAttribArray(nextAttribArray++));
		}

		if (vertexBits.Texcoord)
		{
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->Texcoords->Handle));
			GL_CALL(glVertexAttribPointer(nextAttribArray, 2, GL_FLOAT, GL_FALSE, 0, (void*)0));
			GL_CALL(glEnableVertexAttribArray(nextAttribArray++));
		}

		if (vertexBits.Normal)
		{
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->Normals->Handle));
			GL_CALL(glVertexAttribPointer(nextAttribArray, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
			GL_CALL(glEnableVertexAttribArray(nextAttribArray++));
		}

		if (vertexBits.Tangent)
		{
			GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->Tangents->Handle));
			GL_CALL(glVertexAttribPointer(nextAttribArray, 4, GL_FLOAT, GL_FALSE, 0, (void*)0));
			GL_CALL(glEnableVertexAttribArray(nextAttribArray++));
		}

		return vao;
	}

	bool GetUniformIndex(Shader* shader, const std::string& name, unsigned& uniformIndex)
	{
		uniformIndex = glGetUniformLocation(shader->Handle, name.c_str());
		return uniformIndex != -1;
	}

	void TableBind(ExecuteContext& context, Shader* shader, BindTable* bindTable)
	{
		if (!bindTable)
		{
			return;
		}

		for(const auto& binding : bindTable->Textures)
		{
			Texture* texture = binding.Value.get() ? binding.Value->GetValue(context) : (Texture*) nullptr;
			if (!texture) continue;

			int textureSlot = std::stoi(binding.Name);
			if (textureSlot >= 0 && textureSlot < 32)
			{
				GL_CALL(glActiveTexture(GL_TEXTURE0 + textureSlot));
				GL_CALL(glBindTexture(GL_TEXTURE_2D, texture->TextureHandle));
			}
			else
			{
				Warning("TableBind", "Invalid texture binding " + binding.Name + ". Valid bindings are in range [0,31]");
			}
		}

		for (const auto& binding : bindTable->Floats)
		{
			const float value = binding.Value.get() ? binding.Value->GetValue(context) : 0.0f;

			unsigned uniformSlot;
			if (GetUniformIndex(shader, binding.Name, uniformSlot))
			{
				GL_CALL(glUniform1f(uniformSlot, value));
			}
			else
			{
				Warning("TableBind", "Unable to find " + binding.Name + " binding in shader");
			}
		}

		for (const auto& binding : bindTable->Float2s)
		{
			const Float2 value = binding.Value.get() ? binding.Value->GetValue(context) : Float2{};

			unsigned uniformSlot;
			if (GetUniformIndex(shader, binding.Name, uniformSlot))
			{
				GL_CALL(glUniform2fv(uniformSlot, 1, glm::value_ptr(value)));
			}
			else
			{
				Warning("TableBind", "Unable to find " + binding.Name + " binding in shader");
			}
		}

		for (const auto& binding : bindTable->Float3s)
		{
			const Float3 value = binding.Value.get() ? binding.Value->GetValue(context) : Float3{};

			unsigned uniformSlot;
			if (GetUniformIndex(shader, binding.Name, uniformSlot))
			{
				GL_CALL(glUniform3fv(uniformSlot, 1, glm::value_ptr(value)));
			}
			else
			{
				Warning("TableBind", "Unable to find " + binding.Name + " binding in shader");
			}
		}

		for (const auto& binding : bindTable->Float4s)
		{
			const Float4 value = binding.Value.get() ? binding.Value->GetValue(context) : Float4{};

			unsigned uniformSlot;
			if (GetUniformIndex(shader, binding.Name, uniformSlot))
			{
				GL_CALL(glUniform4fv(uniformSlot, 1, glm::value_ptr(value)));
			}
			else
			{
				Warning("TableBind", "Unable to find " + binding.Name + " binding in shader");
			}
		}

		for (const auto& binding : bindTable->Float4x4s)
		{
			const Float4x4 value = binding.Value.get() ? binding.Value->GetValue(context) : glm::identity<Float4x4>();

			unsigned uniformSlot;
			if (GetUniformIndex(shader, binding.Name, uniformSlot))
			{
				GL_CALL(glUniformMatrix4fv(uniformSlot, 1, GL_FALSE, glm::value_ptr(value)));
			}
			else
			{
				Warning("TableBind", "Unable to find " + binding.Name + " binding in shader");
			}
		}
	}

	void TableUnbind(Shader* shader, BindTable* bindTable)
	{
		if (!bindTable) return;

		for (unsigned i = 0; i < bindTable->Textures.size(); i++)
		{
			const auto& binding = bindTable->Textures[i];
			int textureSlot = std::stoi(binding.Name);
			if (textureSlot >= 0 && textureSlot < 32)
			{
				GL_CALL(glActiveTexture(GL_TEXTURE0 + textureSlot));
				GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
			}
		}
	}

	void RenderStateBind(const RenderState& renderState)
	{
		const bool depthTestEnabled = renderState.DepthWrite || renderState.DepthTest != GL_ALWAYS;

		if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);

		if(depthTestEnabled)
		{
			glDepthMask(renderState.DepthWrite ? GL_TRUE : GL_FALSE);
			glDepthFunc(renderState.DepthTest);
		}
	}
}

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
	if (m_FloatNode)  App::Get()->GetConsole().Log(std::to_string(m_FloatNode->GetValue(context)));
	if (m_Float2Node) App::Get()->GetConsole().Log(ToString(m_Float2Node->GetValue(context)));
	if (m_Float3Node) App::Get()->GetConsole().Log(ToString(m_Float3Node->GetValue(context)));
	if (m_Float4Node) App::Get()->GetConsole().Log(ToString(m_Float4Node->GetValue(context)));
	if (m_IntNode) App::Get()->GetConsole().Log(std::to_string(m_IntNode->GetValue(context)));
	if (m_BoolNode) App::Get()->GetConsole().Log(m_BoolNode->GetValue(context) ? "true" : "false");
	if (m_StringNode) App::Get()->GetConsole().Log(m_StringNode->GetValue(context));
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
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
}

void PresentTextureExecutorNode::Execute(ExecuteContext& context)
{
	Texture* texture = m_Texture->GetValue(context);
	Warning(texture, "PresentTextureExecutorNode", "Input texture is null");
	Warning(texture && texture->Flags & TF_Framebuffer, "PresentTextureExecutorNode", "Trying to present texture that doesn't have framebuffer enabled");
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
	if (!m_NameNode)
	{
		Failure("LoadTextureExecutorNode", "Failed to load texture");
		context.Failure = true;
	}

	const std::string name = m_NameNode->GetValue(context);
	const unsigned varKey = Hash::Crc32(name);

	SceneLoading::Loader l{};
	context.Variables.Textures[varKey] = AddToPtrVector(context.RenderResources.Textures, l.LoadTexture(m_TexturePath, Float4{ 1.0, 0.0f, 0.25f, 1.0f }));
}

void LoadShaderExecutorNode::Execute(ExecuteContext& context)
{
	Shader* shader = AddToPtrVector(context.RenderResources.Shaders, Shader::Compile(m_ShaderPath));
	if (!shader || !m_NameNode)
	{
		Failure("LoadShaderExecutorNode", "Failed to compile shader");
		context.Failure = true;
		return;
	}

	const std::string name = m_NameNode->GetValue(context);
	const unsigned varKey = Hash::Crc32(name);

	context.Variables.Shaders[varKey] = shader;
}

void CreateTextureExecutorNode::Execute(ExecuteContext& context)
{
	if (!m_NameNode)
	{
		Failure("CreateTextureExecutorNode", "Failed to get texture name");
		context.Failure = true;
		return;
	}

	const std::string name = m_NameNode->GetValue(context);
	const unsigned varKey = Hash::Crc32(name);

	const int width = m_WidthNode->GetValue(context);
	const int height = m_HeightNode->GetValue(context);
	const bool isFramebuffer = m_FramebufferNode->GetValue(context);
	const bool isDepthStencil = m_DepthStencilNode->GetValue(context);

	unsigned textureFlags = TF_None;
	if (isFramebuffer) textureFlags |= TF_Framebuffer;
	if (isDepthStencil) textureFlags |= TF_DepthStencil;
	context.Variables.Textures[varKey] = AddToPtrVector(context.RenderResources.Textures, Texture::Create(width, height, textureFlags));
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

	// Render state
	RenderState renderState;
	if (m_RenderState) renderState = m_RenderState->GetValue(context);
	RenderStateBind(renderState);

	// Shader
	GL_CALL(glUseProgram(shader->Handle));

	// Mesh
	GL_CALL(glBindVertexArray(m_VAO));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->Indices->Handle));

	// Bindings
	BindTable* bindTable = nullptr;
	if (m_BindTable) bindTable = m_BindTable->GetValue(context);
	TableBind(context, shader, bindTable);

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

	if (scene->Objects.empty() || l.HasErrors() || !m_NameNode)
	{
		l.PrintErrors();
		Failure("LoadMeshExecutorNode", "Failed to load scene");
		context.Failure = true;
		return;
	}

	const std::string name = m_NameNode->GetValue(context);
	const unsigned varKey = Hash::Crc32(name);

	auto& objectMesh = scene->Objects[0].Mesh;

	Mesh* mesh = new Mesh{};
	mesh->NumPrimitives = objectMesh.PrimitiveCount;
	mesh->Positions = std::move(objectMesh.Positions);
	mesh->Texcoords = std::move(objectMesh.Texcoords);
	mesh->Normals = std::move(objectMesh.Normals);
	mesh->Tangents = std::move(objectMesh.Tangents);
	mesh->Indices = std::move(objectMesh.Indices);

	context.Variables.Meshes[varKey] = AddToPtrVector(context.RenderResources.Meshes, Ptr<Mesh>(mesh));
}
