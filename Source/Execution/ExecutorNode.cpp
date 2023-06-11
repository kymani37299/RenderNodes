#include "ExecutorNode.h"

#include "../Common.h"

void IfExecutorNode::Execute(ExecuteContext& context)
{
	m_PassedCondition = m_Condition->GetValue(context);
}

void PrintExecutorNode::Execute(ExecuteContext& context)
{
	std::cout << m_FloatNode->GetValue(context) << std::endl;
}