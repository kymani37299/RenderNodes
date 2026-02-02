#include "PinEvaluator.h"

template<typename VariableTypeT>
ValueNode<VariableTypeT>* EvaluateVariable(VariableEditorNode* node)
{
	return new VariableValueNode<VariableTypeT>(node->GetVariableID());
}

template<typename RenderResourceTypeT>
ValueNode<RenderResourceTypeT>* EvaluateRenderResourceVariable(VariableEditorNode* node)
{
	return new RenderResourceVariableValueNodeT<RenderResourceTypeT>(node->GetVariableID());
}

BoolValueNode* PinEvaluator::EvaluateBool(EditorNodePin pin)
{
	if (pin.HasConstantValue) return new ConstantValueNode<bool>{ pin.ConstantValue.B };

	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Bool);
	
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Bool pin input missing link!");
		return new ConstantValueNode<bool>(false);
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Bool: return EvaluateBool(static_cast<BoolEditorNode*>(node));
	case EditorNodeType::Variable: return EvaluateVariable<bool>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::BoolBinaryOperator: return EvaluateBoolBinaryOperator(static_cast<BoolBinaryOperatorEditorNode*>(node));
	case EditorNodeType::IntComparisonOperator: return EvaluateIntComparisonOperator(static_cast<IntComparisonOperatorEditorNode*>(node));
	case EditorNodeType::FloatComparisonOperator: return EvaluateFloatComparisonOperator(static_cast<FloatComparisonOperatorEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<BoolValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<BoolValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateBoolPin] internal error!");
	}
	return new ConstantValueNode<bool>(false);
}

IntValueNode* PinEvaluator::EvaluateInt(EditorNodePin pin)
{
	if (pin.HasConstantValue) return new ConstantValueNode<int>{ pin.ConstantValue.I };

	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Int);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Int pin input missing link!");
		return new ConstantValueNode<int>(0);
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Int: return EvaluateInt(static_cast<IntEditorNode*>(node));
	case EditorNodeType::Variable: return EvaluateVariable<int>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::IntBinaryOperator: return EvaluateIntBinaryOperator(static_cast<IntBinaryOperatorEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<IntValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<IntValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateIntPin] internal error!");
	}
	return new ConstantValueNode<int>(0);
}

StringValueNode* PinEvaluator::EvaluateString(EditorNodePin pin)
{
	if (pin.HasConstantValue) return new ConstantValueNode<std::string>{ pin.ConstantValue.STR };
	
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::String);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("String pin input missing link!");
		return new ConstantValueNode<std::string>("");
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::String: return EvaluateString(static_cast<StringEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<StringValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<StringValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateBoolPin] internal error!");
	}
	return new ConstantValueNode<std::string>("");
}

FloatValueNode* PinEvaluator::EvaluateFloat(EditorNodePin pin)
{
	if (pin.HasConstantValue) return new ConstantValueNode<float>{ pin.ConstantValue.F };
	
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Float);
	
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float pin input missing link!");
		return new ConstantValueNode<float>(0.0f);
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::OnUpdate: return EvaluateDeltaTime(static_cast<OnUpdateEditorNode*>(node));
	case EditorNodeType::Float: return EvaluateFloat(static_cast<FloatEditorNode*>(node));
	case EditorNodeType::FloatBinaryOperator: return EvaluateFloatBinaryOperator(static_cast<FloatBinaryOperatorEditorNode*>(node));
	case EditorNodeType::Variable: return EvaluateVariable<float>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::SplitFloat2: return EvaluateSplitFloat2(static_cast<SplitFloat2EditorNode*>(node), pin);
	case EditorNodeType::SplitFloat3: return EvaluateSplitFloat3(static_cast<SplitFloat3EditorNode*>(node), pin);
	case EditorNodeType::SplitFloat4: return EvaluateSplitFloat4(static_cast<SplitFloat4EditorNode*>(node), pin);
	case EditorNodeType::Pin: return EvaluatePinNode<FloatValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<FloatValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<float>(0.0f);
}

Float2ValueNode* PinEvaluator::EvaluateFloat2(EditorNodePin pin)
{
	if (pin.HasConstantValue) return new ConstantValueNode<Float2>{ pin.ConstantValue.F2 };
	
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Float2);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float2 pin input missing link!");
		return new ConstantValueNode<Float2>({});
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Float2: return EvaluateFloat2(static_cast<Float2EditorNode*>(node));
	case EditorNodeType::CreateFloat2: return EvaluateCreateFloat2(static_cast<CreateFloat2EditorNode*>(node));
	case EditorNodeType::Variable: return EvaluateVariable<Float2>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::Float2BinaryOperator: return EvaluateFloat2BinaryOperator(static_cast<Float2BinaryOperatorEditorNode*>(node));
	case EditorNodeType::NormalizeFloat2: return EvaluateNormalizeFloat2(static_cast<NormalizeFloat2EditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<Float2ValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<Float2ValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Float2>({});
}

Float3ValueNode* PinEvaluator::EvaluateFloat3(EditorNodePin pin)
{
	if (pin.HasConstantValue) return new ConstantValueNode<Float3>{ pin.ConstantValue.F3 };
	
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Float3);
	
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float3 pin input missing link!");
		return new ConstantValueNode<Float3>({});
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Float3: return EvaluateFloat3(static_cast<Float3EditorNode*>(node));
	case EditorNodeType::CreateFloat3: return EvaluateCreateFloat3(static_cast<CreateFloat3EditorNode*>(node));
	case EditorNodeType::Variable: return EvaluateVariable<Float3>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::Float3BinaryOperator: return EvaluateFloat3BinaryOperator(static_cast<Float3BinaryOperatorEditorNode*>(node));
	case EditorNodeType::NormalizeFloat3: return EvaluateNormalizeFloat3(static_cast<NormalizeFloat3EditorNode*>(node));
	case EditorNodeType::CrossProductOperation: return EvaluateCrossProductOperation(static_cast<CrossProductOperationEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<Float3ValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<Float3ValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Float3>({});
}

Float4ValueNode* PinEvaluator::EvaluateFloat4(EditorNodePin pin)
{
	if (pin.HasConstantValue) return new ConstantValueNode<Float4>{ pin.ConstantValue.F4 };
	
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Float4);
	
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float4 pin input missing link!");
		return new ConstantValueNode<Float4>({});
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Float4: return EvaluateFloat4(static_cast<Float4EditorNode*>(node));
	case EditorNodeType::CreateFloat4: return EvaluateCreateFloat4(static_cast<CreateFloat4EditorNode*>(node));
	case EditorNodeType::Variable: return EvaluateVariable<Float4>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::Float4BinaryOperator: return EvaluateFloat4BinaryOperator(static_cast<Float4BinaryOperatorEditorNode*>(node));
	case EditorNodeType::NormalizeFloat4: return EvaluateNormalizeFloat4(static_cast<NormalizeFloat4EditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<Float4ValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<Float4ValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Float4>({});
}

Float4x4ValueNode* PinEvaluator::EvaluateFloat4x4(EditorNodePin pin)
{
	// if (pin.HasConstantValue) return new ConstantValueNode<Float4x4>{ pin.ConstantValue.F4X4 };

	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Float4x4);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Float4x4 pin input missing link!");
		return new ConstantValueNode<Float4x4>(glm::identity<Float4x4>());
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Float4x4: return EvaluateFloat4x4(static_cast<Float4x4EditorNode*>(node));
	case EditorNodeType::Variable: return EvaluateVariable<Float4x4>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::Float4x4BinaryOperator: return EvaluateFloat4x4BinaryOperator(static_cast<Float4x4BinaryOperatorEditorNode*>(node));
	case EditorNodeType::Transform_Rotate_Float4x4: return EvaluateFloat4x4Rotate(static_cast<Float4x4RotationTransformEditorNode*>(node));
	case EditorNodeType::Transform_Translate_Float4x4: return EvaluateFloat4x4Translate(static_cast<Float4x4TranslationTransformEditorNode*>(node));
	case EditorNodeType::Transform_Scale_Float4x4: return EvaluateFloat4x4Scale(static_cast<Float4x4ScaleTransformEditorNode*>(node));
	case EditorNodeType::Transform_LookAt_Float4x4: return EvaluateFloat4x4LookAt(static_cast<Float4x4LookAtTransformEditorNode*>(node));
	case EditorNodeType::Transform_PerspectiveProjection_Float4x4: return EvaluateFloat4x4Perspective(static_cast<Float4x4PerspectiveTransformEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<Float4x4ValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<Float4x4ValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloat4x4Pin] internal error!");
	}
	return new ConstantValueNode<Float4x4>(glm::identity<Float4x4>());
}

TextureValueNode* PinEvaluator::EvaluateTexture(EditorNodePin pin)
{
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Texture);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Texture pin input not linked!");
		return new ConstantValueNode<Texture*>(nullptr);
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Variable: return EvaluateRenderResourceVariable<Texture*>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<TextureValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<TextureValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Texture*>(nullptr);
}

BufferValueNode* PinEvaluator::EvaluateBuffer(EditorNodePin pin)
{
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Buffer);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Buffer pin input not linked!");
		return new ConstantValueNode<Buffer*>(nullptr);
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Pin: return EvaluatePinNode<BufferValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<BufferValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Buffer*>(nullptr);
}

MeshValueNode* PinEvaluator::EvaluateMesh(EditorNodePin pin)
{
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Mesh);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Mesh pin input not linked!");
		return new ConstantValueNode<Mesh*>(nullptr);
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::GetMesh: return EvaluateGetMesh(static_cast<GetMeshEditorNode*>(node));
	case EditorNodeType::GetCubeMesh: return EvaluateGetCubeMesh(static_cast<GetCubeMeshEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<MeshValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<MeshValueNode>(static_cast<CustomEditorNode*>(node), pin);

	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Mesh*>(nullptr);
}

ShaderValueNode* PinEvaluator::EvaluateShader(EditorNodePin pin)
{
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Shader);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Shader pin input not linked!");
		return new ConstantValueNode<Shader*>(nullptr);
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Variable: return EvaluateRenderResourceVariable<Shader*>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<ShaderValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<ShaderValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateShader] internal error!");
	}
	return new ConstantValueNode<Shader*>(nullptr);
}

BindTableValueNode* PinEvaluator::EvaluateBindTable(EditorNodePin pin)
{
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::BindTable);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("BindTable pin input not linked!");
		return new ConstantValueNode<BindTable*>(nullptr);
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::BindTable: return EvaluateBindTable(static_cast<BindTableEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<BindTableValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<BindTableValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateBindTable] internal error!");
	}
	return new ConstantValueNode<BindTable*>(nullptr);
}

RenderStateValueNode* PinEvaluator::EvaluateRenderState(EditorNodePin pin)
{
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::RenderState);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("RenderState pin input not linked!");
		return new ConstantValueNode<RenderState>({});
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::RenderState: return EvaluateRenderState(static_cast<RenderStateEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<RenderStateValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<RenderStateValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateRenderState] internal error!");
	}
	return new ConstantValueNode<RenderState>({});
}

SceneObjectValueNode* PinEvaluator::EvaluateSceneObject(EditorNodePin pin)
{
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::SceneObject);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("SceneObject pin input not linked!");
		return new ConstantValueNode<SceneObject*>({});
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::ForEachSceneObject: return EvaluateForEachSceneObject(static_cast<ForEachSceneObjectEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<SceneObjectValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<SceneObjectValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateRenderState] internal error!");
	}
	return new ConstantValueNode<SceneObject*>({});
}

SceneValueNode* PinEvaluator::EvaluateScene(EditorNodePin pin)
{
	pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
	ASSERT(pin.Type == PinType::Scene);

	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Scene pin input not linked!");
		return new ConstantValueNode<Scene*>({});
	}

	EditorNode* node = GetNodeGraph()->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Variable: return EvaluateRenderResourceVariable<Scene*>(static_cast<VariableEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<SceneValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<SceneValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateRenderState] internal error!");
	}
	return new ConstantValueNode<Scene*>({});
}

IntValueNode* PinEvaluator::EvaluateInt(IntEditorNode* node)
{
	return new ConstantValueNode<int>(node->GetValue());
}

IntValueNode* PinEvaluator::EvaluateIntBinaryOperator(IntBinaryOperatorEditorNode* node)
{
	IntValueNode* a = EvaluateInt(node->GetAPin());
	IntValueNode* b = EvaluateInt(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<int>{ a, b, node->GetOp()[0] };
}

BoolValueNode* PinEvaluator::EvaluateIntComparisonOperator(IntComparisonOperatorEditorNode* node)
{
	IntValueNode* a = EvaluateInt(node->GetAPin());
	IntValueNode* b = EvaluateInt(node->GetBPin());
	return new ComparisonValueNode<int>{ a, b, node->GetOp() };
}

BoolValueNode* PinEvaluator::EvaluateBoolBinaryOperator(BoolBinaryOperatorEditorNode* node)
{
	BoolValueNode* a = EvaluateBool(node->GetAPin());
	BoolValueNode* b = EvaluateBool(node->GetBPin());
	return new BoolBinaryOperatorValueNode(a, b, node->GetOp());
}

BoolValueNode* PinEvaluator::EvaluateFloatComparisonOperator(FloatComparisonOperatorEditorNode* node)
{
	FloatValueNode* a = EvaluateFloat(node->GetAPin());
	FloatValueNode* b = EvaluateFloat(node->GetBPin());
	return new ComparisonValueNode<float>{ a, b, node->GetOp() };
}

StringValueNode* PinEvaluator::EvaluateString(StringEditorNode* node)
{
	return new ConstantValueNode<std::string>(node->GetValue());
}

BoolValueNode* PinEvaluator::EvaluateBool(BoolEditorNode* node)
{
	return new ConstantValueNode<bool>(node->GetValue());
}

FloatValueNode* PinEvaluator::EvaluateFloat(FloatEditorNode* node)
{
	return new ConstantValueNode<float>(node->GetFloat());
}

FloatValueNode* PinEvaluator::EvaluateSplitFloat2(SplitFloat2EditorNode* node, const EditorNodePin& pin)
{
	ASSERT(pin.Type == PinType::Float);
	ASSERT(GetNodeGraph()->GetPinOwner(pin.ID) == node);

	ValueNode<Float2>* vec = EvaluateFloat2(node->GetInputVectorPin());
	const unsigned vecIndex = node->GetOutputPinIndex(pin.ID);
	return new SplitVectorValueNode<Float2, float>(vec, vecIndex);
}

FloatValueNode* PinEvaluator::EvaluateSplitFloat3(SplitFloat3EditorNode* node, const EditorNodePin& pin)
{
	ASSERT(pin.Type == PinType::Float);
	ASSERT(GetNodeGraph()->GetPinOwner(pin.ID) == node);

	ValueNode<Float3>* vec = EvaluateFloat3(node->GetInputVectorPin());
	const unsigned vecIndex = node->GetOutputPinIndex(pin.ID);
	return new SplitVectorValueNode<Float3, float>(vec, vecIndex);
}

FloatValueNode* PinEvaluator::EvaluateSplitFloat4(SplitFloat4EditorNode* node, const EditorNodePin& pin)
{
	ASSERT(pin.Type == PinType::Float);
	ASSERT(GetNodeGraph()->GetPinOwner(pin.ID) == node);

	ValueNode<Float4>* vec = EvaluateFloat4(node->GetInputVectorPin());
	const unsigned vecIndex = node->GetOutputPinIndex(pin.ID);
	return new SplitVectorValueNode<Float4, float>(vec, vecIndex);
}

FloatValueNode* PinEvaluator::EvaluateFloatBinaryOperator(FloatBinaryOperatorEditorNode* node)
{
	FloatValueNode* a = EvaluateFloat(node->GetAPin());
	FloatValueNode* b = EvaluateFloat(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<float>{ a, b, node->GetOp()[0] };
}

FloatValueNode* PinEvaluator::EvaluateDeltaTime(OnUpdateEditorNode* node)
{
	return new VariableValueNode<float>(VariablePool::ID_DT);
}

Float2ValueNode* PinEvaluator::EvaluateFloat2(Float2EditorNode* node)
{
	return new ConstantValueNode<Float2>(node->GetFloat2());
}

Float2ValueNode* PinEvaluator::EvaluateCreateFloat2(CreateFloat2EditorNode* node)
{
	ValueNode<float>* x = EvaluateFloat(node->GetInputPin(0));
	ValueNode<float>* y = EvaluateFloat(node->GetInputPin(1));
	return new CreateVectorValueNode<Float2, float, 2>({ x,y });
}

Float2ValueNode* PinEvaluator::EvaluateFloat2BinaryOperator(Float2BinaryOperatorEditorNode* node)
{
	Float2ValueNode* a = EvaluateFloat2(node->GetAPin());
	Float2ValueNode* b = EvaluateFloat2(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<Float2>{ a, b, node->GetOp()[0]};
}

Float2ValueNode* PinEvaluator::EvaluateNormalizeFloat2(NormalizeFloat2EditorNode* node)
{
	Float2ValueNode* a = EvaluateFloat2(node->GetInputPin());
	return new NormalizeVectorValueNode<Float2>(a);
}

Float3ValueNode* PinEvaluator::EvaluateFloat3(Float3EditorNode* node)
{
	return new ConstantValueNode<Float3>(node->GetFloat3());
}

Float3ValueNode* PinEvaluator::EvaluateCreateFloat3(CreateFloat3EditorNode* node)
{
	ValueNode<float>* x = EvaluateFloat(node->GetInputPin(0));
	ValueNode<float>* y = EvaluateFloat(node->GetInputPin(1));
	ValueNode<float>* z = EvaluateFloat(node->GetInputPin(2));
	return new CreateVectorValueNode<Float3, float, 3>({ x,y, z });
}

Float3ValueNode* PinEvaluator::EvaluateFloat3BinaryOperator(Float3BinaryOperatorEditorNode* node)
{
	Float3ValueNode* a = EvaluateFloat3(node->GetAPin());
	Float3ValueNode* b = EvaluateFloat3(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<Float3>{ a, b, node->GetOp()[0]};
}

Float3ValueNode* PinEvaluator::EvaluateNormalizeFloat3(NormalizeFloat3EditorNode* node)
{
	Float3ValueNode* a = EvaluateFloat3(node->GetInputPin());
	return new NormalizeVectorValueNode<Float3>(a);
}

Float3ValueNode* PinEvaluator::EvaluateCrossProductOperation(CrossProductOperationEditorNode* node)
{
	Float3ValueNode* a = EvaluateFloat3(node->GetAPin());
	Float3ValueNode* b = EvaluateFloat3(node->GetBPin());
	return new CrossProductValueNode(a, b);
}

Float4ValueNode* PinEvaluator::EvaluateFloat4(Float4EditorNode* node)
{
	return new ConstantValueNode<Float4>(node->GetFloat4());
}

Float4ValueNode* PinEvaluator::EvaluateFloat4BinaryOperator(Float4BinaryOperatorEditorNode* node)
{
	Float4ValueNode* a = EvaluateFloat4(node->GetAPin());
	Float4ValueNode* b = EvaluateFloat4(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<Float4>{ a, b, node->GetOp()[0] };
}

Float4ValueNode* PinEvaluator::EvaluateNormalizeFloat4(NormalizeFloat4EditorNode* node)
{
	Float4ValueNode* a = EvaluateFloat4(node->GetInputPin());
	return new NormalizeVectorValueNode<Float4>(a);
}

Float4ValueNode* PinEvaluator::EvaluateCreateFloat4(CreateFloat4EditorNode* node)
{
	ValueNode<float>* x = EvaluateFloat(node->GetInputPin(0));
	ValueNode<float>* y = EvaluateFloat(node->GetInputPin(1));
	ValueNode<float>* z = EvaluateFloat(node->GetInputPin(2));
	ValueNode<float>* w = EvaluateFloat(node->GetInputPin(3));
	return new CreateVectorValueNode<Float4, float, 4>({ x, y, z, w });
}

Float4x4ValueNode* PinEvaluator::EvaluateFloat4x4(Float4x4EditorNode* node)
{
	return new ConstantValueNode<Float4x4>(node->GetFloat4x4());
}

Float4x4ValueNode* PinEvaluator::EvaluateFloat4x4BinaryOperator(Float4x4BinaryOperatorEditorNode* node)
{
	Float4x4ValueNode* a = EvaluateFloat4x4(node->GetAPin());
	Float4x4ValueNode* b = EvaluateFloat4x4(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<Float4x4>{ a, b, node->GetOp()[0] };
}

Float4x4ValueNode* PinEvaluator::EvaluateFloat4x4Rotate(Float4x4RotationTransformEditorNode* node)
{
	Float4x4ValueNode* lastTransform = EvaluatePinOptional<Float4x4ValueNode, &PinEvaluator::EvaluateFloat4x4>(node->GetLastTransformPin());
	FloatValueNode* angle = EvaluateFloat(node->GetAnglePin());
	Float3ValueNode* axis = EvaluateFloat3(node->GetAxisPin());
	return new Float4x4RotateValueNode{ lastTransform, angle, axis };
}

Float4x4ValueNode* PinEvaluator::EvaluateFloat4x4Translate(Float4x4TranslationTransformEditorNode* node)
{
	Float4x4ValueNode* lastTransform = EvaluatePinOptional<Float4x4ValueNode, &PinEvaluator::EvaluateFloat4x4>(node->GetLastTransformPin());
	Float3ValueNode* value = EvaluateFloat3(node->GetValuePin());
	return new Float4x4TranslateValueNode{ lastTransform, value };
}

Float4x4ValueNode* PinEvaluator::EvaluateFloat4x4Scale(Float4x4ScaleTransformEditorNode* node)
{
	Float4x4ValueNode* lastTransform = EvaluatePinOptional<Float4x4ValueNode, &PinEvaluator::EvaluateFloat4x4>(node->GetLastTransformPin());
	Float3ValueNode* value = EvaluateFloat3(node->GetValuePin());
	return new Float4x4ScaleValueNode{ lastTransform, value };

}

Float4x4ValueNode* PinEvaluator::EvaluateFloat4x4LookAt(Float4x4LookAtTransformEditorNode* node)
{
	Float3ValueNode* eye = EvaluateFloat3(node->GetEyePin());
	Float3ValueNode* center = EvaluateFloat3(node->GetCenterPin());
	Float3ValueNode* up = EvaluateFloat3(node->GetUpPin());
	return new Float4x4LookAtValueNode{ eye, center, up };
}

Float4x4ValueNode* PinEvaluator::EvaluateFloat4x4Perspective(Float4x4PerspectiveTransformEditorNode* node)
{
	FloatValueNode* fov = EvaluateFloat(node->GetFOVPin());
	FloatValueNode* aspect = EvaluateFloat(node->GetAspectRatioPin());
	FloatValueNode* znear = EvaluateFloat(node->GetZNearPin());
	FloatValueNode* zfar = EvaluateFloat(node->GetZFarPin());
	return new Float4x4PerspectiveValueNode{ fov, aspect, znear, zfar };
}

MeshValueNode* PinEvaluator::EvaluateGetCubeMesh(GetCubeMeshEditorNode* node)
{
	ValueNodeExtraInfo extraInfo;
	extraInfo.MeshVertexBits.Position = true;
	extraInfo.MeshVertexBits.Texcoord = false;
	extraInfo.MeshVertexBits.Normal = false;
	extraInfo.MeshVertexBits.Tangent = false;
	return new StaticResourceNode<Mesh*, ExecutorStaticResource::CubeMesh>(extraInfo);
}

MeshValueNode* PinEvaluator::EvaluateGetMesh(GetMeshEditorNode* node)
{
	SceneObjectValueNode* sceneObjectNode = EvaluateSceneObject(node->GetSceneObjectPin());
	
	ValueNodeExtraInfo extraInfo;
	extraInfo.MeshVertexBits.Position = node->GetPositionBit();
	extraInfo.MeshVertexBits.Texcoord = node->GetTexcoordBit();
	extraInfo.MeshVertexBits.Normal = node->GetNormalBit();
	extraInfo.MeshVertexBits.Tangent = node->GetTangentBit();
	return new GetMeshValueNode(sceneObjectNode, extraInfo);
}

BindTableValueNode* PinEvaluator::EvaluateBindTable(BindTableEditorNode* node)
{
	BindTable* bindTable = new BindTable{};
	for (const auto& pin : node->GetCustomPins())
	{
		ASSERT(pin.IsInput);
		switch (pin.Type)
		{
		case PinType::Texture:
			bindTable->Textures.push_back(BindTable::Binding<Texture*>{ pin.Label, Ptr<TextureValueNode>(EvaluateTexture(pin)) });
			break;
		case PinType::Float:
			bindTable->Floats.push_back(BindTable::Binding<float>{pin.Label, Ptr<FloatValueNode>(EvaluateFloat(pin))});
			break;
		case PinType::Float2:
			bindTable->Float2s.push_back(BindTable::Binding<Float2>{pin.Label, Ptr<Float2ValueNode>(EvaluateFloat2(pin))});
			break;
		case PinType::Float3:
			bindTable->Float3s.push_back(BindTable::Binding<Float3>{pin.Label, Ptr<Float3ValueNode>(EvaluateFloat3(pin))});
			break;
		case PinType::Float4:
			bindTable->Float4s.push_back(BindTable::Binding<Float4>{pin.Label, Ptr<Float4ValueNode>(EvaluateFloat4(pin))});
			break;
		case PinType::Float4x4:
			bindTable->Float4x4s.push_back(BindTable::Binding<Float4x4>{pin.Label, Ptr<Float4x4ValueNode>(EvaluateFloat4x4(pin))});
			break;
		default:
			NOT_IMPLEMENTED;
			m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateBindTable] internal error!");
		}
	}
	return new ConstantPtrValueNode<BindTable>{ bindTable };
}

RenderStateValueNode* PinEvaluator::EvaluateRenderState(RenderStateEditorNode* node)
{
	RenderState state;
	state.DepthWrite = node->IsDepthWrite();

	const std::string& dt = node->GetDepthTestMode();
	if (dt == "Always") state.DepthTest = GL_ALWAYS;
	else if (dt == "Never") state.DepthTest = GL_NEVER;
	else if (dt == "Less") state.DepthTest = GL_LESS;
	else if (dt == "Equal") state.DepthTest = GL_EQUAL;
	else if (dt == "LessEqual") state.DepthTest = GL_LEQUAL;
	else if (dt == "Greater") state.DepthTest = GL_GREATER;
	else if (dt == "GreaterEqual") state.DepthTest = GL_GEQUAL;
	else if (dt == "NotEqual") state.DepthTest = GL_NOTEQUAL;
	else NOT_IMPLEMENTED;

	return new ConstantValueNode<RenderState>{ state };
}

SceneObjectValueNode* PinEvaluator::EvaluateForEachSceneObject(ForEachSceneObjectEditorNode* node)
{
	const PinID sceneObjectiteratorPin = node->GetSceneObjectPin().ID;
	return new IteratorValueNode<SceneObject*>{ sceneObjectiteratorPin };
}