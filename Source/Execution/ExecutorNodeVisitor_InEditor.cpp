#include "ExecutorNodeVisitor.h"

#include <glm/gtc/type_ptr.hpp>

#include "../App/App.h"
#include "../Common.h"
#include "../Render/Texture.h"
#include "../Render/Shader.h"

#include "ExecutorNode.h"
#include "RenderStructs.h"

namespace ExecutionPrivate
{
	void Failure(const std::string& nodeName, const std::string& msg)
	{
		App::Get()->GetConsole().Log("<red>[FAILURE] </red>[" + nodeName + "] " + msg);
	}

	void Warning(bool condition, const std::string& nodeName, const std::string& msg)
	{
		if (!condition)
		{
			App::Get()->GetConsole().Log("<yellow>[WARN] </yellow>[" + nodeName + "] " + msg);
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

		for (const auto& binding : bindTable->Textures)
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
				GL_CALL(glUniformMatrix4fv(uniformSlot, 1, GL_FALSE, glm::value_ptr(glm::transpose(value))));
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

		if (depthTestEnabled)
		{
			glDepthMask(renderState.DepthWrite ? GL_TRUE : GL_FALSE);
			glDepthFunc(renderState.DepthTest);
		}
	}
}

namespace
{
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
}

DrawMeshExecutorNode::~DrawMeshExecutorNode()
{
	if (m_VAO) GL_CALL(glDeleteVertexArrays(1, &m_VAO));
}

void InEditorExecutorNodeVisitor::Visit(EmptyExecutorNode& node)
{
	// Do nothing...
}

void InEditorExecutorNodeVisitor::Visit(IfExecutorNode& node)
{
	node.SetCondition(node.GetConditionNode()->GetValue(m_Context));
}

void InEditorExecutorNodeVisitor::Visit(PrintExecutorNode& node)
{
	if (node.GetFloatValueNode())  App::Get()->GetConsole().Log(std::to_string(node.GetFloatValueNode()->GetValue(m_Context)));
	if (node.GetFloat2ValueNode()) App::Get()->GetConsole().Log(ToString(node.GetFloat2ValueNode()->GetValue(m_Context)));
	if (node.GetFloat3ValueNode()) App::Get()->GetConsole().Log(ToString(node.GetFloat3ValueNode()->GetValue(m_Context)));
	if (node.GetFloat4ValueNode()) App::Get()->GetConsole().Log(ToString(node.GetFloat4ValueNode()->GetValue(m_Context)));
	if (node.GetIntValueNode()) App::Get()->GetConsole().Log(std::to_string(node.GetIntValueNode()->GetValue(m_Context)));
	if (node.GetBoolValueNode()) App::Get()->GetConsole().Log(node.GetBoolValueNode()->GetValue(m_Context) ? "true" : "false");
	if (node.GetStringValueNode()) App::Get()->GetConsole().Log(node.GetStringValueNode()->GetValue(m_Context));
}

void InEditorExecutorNodeVisitor::Visit(ClearRenderTargetExecutorNode& node)
{
	const Float4 clearColor = node.GetClearColorNode()->GetValue(m_Context);
	const Texture* texture = node.GetTextureNode()->GetValue(m_Context);
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

template<typename T>
void AsignVariable(Variable& variable, AsignVariableExecutorNode& node, ExecuteContext& context)
{
	variable.Get<T>() = node.GetValueNode<T>()->GetValue(context);
}

void InEditorExecutorNodeVisitor::Visit(AsignVariableExecutorNode& node)
{
	if (!node.GetVariableID())
	{
		ExecutionPrivate::Failure("AsignVariableExecutorNode", "Variable name not defined");
		m_Context.Failure = true;
		return;
	}

	Variable& var = m_Context.VariablePool.GetRef(node.GetVariableID());
	switch (var.Type)
	{
	case VariableType::Invalid:
		ExecutionPrivate::Failure("AsignVariableExecutorNode", "Variable name not defined");
		m_Context.Failure = true;
		break;
	case VariableType::Bool:
		AsignVariable<bool>(var, node, m_Context);
		break;
	case VariableType::Int:
		AsignVariable<int>(var, node, m_Context);
		break;
	case VariableType::Float:
		AsignVariable<float>(var, node, m_Context);
		break;
	case VariableType::Float2:
		AsignVariable<Float2>(var, node, m_Context);
		break;
	case VariableType::Float3:
		AsignVariable<Float3>(var, node, m_Context);
		break;
	case VariableType::Float4:
		AsignVariable<Float4>(var, node, m_Context);
		break;
	case VariableType::Float4x4:
		AsignVariable<Float4x4>(var, node, m_Context);
		break;
	case VariableType::Shader:
	case VariableType::Texture:
	case VariableType::Scene:
	default:
		NOT_IMPLEMENTED;
	}
}

void InEditorExecutorNodeVisitor::Visit(PresentTextureExecutorNode& node)
{
	Texture* texture = node.GetTextureNode()->GetValue(m_Context);
	Warning(texture, "PresentTextureExecutorNode", "Input texture is null");
	m_Context.RenderTarget = texture;
}

void InEditorExecutorNodeVisitor::Visit(DrawMeshExecutorNode& node)
{
	if (!node.GetFramebufferNode() || !node.GetShaderNode() || !node.GetMeshNode())
	{
		Failure("DrawMeshExecutorNode", "Missing inputs");
		m_Context.Failure = true;
		return;
	}

	Texture* framebuffer = node.GetFramebufferNode()->GetValue(m_Context);
	Shader* shader = node.GetShaderNode()->GetValue(m_Context);
	Mesh* mesh = node.GetMeshNode()->GetValue(m_Context);

	if (!framebuffer || !shader || !mesh)
	{
		Failure("DrawMeshExecutorNode", "Invalid inputs");
		m_Context.Failure = true;
		return;
	}

	if ((framebuffer->Flags & TF_Framebuffer) == 0)
	{
		Failure("DrawMeshExecutorNode", "Input framebuffer texture isn't created with framebuffer flag");
		m_Context.Failure = true;
		return;
	}

	if (!node.GetVAO()) node.SetVAO(CreateVAO(mesh, node.GetMeshNode()->GetExtraInfo()));

	// Framebuffer
	GL_CALL(glViewport(0, 0, framebuffer->Width, framebuffer->Height));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FrameBufferHandle));

	// Render state
	RenderState renderState;
	if (node.GetRenderStateNode()) renderState = node.GetRenderStateNode()->GetValue(m_Context);
	RenderStateBind(renderState);

	// Shader
	GL_CALL(glUseProgram(shader->Handle));

	// Mesh
	GL_CALL(glBindVertexArray(node.GetVAO()));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->Indices->Handle));

	// Bindings
	BindTable* bindTable = nullptr;
	if (node.GetBindTableNode()) bindTable = node.GetBindTableNode()->GetValue(m_Context);
	TableBind(m_Context, shader, bindTable);

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

void InEditorExecutorNodeVisitor::Visit(ForEachSceneObjectExecutorNode& node)
{
	Scene* scene = node.GetSceneNode()->GetValue(m_Context);
	for (SceneObject& sceneObject : scene->SceneObjects)
	{
		m_Context.Iterators.Data[node.GetIteratorPin()] = &sceneObject;

		InEditorExecutorNodeVisitor visitor{ m_Context };
		visitor.ExecuteNodes(node.GetLoopExecutorNode());
	}
}