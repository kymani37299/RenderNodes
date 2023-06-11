#include "ExecutorNode.h"

#include "../Common.h"

void IfExecutorNode::Execute()
{
	m_PassedCondition = m_Condition->GetValue();
}

void PrintExecutorNode::Execute()
{
	std::cout << m_FloatNode->GetValue() << std::endl;
}