#include "NodeGraphCompiler.h"

#include "../Execution/ExecutorNode.h"
#include "../Editor/EditorNode.h"
#include "../Editor/RenderPipelineEditor.h"

static EditorNodePin GetOutputPinIfInput(const NodeGraph* nodeGraph, EditorNodePin pin)
{
	if (pin.IsInput)
	{
		const PinID outPinID = nodeGraph->GetOutputPinForInput(pin.ID);
		if (!outPinID) return EditorNodePin{};
		pin = nodeGraph->GetPinByID(outPinID);
	}
	return pin;
}

ValueNode<bool>* NodeGraphCompiler::EvaluateBoolPin(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Bool);
	
	pin = GetOutputPinIfInput(m_CurrentGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Add node missing outputs!");
		return new ConstantValueNode<bool>(false);
	}

	EditorNode* node = m_CurrentGraph->GetPinOwner(pin.ID);
	switch (node->GetType())
	{
	case EditorNodeType::Bool: return EvaluateBool(static_cast<BoolEditorNode*>(node));
	default: 
		NOT_IMPLEMENTED;
		m_ErrorMessages.push_back("[NodeGraphCompiler::EvaluateBoolPin] internal error!");
	}
	return new ConstantValueNode<bool>(false);
}

ValueNode<bool>* NodeGraphCompiler::EvaluateBool(BoolEditorNode* boolNode)
{
	return new ConstantValueNode<bool>(boolNode->GetValue());
}

ValueNode<float>* NodeGraphCompiler::EvaluateFloatPin(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Float);
	
	pin = GetOutputPinIfInput(m_CurrentGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Add node missing outputs!");
		return new ConstantValueNode<float>(0.0f);
	}

	EditorNode* node = m_CurrentGraph->GetPinOwner(pin.ID);
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

ValueNode<float>* NodeGraphCompiler::EvaluateFloat(FloatEditorNode* floatNode)
{
	return new ConstantValueNode<float>(floatNode->GetFloat());
}

ValueNode<float>* NodeGraphCompiler::EvaluateVarFloat(VarFloatEditorNode* floatNode)
{
	// TODO: Validate if the variable is actually initialized at this point
	return new VariableValueNode<float>(floatNode->GetVaraibleName());
}

ValueNode<float>* NodeGraphCompiler::EvaluateFloatBinaryOperator(FloatBinaryOperatorEditorNode* floatOpNode)
{
	const auto& aPin = m_CurrentGraph->GetPinByID(floatOpNode->GetAPin());
	const auto& bPin = m_CurrentGraph->GetPinByID(floatOpNode->GetBPin());

	ValueNode<float>* a = EvaluateFloatPin(aPin);
	ValueNode<float>* b = EvaluateFloatPin(bPin);

	return new BinaryOperatorValueNode<float>{ a, b, floatOpNode->GetOp()};
}

ValueNode<glm::vec2>* NodeGraphCompiler::EvaluateFloat2Pin(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Float2);

	pin = GetOutputPinIfInput(m_CurrentGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Add node missing outputs!");
		return new ConstantValueNode<glm::vec2>({});
	}

	EditorNode* node = m_CurrentGraph->GetPinOwner(pin.ID);
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

ValueNode<glm::vec2>* NodeGraphCompiler::EvaluateFloat2(Float2EditorNode* floatNode)
{
	return new ConstantValueNode<glm::vec2>(floatNode->GetFloat2());
}

ValueNode<glm::vec2>* NodeGraphCompiler::EvaluateVarFloat2(VarFloat2EditorNode* floatNode)
{
	// TODO: Validate if the variable is actually initialized at this point
	return new VariableValueNode<glm::vec2>(floatNode->GetVaraibleName());
}

ValueNode<glm::vec2>* NodeGraphCompiler::EvaluateFloat2BinaryOperator(Float2BinaryOperatorEditorNode* floatOpNode)
{
	const auto& aPin = m_CurrentGraph->GetPinByID(floatOpNode->GetAPin());
	const auto& bPin = m_CurrentGraph->GetPinByID(floatOpNode->GetBPin());

	ValueNode<glm::vec2>* a = EvaluateFloat2Pin(aPin);
	ValueNode<glm::vec2>* b = EvaluateFloat2Pin(bPin);

	return new BinaryOperatorValueNode<glm::vec2>{ a, b, floatOpNode->GetOp() };
}

ValueNode<glm::vec3>* NodeGraphCompiler::EvaluateFloat3Pin(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Float3);

	pin = GetOutputPinIfInput(m_CurrentGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Add node missing outputs!");
		return new ConstantValueNode<glm::vec3>({});
	}

	EditorNode* node = m_CurrentGraph->GetPinOwner(pin.ID);
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

ValueNode<glm::vec3>* NodeGraphCompiler::EvaluateFloat3(Float3EditorNode* floatNode)
{
	return new ConstantValueNode<glm::vec3>(floatNode->GetFloat3());
}

ValueNode<glm::vec3>* NodeGraphCompiler::EvaluateVarFloat3(VarFloat3EditorNode* floatNode)
{
	// TODO: Validate if the variable is actually initialized at this point
	return new VariableValueNode<glm::vec3>(floatNode->GetVaraibleName());
}

ValueNode<glm::vec3>* NodeGraphCompiler::EvaluateFloat3BinaryOperator(Float3BinaryOperatorEditorNode* floatOpNode)
{
	const auto& aPin = m_CurrentGraph->GetPinByID(floatOpNode->GetAPin());
	const auto& bPin = m_CurrentGraph->GetPinByID(floatOpNode->GetBPin());

	ValueNode<glm::vec3>* a = EvaluateFloat3Pin(aPin);
	ValueNode<glm::vec3>* b = EvaluateFloat3Pin(bPin);

	return new BinaryOperatorValueNode<glm::vec3>{ a, b, floatOpNode->GetOp() };
}

ValueNode<glm::vec4>* NodeGraphCompiler::EvaluateFloat4Pin(EditorNodePin pin)
{
	ASSERT(pin.Type == PinType::Float4);

	pin = GetOutputPinIfInput(m_CurrentGraph, pin);
	if (pin.Type == PinType::Invalid)
	{
		m_ErrorMessages.push_back("Add node missing outputs!");
		return new ConstantValueNode<glm::vec4>({});
	}

	EditorNode* node = m_CurrentGraph->GetPinOwner(pin.ID);
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

ValueNode<glm::vec4>* NodeGraphCompiler::EvaluateFloat4(Float4EditorNode* floatNode)
{
	return new ConstantValueNode<glm::vec4>(floatNode->GetFloat4());
}

ValueNode<glm::vec4>* NodeGraphCompiler::EvaluateVarFloat4(VarFloat4EditorNode* floatNode)
{
	// TODO: Validate if the variable is actually initialized at this point
	return new VariableValueNode<glm::vec4>(floatNode->GetVaraibleName());
}

ValueNode<glm::vec4>* NodeGraphCompiler::EvaluateFloat4BinaryOperator(Float4BinaryOperatorEditorNode* floatOpNode)
{
	const auto& aPin = m_CurrentGraph->GetPinByID(floatOpNode->GetAPin());
	const auto& bPin = m_CurrentGraph->GetPinByID(floatOpNode->GetBPin());

	ValueNode<glm::vec4>* a = EvaluateFloat4Pin(aPin);
	ValueNode<glm::vec4>* b = EvaluateFloat4Pin(bPin);

	return new BinaryOperatorValueNode<glm::vec4>{ a, b, floatOpNode->GetOp() };
}

ExecutionEditorNode* NodeGraphCompiler::GetNextExecutorNode(ExecutionEditorNode* executorNode)
{
	// TODO: Do not use dynamic cast, but check node by type if it is execution

	const auto nextNodePin = m_CurrentGraph->GetInputPinFromOutput(executorNode->GetExecutionOutput());
	if (!nextNodePin) return (ExecutionEditorNode*) nullptr;
	EditorNode* node = m_CurrentGraph->GetPinOwner(nextNodePin);
	ExecutionEditorNode* exNode = dynamic_cast<ExecutionEditorNode*>(node);
	ASSERT(exNode);
	return exNode;
}

ExecutorNode* NodeGraphCompiler::CompileIfNode(IfEditorNode* ifNode)
{
	const auto& conditionPin = m_CurrentGraph->GetPinByID(ifNode->GetConditionPin());
	ValueNode<bool>* conditionNode = EvaluateBoolPin(conditionPin);
	
	ExecutorNode* elseBranch = nullptr;
	if (const NodeID elsePinInput = m_CurrentGraph->GetInputPinFromOutput(ifNode->GetExecutionElse()))
	{
		// TODO: Do not use dynamic cast, but check node by type if it is execution
		EditorNode* elseEditorNode = m_CurrentGraph->GetPinOwner(elsePinInput);
		ExecutionEditorNode* exElseEditorNode = dynamic_cast<ExecutionEditorNode*>(elseEditorNode);
		ASSERT(exElseEditorNode);
		
		if (exElseEditorNode) elseBranch = Compile(exElseEditorNode);
	}
	return new IfExecutorNode{ conditionNode, elseBranch };
}

ExecutorNode* NodeGraphCompiler::CompilePrintNode(PrintEditorNode* printNode)
{
	PinID inputPinID = m_CurrentGraph->GetOutputPinForInput(printNode->GetFloatInputPin());
	if(!inputPinID) inputPinID = m_CurrentGraph->GetOutputPinForInput(printNode->GetFloat2InputPin());
	if(!inputPinID) inputPinID = m_CurrentGraph->GetOutputPinForInput(printNode->GetFloat3InputPin());
	if(!inputPinID) inputPinID = m_CurrentGraph->GetOutputPinForInput(printNode->GetFloat4InputPin());

	EditorNodePin floatInputPin = m_CurrentGraph->GetPinByID(inputPinID);
	switch (floatInputPin.Type)
	{
	case PinType::Float: return new PrintExecutorNode{ EvaluateFloatPin(floatInputPin) };
	case PinType::Float2: return new PrintExecutorNode{ EvaluateFloat2Pin(floatInputPin) };
	case PinType::Float3: return new PrintExecutorNode{ EvaluateFloat3Pin(floatInputPin) };
	case PinType::Float4: return new PrintExecutorNode{ EvaluateFloat4Pin(floatInputPin) };
	default:
		NOT_IMPLEMENTED;
	}
	return nullptr;
}

ExecutorNode* NodeGraphCompiler::CompileAsignVariableNode(AsignVariableEditorNode* asignFloatNode)
{
	const EditorNodePin asignValuePin = m_CurrentGraph->GetPinByID(asignFloatNode->GetValuePin());
	switch (asignValuePin.Type)
	{
	case PinType::Float: return new AsignVariableExecutorNode<float>(asignFloatNode->GetName(), EvaluateFloatPin(asignValuePin));
	case PinType::Float2: return new AsignVariableExecutorNode<glm::vec2>(asignFloatNode->GetName(), EvaluateFloat2Pin(asignValuePin));
	case PinType::Float3: return new AsignVariableExecutorNode<glm::vec3>(asignFloatNode->GetName(), EvaluateFloat3Pin(asignValuePin));
	case PinType::Float4: return new AsignVariableExecutorNode<glm::vec4>(asignFloatNode->GetName(), EvaluateFloat4Pin(asignValuePin));
	default:
		NOT_IMPLEMENTED;
	}
	return new EmptyExecutorNode{};
}

ExecutorNode* NodeGraphCompiler::CompileClearRenderTargetNode(ClearRenderTargetEditorNode* clearRtNode)
{
	const PinID clearColorOutPin = m_CurrentGraph->GetOutputPinForInput(clearRtNode->GetClearColorPin());
	ValueNode<glm::vec4>* clearColorNode = clearColorOutPin ? 
		EvaluateFloat4Pin(m_CurrentGraph->GetPinByID(clearColorOutPin)) :
		new ConstantValueNode<glm::vec4>(glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f });
	return new ClearRenderTargetExecutorNode{ clearColorNode };
}

ExecutorNode* NodeGraphCompiler::CompileExecutorNode(ExecutionEditorNode* executorNode)
{
	switch (executorNode->GetType())
	{
	case EditorNodeType::Print:			return CompilePrintNode(static_cast<PrintEditorNode*>(executorNode));
	case EditorNodeType::If:			return CompileIfNode(static_cast<IfEditorNode*>(executorNode));
	case EditorNodeType::AsignFloat:	
	case EditorNodeType::AsignFloat2:	
	case EditorNodeType::AsignFloat3:	
	case EditorNodeType::AsignFloat4:	
		return CompileAsignVariableNode(static_cast<AsignFloatEditorNode*>(executorNode));
	case EditorNodeType::ClearRenderTarget:
		return CompileClearRenderTargetNode(static_cast<ClearRenderTargetEditorNode*>(executorNode));
	case EditorNodeType::OnStart:
	case EditorNodeType::OnUpdate:
		return new EmptyExecutorNode();
	default:
		m_ErrorMessages.push_back("[NodeGraphCompiler::CompileExecutorNode] internal error!");
		NOT_IMPLEMENTED;
	}
	return new EmptyExecutorNode();
}

ExecutorNode* NodeGraphCompiler::Compile(ExecutionEditorNode* executorNode)
{
	ExecutorNode* firstNode = new EmptyExecutorNode();
	ExecutorNode* currentNode = firstNode;
	ExecutionEditorNode* currentEditorNode = executorNode;

	while (currentEditorNode)
	{
		ExecutorNode* nextNode = CompileExecutorNode(currentEditorNode);
		currentNode->SetNextNode(nextNode);
		currentNode = nextNode;
		currentEditorNode = GetNextExecutorNode(currentEditorNode);
	}
	return firstNode;
}

CompiledPipeline NodeGraphCompiler::Compile(const NodeGraph& graph)
{
	m_CurrentGraph = &graph;
	
	CompiledPipeline pipeline{};
	pipeline.OnStartNode = Compile(m_CurrentGraph->GetOnStartNode());
	pipeline.OnUpdateNode = Compile(m_CurrentGraph->GetOnUpdateNode());
	return pipeline;
}
