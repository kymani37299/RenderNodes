#include "ExecutorNode.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../Common.h"
#include "../Render/Texture.h"
#include "../Render/Shader.h"

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

using namespace ExecutionPrivate;

void IfExecutorNode::Execute(ExecuteContext& context)
{
	m_PassedCondition = m_Condition->GetValue(context);
}

std::string ToString(const glm::vec2& value)
{
	return "(" + std::to_string(value.x) + ",  " + std::to_string(value.y) + ")";
}

std::string ToString(const glm::vec3& value)
{
	return "(" + std::to_string(value.x) + ",  " + std::to_string(value.y) + ",  " + std::to_string(value.z) + ")";
}

std::string ToString(const glm::vec4& value)
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
	const glm::vec4 clearColor = m_ClearColorNode->GetValue(context);
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
	static unsigned char INVALID_TEXTURE_COLOR[] = { 0xff, 0x00, 0x33, 0xff };

	int width, height, bpp;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(m_TexturePath.c_str(), &width, &height, &bpp, 3);
	if (!data)
	{
		Warning(false, "LoadTextureExecutorNode", "Failed to load texture");
		data = INVALID_TEXTURE_COLOR;
		width = 1;
		height = 1;
		// bpp = 4; ?
	}

	context.Variables.Textures[m_TextureKey] = AddToPtrVector(context.RenderResources.Textures, Texture::Create(width, height, TF_None, data));

	stbi_image_free(data);
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

	if (!m_VAO)
	{
		GL_CALL(glGenVertexArrays(1, &m_VAO));
		GL_CALL(glBindVertexArray(m_VAO));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->VertexBuffer->Handle));
		GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
		GL_CALL(glEnableVertexAttribArray(0));
	}

	GL_CALL(glViewport(0, 0, framebuffer->Width, framebuffer->Height));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FrameBufferHandle));
	GL_CALL(glUseProgram(shader->Handle));
	GL_CALL(glBindVertexArray(m_VAO));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IndexBuffer->Handle));

	GL_CALL(glDrawElements(GL_TRIANGLES, mesh->NumPrimitives, GL_UNSIGNED_INT, 0));

	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_CALL(glUseProgram(0));
	GL_CALL(glBindVertexArray(0));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
