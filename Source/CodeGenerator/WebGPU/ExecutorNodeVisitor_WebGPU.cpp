#include "../../Execution/ExecutorNodeVisitor.h"

#include "../CodeGenerator.h"
#include "../../Execution/ExecutorNode.h"
#include "../../Execution/ValueNode.h"

void WebGLExecutorNodeVisitor::Visit(EmptyExecutorNode& node)
{
	// Do nothing...
}

void WebGLExecutorNodeVisitor::Visit(IfExecutorNode& node)
{
	// Hacky solution but is needed
	node.SetCondition(true);

	m_Generator.WriteKeyword("if");
	m_Generator.BeginExpression();
	node.GetConditionNode()->GenerateExpression(m_Generator, m_Context);
	m_Generator.EndExpression();
	m_Generator.BeginBlock();
	ExecuteNodes(node.GetNextNode());
	m_Generator.EndBlock();

	if (node.GetElseExecutorNode())
	{
		m_Generator.WriteKeyword("else");
		m_Generator.BeginBlock();
		ExecuteNodes(node.GetElseExecutorNode());
		m_Generator.EndBlock();
	}

	// Hacky solution but will terminate parent iteration
	node.SetNextNode(nullptr);
}

void WebGLExecutorNodeVisitor::Visit(PrintExecutorNode& node)
{
	m_Generator.FunctionCall("RenderNodeAPI.Print");
	m_Generator.FunctionArgumentsBegin();

	if (node.GetFloatValueNode())  node.GetFloatValueNode()->GenerateExpression(m_Generator, m_Context);
	if (node.GetFloat2ValueNode()) node.GetFloat2ValueNode()->GenerateExpression(m_Generator, m_Context);
	if (node.GetFloat3ValueNode()) node.GetFloat3ValueNode()->GenerateExpression(m_Generator, m_Context);
	if (node.GetFloat4ValueNode()) node.GetFloat4ValueNode()->GenerateExpression(m_Generator, m_Context);
	if (node.GetIntValueNode()) node.GetIntValueNode()->GenerateExpression(m_Generator, m_Context);
	if (node.GetBoolValueNode()) node.GetBoolValueNode()->GenerateExpression(m_Generator, m_Context);
	if (node.GetStringValueNode()) node.GetStringValueNode()->GenerateExpression(m_Generator, m_Context);

	m_Generator.FunctionArgumentsEnd();
	m_Generator.EndInstruction();
}

void WebGLExecutorNodeVisitor::Visit(ClearRenderTargetExecutorNode& node)
{
	m_Generator.FunctionCall("RenderNodeAPI.ClearFramebuffer");
	m_Generator.FunctionArgumentsBegin();

	node.GetTextureNode()->GenerateExpression(m_Generator, m_Context);
	m_Generator.ArgumentsSeparator();

	node.GetClearColorNode()->GenerateExpression(m_Generator, m_Context);

	m_Generator.FunctionArgumentsEnd();
	m_Generator.EndInstruction();
}

void WebGLExecutorNodeVisitor::Visit(AsignVariableExecutorNode& node)
{
	if (!node.GetVariableID())
	{
		ExecutionPrivate::Failure("AsignVariableExecutorNode", "Variable does not exist");
		m_Context.Failure = true;
		return;
	}

	const Variable& var = m_Context.VariablePool.GetRef(node.GetVariableID());

	if (var.Type == VariableType::Invalid)
	{
		ExecutionPrivate::Failure("AsignVariableExecutorNode", "Invalid variable type");
		m_Context.Failure = true;
		return;
	}

	m_Generator.WriteVariable(var);
	m_Generator.WriteOperator(CodeGenerator::Operator::Asign);

	switch (var.Type)
	{
	case VariableType::Bool:
		node.GetValueNode<bool>()->GenerateExpression(m_Generator, m_Context);
		break;
	case VariableType::Int:
		node.GetValueNode<int>()->GenerateExpression(m_Generator, m_Context);
		break;
	case VariableType::Float:
		node.GetValueNode<float>()->GenerateExpression(m_Generator, m_Context);
		break;
	case VariableType::Float2:
		node.GetValueNode<Float2>()->GenerateExpression(m_Generator, m_Context);
		break;
	case VariableType::Float3:
		node.GetValueNode<Float3>()->GenerateExpression(m_Generator, m_Context);
		break;
	case VariableType::Float4:
		node.GetValueNode<Float4>()->GenerateExpression(m_Generator, m_Context);
		break;
	case VariableType::Float4x4:
		node.GetValueNode<Float4x4>()->GenerateExpression(m_Generator, m_Context);
		break;
	case VariableType::Shader:
	case VariableType::Texture:
	case VariableType::Scene:
		NOT_IMPLEMENTED;
		break;
	default:
		m_Generator.WriteNull();
		break;
	}

	m_Generator.EndInstruction();
}

void WebGLExecutorNodeVisitor::Visit(PresentTextureExecutorNode& node)
{
	m_Generator.FunctionCall("RenderNodeAPI.PresentFramebuffer");
	m_Generator.FunctionArgumentsBegin();

	// Texture argument
	node.GetTextureNode()->GenerateExpression(m_Generator, m_Context);

	m_Generator.FunctionArgumentsEnd();
	m_Generator.EndInstruction();
}

void WebGLExecutorNodeVisitor::Visit(DrawMeshExecutorNode& node)
{
	m_Generator.FunctionCall("RenderNodeAPI.DrawMesh");
	m_Generator.FunctionArgumentsBegin();

	node.GetFramebufferNode()->GenerateExpression(m_Generator, m_Context);
	m_Generator.ArgumentsSeparator();

	node.GetShaderNode()->GenerateExpression(m_Generator, m_Context);
	m_Generator.ArgumentsSeparator();

	node.GetMeshNode()->GenerateExpression(m_Generator, m_Context);
	m_Generator.ArgumentsSeparator();

	if (node.GetRenderStateNode())
	{
		node.GetRenderStateNode()->GenerateExpression(m_Generator, m_Context);
	}
	else
	{
		m_Generator.WriteNull();
	}
	m_Generator.ArgumentsSeparator();

	if (node.GetBindTableNode())
	{
		node.GetBindTableNode()->GenerateExpression(m_Generator, m_Context);
	}
	else
	{
		m_Generator.WriteNull();
	}
	m_Generator.ArgumentsSeparator();

	const auto& vertexBits = node.GetMeshNode()->GetExtraInfo().MeshVertexBits;
	m_Generator.BeginInlineObject();
	m_Generator.ClassMemberDeclaration("Position");
	m_Generator.WriteConstant((bool)vertexBits.Position);
	m_Generator.ArgumentsSeparator();
	m_Generator.ClassMemberDeclaration("Texcoord");
	m_Generator.WriteConstant((bool)vertexBits.Texcoord);
	m_Generator.ArgumentsSeparator();
	m_Generator.ClassMemberDeclaration("Normal");
	m_Generator.WriteConstant((bool)vertexBits.Normal);
	m_Generator.ArgumentsSeparator();
	m_Generator.ClassMemberDeclaration("Tangent");
	m_Generator.WriteConstant((bool)vertexBits.Tangent);
	m_Generator.EndInlineObject();

	m_Generator.FunctionArgumentsEnd();
	m_Generator.EndInstruction();
}

void WebGLExecutorNodeVisitor::Visit(ForEachSceneObjectExecutorNode& node)
{
	m_Generator.WriteKeyword("for");
	m_Generator.BeginExpression();
	m_Generator.WriteKeyword("const");
	m_Generator.WriteVariable("it_" + std::to_string(node.GetIteratorPin()) + " ");
	m_Generator.WriteKeyword("of");
	node.GetSceneNode()->GenerateExpression(m_Generator, m_Context);
	m_Generator.ClassMemeberAccess("SceneObjects");
	m_Generator.EndExpression();
	m_Generator.BeginBlock();

	ExecuteNodes(node.GetLoopExecutorNode());

	m_Generator.EndBlock();
}