#include "RenderPipelineExecutor.h"

#include "ExecutorNode.h"

void RenderPipelineExecutor::OnStart()
{
	ExecutorNode* node = m_Pipeline.OnStartNode;
	while (node)
	{
		node->Execute();
		node = node->GetNextNode();
	}
}

void RenderPipelineExecutor::OnUpdate()
{
	ExecutorNode* node = m_Pipeline.OnUpdateNode;
	while (node)
	{
		node->Execute();
		node = node->GetNextNode();
	}
}