#include "ExecutorNode.h"

#include "../Common.h"

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

	glBindFramebuffer(GL_FRAMEBUFFER, context.RenderTarget->FrameBufferHandle);
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
