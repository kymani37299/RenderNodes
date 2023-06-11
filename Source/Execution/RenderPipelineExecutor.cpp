#include "RenderPipelineExecutor.h"

#include "ExecutorNode.h"

void RenderPipelineExecutor::OnStart()
{
	m_Context = ExecuteContext{};

	ExecutorNode* node = m_Pipeline.OnStartNode;
	while (node)
	{
		node->Execute(m_Context);
		node = node->GetNextNode();
	}
}

void RenderPipelineExecutor::OnUpdate()
{
	ExecutorNode* node = m_Pipeline.OnUpdateNode;
	while (node)
	{
		node->Execute(m_Context);
		node = node->GetNextNode();
	}
}