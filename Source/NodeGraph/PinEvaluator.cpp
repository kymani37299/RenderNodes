#include "PinEvaluator.h"

BoolValueNode* PinEvaluator::EvaluateBool(EditorNodePin pin)
{
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
	case EditorNodeType::VarBool: return EvaluateVarBool(static_cast<VarBoolEditorNode*>(node));
	case EditorNodeType::BoolBinaryOperator: return EvaluateBoolBinaryOperator(static_cast<BoolBinaryOperatorEditorNode*>(node));
	case EditorNodeType::FloatComparisonOperator: return EvaluateFloatComparisonOperator(static_cast<FloatComparisonOperatorEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<BoolValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<BoolValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateBoolPin] internal error!");
	}
	return new ConstantValueNode<bool>(false);
}

StringValueNode* PinEvaluator::EvaluateString(EditorNodePin pin)
{
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
	case EditorNodeType::VarFloat: return EvaluateVarFloat(static_cast<VarFloatEditorNode*>(node));
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
	case EditorNodeType::VarFloat2: return EvaluateVarFloat2(static_cast<VarFloat2EditorNode*>(node));
	case EditorNodeType::Float2BinaryOperator: return EvaluateFloat2BinaryOperator(static_cast<Float2BinaryOperatorEditorNode*>(node));
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
	case EditorNodeType::VarFloat3: return EvaluateVarFloat3(static_cast<VarFloat3EditorNode*>(node));
	case EditorNodeType::Float3BinaryOperator: return EvaluateFloat3BinaryOperator(static_cast<Float3BinaryOperatorEditorNode*>(node));
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
	case EditorNodeType::VarFloat4: return EvaluateVarFloat4(static_cast<VarFloat4EditorNode*>(node));
	case EditorNodeType::Float4BinaryOperator: return EvaluateFloat4BinaryOperator(static_cast<Float4BinaryOperatorEditorNode*>(node));
	case EditorNodeType::Pin: return EvaluatePinNode<Float4ValueNode>(static_cast<PinEditorNode*>(node));
	case EditorNodeType::Custom: return EvaluateCustomNode<Float4ValueNode>(static_cast<CustomEditorNode*>(node), pin);
	default:
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateFloatPin] internal error!");
	}
	return new ConstantValueNode<Float4>({});
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
	case EditorNodeType::GetTexture: return EvaluateGetTexture(static_cast<GetTextureEditorNode*>(node));
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
	case EditorNodeType::GetShader: return EvaluateGetShader(static_cast<GetShaderEditorNode*>(node));
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

BoolValueNode* PinEvaluator::EvaluateVarBool(VarBoolEditorNode* node)
{
	StringValueNode* nameNode = EvaluateString(node->GetNamePin());
	return new VariableValueNode<bool>(nameNode);
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

FloatValueNode* PinEvaluator::EvaluateVarFloat(VarFloatEditorNode* node)
{
	StringValueNode* nameNode = EvaluateString(node->GetNamePin());
	return new VariableValueNode<float>(nameNode);
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
	return new VariableValueNode<float>(new ConstantValueNode<std::string>("DT"));
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

Float2ValueNode* PinEvaluator::EvaluateVarFloat2(VarFloat2EditorNode* node)
{
	StringValueNode* nameNode = EvaluateString(node->GetNamePin());
	return new VariableValueNode<Float2>(nameNode);
}

Float2ValueNode* PinEvaluator::EvaluateFloat2BinaryOperator(Float2BinaryOperatorEditorNode* node)
{
	Float2ValueNode* a = EvaluateFloat2(node->GetAPin());
	Float2ValueNode* b = EvaluateFloat2(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<Float2>{ a, b, node->GetOp()[0]};
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

Float3ValueNode* PinEvaluator::EvaluateVarFloat3(VarFloat3EditorNode* node)
{
	StringValueNode* nameNode = EvaluateString(node->GetNamePin());
	return new VariableValueNode<Float3>(nameNode);
}

Float3ValueNode* PinEvaluator::EvaluateFloat3BinaryOperator(Float3BinaryOperatorEditorNode* node)
{
	Float3ValueNode* a = EvaluateFloat3(node->GetAPin());
	Float3ValueNode* b = EvaluateFloat3(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<Float3>{ a, b, node->GetOp()[0]};
}

Float4ValueNode* PinEvaluator::EvaluateFloat4(Float4EditorNode* node)
{
	return new ConstantValueNode<Float4>(node->GetFloat4());
}

Float4ValueNode* PinEvaluator::EvaluateVarFloat4(VarFloat4EditorNode* node)
{
	StringValueNode* nameNode = EvaluateString(node->GetNamePin());
	return new VariableValueNode<Float4>(nameNode);
}

Float4ValueNode* PinEvaluator::EvaluateFloat4BinaryOperator(Float4BinaryOperatorEditorNode* node)
{
	Float4ValueNode* a = EvaluateFloat4(node->GetAPin());
	Float4ValueNode* b = EvaluateFloat4(node->GetBPin());
	return new BinaryArithmeticOperatorValueNode<Float4>{ a, b, node->GetOp()[0] };
}

Float4ValueNode* PinEvaluator::EvaluateCreateFloat4(CreateFloat4EditorNode* node)
{
	ValueNode<float>* x = EvaluateFloat(node->GetInputPin(0));
	ValueNode<float>* y = EvaluateFloat(node->GetInputPin(1));
	ValueNode<float>* z = EvaluateFloat(node->GetInputPin(2));
	ValueNode<float>* w = EvaluateFloat(node->GetInputPin(3));
	return new CreateVectorValueNode<Float4, float, 4>({ x, y, z, w });
}

TextureValueNode* PinEvaluator::EvaluateGetTexture(GetTextureEditorNode* node)
{
	StringValueNode* nameNode = EvaluateString(node->GetNamePin());
	return new VariableValueNode<Texture*>(nameNode);
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
	StringValueNode* nameNode = EvaluateString(node->GetNamePin());

	ValueNodeExtraInfo extraInfo;
	extraInfo.MeshVertexBits.Position = node->GetPositionBit();
	extraInfo.MeshVertexBits.Texcoord = node->GetTexcoordBit();
	extraInfo.MeshVertexBits.Normal = node->GetNormalBit();
	extraInfo.MeshVertexBits.Tangent = node->GetTangentBit();
	return new VariableValueNode<Mesh*>(nameNode, extraInfo);
}

ShaderValueNode* PinEvaluator::EvaluateGetShader(GetShaderEditorNode* node)
{
	StringValueNode* nameNode = EvaluateString(node->GetNamePin());
	return new VariableValueNode<Shader*>(nameNode);
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