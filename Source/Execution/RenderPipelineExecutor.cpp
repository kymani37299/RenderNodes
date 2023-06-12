#include "RenderPipelineExecutor.h"

#include "ExecutorNode.h"
#include "../Render/Texture.h"

void RenderPipelineExecutor::OnStart()
{
	// Init context
	m_Context = ExecuteContext{};
	m_Context.RenderTarget = Texture::Create(1024, 768, TF_Framebuffer);

	ExecutorNode* node = m_Pipeline.OnStartNode;
	while (node)
	{
		node->Execute(m_Context);
		node = node->GetNextNode();
	}
}

void RenderPipelineExecutor::OnUpdate(float dt)
{
	ExecutorNode* node = m_Pipeline.OnUpdateNode;
	const std::string dtVar = "dt";
	m_Context.Variables.GetMapFromType<float>()[Hash::Crc32(dtVar)] = dt;
	while (node)
	{
		node->Execute(m_Context);
		node = node->GetNextNode();
	}
}

void RenderPipelineExecutor::Render()
{
	ImGui::Image((ImTextureID) m_Context.RenderTarget->TextureHandle, ImVec2{ 1024, 768 });
}
