#include "PinEvaluator.h"

static EditorNodePin GetOutputPinIfInput(const NodeGraph& nodeGraph, EditorNodePin pin)
{
	if (pin.IsInput)
	{
		const PinID outPinID = nodeGraph.GetOutputPinForInput(pin.ID);
		if (!outPinID) return EditorNodePin{};
		pin = nodeGraph.GetPinByID(outPinID);
	}
	return pin;
}

BoolValueNode* PinEvaluator::EvaluateBool(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Bool);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Bool pin input missing link!");
		return new ConstantValueNode<bool>(false);
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Bool: return EvaluateBool(static_cast<BoolEditorNode*>(node));
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateBoolPin] internal error!");
	}
	return new ConstantValueNode<bool>(false);
}

FloatValueNode* PinEvaluator::EvaluateFloat(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Float);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float pin input missing link!");
		return new ConstantValueNode<float>(0.0f);
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::OnUpdate: return new VariableValueNode<float>("dt");
	case EditorNodeType::Float: return EvaluateFloat(static_cast<FloatEditorNode*>(node));
	case EditorNodeType::FloatBinaryOperator: return EvaluateFloatBinaryOperator(static_cast<FloatBinaryOperatorEditorNode*>(node));
	case EditorNodeType::VarFloat: return EvaluateVarFloat(static_cast<VarFloatEditorNode*>(node));
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<float>(0.0f);
}

Float2ValueNode* PinEvaluator::EvaluateFloat2(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Float2);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float2 pin input missing link!");
		return new ConstantValueNode<glm::vec2>({});
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Float2: return EvaluateFloat2(static_cast<Float2EditorNode*>(node));
	case EditorNodeType::VarFloat2: return EvaluateVarFloat2(static_cast<VarFloat2EditorNode*>(node));
	case EditorNodeType::Float2BinaryOperator: return EvaluateFloat2BinaryOperator(static_cast<Float2BinaryOperatorEditorNode*>(node));
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<glm::vec2>({});
}

Float3ValueNode* PinEvaluator::EvaluateFloat3(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Float3);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float3 pin input missing link!");
		return new ConstantValueNode<glm::vec3>({});
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Float3: return EvaluateFloat3(static_cast<Float3EditorNode*>(node));
	case EditorNodeType::VarFloat3: return EvaluateVarFloat3(static_cast<VarFloat3EditorNode*>(node));
	case EditorNodeType::Float3BinaryOperator: return EvaluateFloat3BinaryOperator(static_cast<Float3BinaryOperatorEditorNode*>(node));
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<glm::vec3>({});
}

Float4ValueNode* PinEvaluator::EvaluateFloat4(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Float4);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float4 pin input missing link!");
		return new ConstantValueNode<glm::vec4>({});
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Float4: return EvaluateFloat4(static_cast<Float4EditorNode*>(node));
	case EditorNodeType::VarFloat4: return EvaluateVarFloat4(static_cast<VarFloat4EditorNode*>(node));
	case EditorNodeType::Float4BinaryOperator: return EvaluateFloat4BinaryOperator(static_cast<Float4BinaryOperatorEditorNode*>(node));
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<glm::vec4>({});
}

TextureValueNode* PinEvaluator::EvaluateTexture(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Texture);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Texture pin input not linked!");
		return new ConstantValueNode<Texture*>(nullptr);
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::GetTexture: return EvaluateGetTexture(static_cast<GetTextureEditorNode*>(node));
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Texture*>(nullptr);
}

BufferValueNode* PinEvaluator::EvaluateBuffer(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Buffer);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Buffer pin input not linked!");
		return new ConstantValueNode<Buffer*>(nullptr);
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Buffer*>(nullptr);
}

MeshValueNode* PinEvaluator::EvaluateMesh(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Mesh);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Mesh pin input not linked!");
		return new ConstantValueNode<Mesh*>(nullptr);
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::GetCubeMesh: return EvaluateGetCubeMesh(static_cast<GetCubeMeshEditorNode*>(node));
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Mesh*>(nullptr);
}

ShaderValueNode* PinEvaluator::EvaluateShader(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Shader);

	pin = GetOutputPinIfInput(m_NodeGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Shader pin input not linked!");
		return new ConstantValueNode<Shader*>(nullptr);
	}

	EditorNode* node = m_NodeGraph.GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::GetShader: return EvaluateGetShader(static_cast<GetShaderEditorNode*>(node));
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Shader*>(nullptr);
}

BoolValueNode* PinEvaluator::EvaluateBool(BoolEditorNode* boolNode)
{
	return new ConstantValueNode<bool>(boolNode->GetValue());
}

FloatValueNode* PinEvaluator::EvaluateFloat(FloatEditorNode* floatNode)
{
	return new ConstantValueNode<float>(floatNode->GetFloat());
}

FloatValueNode* PinEvaluator::EvaluateVarFloat(VarFloatEditorNode* floatNode)
{
	return new VariableValueNode<float>(floatNode->GetVaraibleName());
}

FloatValueNode* PinEvaluator::EvaluateFloatBinaryOperator(FloatBinaryOperatorEditorNode* floatOpNode)
{
	FloatValueNode* a = EvaluateFloat(floatOpNode->GetAPin());
	FloatValueNode* b = EvaluateFloat(floatOpNode->GetBPin());
	return new BinaryOperatorValueNode<float>{ a, b, floatOpNode->GetOp() };
}

Float2ValueNode* PinEvaluator::EvaluateFloat2(Float2EditorNode* floatNode)
{
	return new ConstantValueNode<glm::vec2>(floatNode->GetFloat2());
}

Float2ValueNode* PinEvaluator::EvaluateVarFloat2(VarFloat2EditorNode* floatNode)
{
	return new VariableValueNode<glm::vec2>(floatNode->GetVaraibleName());
}

Float2ValueNode* PinEvaluator::EvaluateFloat2BinaryOperator(Float2BinaryOperatorEditorNode* floatOpNode)
{
	Float2ValueNode* a = EvaluateFloat2(floatOpNode->GetAPin());
	Float2ValueNode* b = EvaluateFloat2(floatOpNode->GetBPin());
	return new BinaryOperatorValueNode<glm::vec2>{ a, b, floatOpNode->GetOp() };
}

Float3ValueNode* PinEvaluator::EvaluateFloat3(Float3EditorNode* floatNode)
{
	return new ConstantValueNode<glm::vec3>(floatNode->GetFloat3());
}

Float3ValueNode* PinEvaluator::EvaluateVarFloat3(VarFloat3EditorNode* floatNode)
{
	return new VariableValueNode<glm::vec3>(floatNode->GetVaraibleName());
}

Float3ValueNode* PinEvaluator::EvaluateFloat3BinaryOperator(Float3BinaryOperatorEditorNode* floatOpNode)
{
	Float3ValueNode* a = EvaluateFloat3(floatOpNode->GetAPin());
	Float3ValueNode* b = EvaluateFloat3(floatOpNode->GetBPin());
	return new BinaryOperatorValueNode<glm::vec3>{ a, b, floatOpNode->GetOp() };
}

Float4ValueNode* PinEvaluator::EvaluateFloat4(Float4EditorNode* floatNode)
{
	return new ConstantValueNode<glm::vec4>(floatNode->GetFloat4());
}

Float4ValueNode* PinEvaluator::EvaluateVarFloat4(VarFloat4EditorNode* floatNode)
{
	return new VariableValueNode<glm::vec4>(floatNode->GetVaraibleName());
}

Float4ValueNode* PinEvaluator::EvaluateFloat4BinaryOperator(Float4BinaryOperatorEditorNode* floatOpNode)
{
	Float4ValueNode* a = EvaluateFloat4(floatOpNode->GetAPin());
	Float4ValueNode* b = EvaluateFloat4(floatOpNode->GetBPin());
	return new BinaryOperatorValueNode<glm::vec4>{ a, b, floatOpNode->GetOp() };
}

TextureValueNode* PinEvaluator::EvaluateGetTexture(GetTextureEditorNode* getTextureNode)
{
	return new VariableValueNode<Texture*>(getTextureNode->GetVaraibleName());
}

MeshValueNode* PinEvaluator::EvaluateGetCubeMesh(GetCubeMeshEditorNode* getCubeMeshNode)
{
	return new StaticResourceNode<Mesh*, ExecutorStaticResource::CubeMesh>();
}

ShaderValueNode* PinEvaluator::EvaluateGetShader(GetShaderEditorNode* getShaderNode)
{
	return new VariableValueNode<Shader*>(getShaderNode->GetVaraibleName());
}
