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
	return new ConstantValueNode<float>(floatNode->GetValue());
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
	EditorNodePin floatInputPin = m_CurrentGraph->GetPinByID(printNode->GetFloatInputPin());
	ASSERT(floatInputPin.Type == PinType::Float);
	return new PrintExecutorNode{ EvaluateFloatPin(floatInputPin) };
}

ExecutorNode* NodeGraphCompiler::CompileAsignFloatNode(AsignFloatEditorNode* asignFloatNode)
{
	const EditorNodePin asignValuePin = m_CurrentGraph->GetPinByID(asignFloatNode->GetValuePin());
	ASSERT(asignValuePin.Type == PinType::Float);
	return new AsignVariableExecutorNode<float>(asignFloatNode->GetName(), EvaluateFloatPin(asignValuePin));
}

ExecutorNode* NodeGraphCompiler::CompileExecutorNode(ExecutionEditorNode* executorNode)
{
	switch (executorNode->GetType())
	{
	case EditorNodeType::Print:			return CompilePrintNode(static_cast<PrintEditorNode*>(executorNode));
	case EditorNodeType::If:			return CompileIfNode(static_cast<IfEditorNode*>(executorNode));
	case EditorNodeType::AsignFloat:	return CompileAsignFloatNode(static_cast<AsignFloatEditorNode*>(executorNode));
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