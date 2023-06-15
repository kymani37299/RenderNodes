#include "NodeGraphCompiler.h"

#include "../Execution/ExecutorNode.h"
#include "../Editor/EditorNode.h"
#include "../Editor/RenderPipelineEditor.h"

ExecutionEditorNode* NodeGraphCompiler::GetNextExecutorNode(ExecutionEditorNode* executorNode)
{
	// TODO: Do not use dynamic cast, but check node by type if it is execution

	const auto nextNodePin = m_CurrentGraph->GetInputPinFromOutput(executorNode->GetExecutionOutput().ID);
	if (!nextNodePin) return (ExecutionEditorNode*) nullptr;
	EditorNode* node = m_CurrentGraph->GetPinOwner(nextNodePin);
	ExecutionEditorNode* exNode = dynamic_cast<ExecutionEditorNode*>(node);
	ASSERT(exNode);
	return exNode;
}

CompiledPipeline NodeGraphCompiler::Compile(const NodeGraph& graph)
{
	m_ErrorMessages.clear();
	m_CurrentGraph = &graph;
	m_PinEvaluator = Ptr<PinEvaluator>(new PinEvaluator{ graph, m_ErrorMessages });

	CompiledPipeline pipeline{};
	pipeline.OnStartNode = Compile(m_CurrentGraph->GetOnStartNode());
	pipeline.OnUpdateNode = Compile(m_CurrentGraph->GetOnUpdateNode());
	return pipeline;
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

#define COMPILE_NODE(NodeType, Compiler, SpecificType) case EditorNodeType::NodeType: return Compiler(static_cast<SpecificType*>(executorNode))

ExecutorNode* NodeGraphCompiler::CompileExecutorNode(ExecutionEditorNode* executorNode)
{
	switch (executorNode->GetType())
	{
		COMPILE_NODE(Print, CompilePrintNode, PrintEditorNode);
		COMPILE_NODE(If, CompileIfNode, IfEditorNode);
		COMPILE_NODE(AsignFloat, CompileAsignVariableNode, AsignVariableEditorNode);
		COMPILE_NODE(AsignFloat2, CompileAsignVariableNode, AsignVariableEditorNode);
		COMPILE_NODE(AsignFloat3, CompileAsignVariableNode, AsignVariableEditorNode);
		COMPILE_NODE(AsignFloat4, CompileAsignVariableNode, AsignVariableEditorNode);
		COMPILE_NODE(AsignBool, CompileAsignVariableNode, AsignVariableEditorNode);
		COMPILE_NODE(ClearRenderTarget, CompileClearRenderTargetNode, ClearRenderTargetEditorNode);
		COMPILE_NODE(CreateTexture, CompileCreateTextureNode, CreateTextureEditorNode);
		COMPILE_NODE(LoadTexture, CompileLoadTextureNode, LoadTextureEditorNode);
		COMPILE_NODE(LoadShader, CompileLoadShaderNode, LoadShaderEditorNode);
		COMPILE_NODE(PresentTexture, CompilePresentTextureTargetNode, PresentTextureEditorNode);
		COMPILE_NODE(DrawMesh, CompileDrawMeshNode, DrawMeshEditorNode);
		COMPILE_NODE(LoadMesh, CompileLoadMeshNode, LoadMeshEditorNode);
		COMPILE_NODE(OnStart, CompileEmptyNode, ExecutionEditorNode);
		COMPILE_NODE(OnUpdate, CompileEmptyNode, ExecutionEditorNode);
	default:
		m_ErrorMessages.push_back("[NodeGraphCompiler::CompileExecutorNode] internal error!");
		NOT_IMPLEMENTED;
	}
	return new EmptyExecutorNode();
}



ExecutorNode* NodeGraphCompiler::CompileIfNode(IfEditorNode* ifNode)
{
	BoolValueNode* conditionNode = m_PinEvaluator->EvaluateBool(ifNode->GetConditionPin());
	
	ExecutorNode* elseBranch = nullptr;
	if (const NodeID elsePinInput = m_CurrentGraph->GetInputPinFromOutput(ifNode->GetExecutionElse().ID))
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
	PinID inputPinID = m_CurrentGraph->GetOutputPinForInput(printNode->GetFloatInputPin().ID);
	if (!inputPinID) inputPinID = m_CurrentGraph->GetOutputPinForInput(printNode->GetFloat2InputPin().ID);
	if (!inputPinID) inputPinID = m_CurrentGraph->GetOutputPinForInput(printNode->GetFloat3InputPin().ID);
	if (!inputPinID) inputPinID = m_CurrentGraph->GetOutputPinForInput(printNode->GetFloat4InputPin().ID);

	if (!inputPinID)
	{
		m_ErrorMessages.push_back("Print node missing outputs!");
		return new EmptyExecutorNode{};
	}

	EditorNodePin inputPin = m_CurrentGraph->GetPinByID(inputPinID);
	switch (inputPin.Type)
	{
	case PinType::Float: return new PrintExecutorNode{ m_PinEvaluator->EvaluateFloat(inputPin) };
	case PinType::Float2: return new PrintExecutorNode{ m_PinEvaluator->EvaluateFloat2(inputPin) };
	case PinType::Float3: return new PrintExecutorNode{ m_PinEvaluator->EvaluateFloat3(inputPin) };
	case PinType::Float4: return new PrintExecutorNode{ m_PinEvaluator->EvaluateFloat4(inputPin) };
	default:
		NOT_IMPLEMENTED;
	}
	return new EmptyExecutorNode{};
}

ExecutorNode* NodeGraphCompiler::CompileAsignVariableNode(AsignVariableEditorNode* node)
{
	const auto& asignValuePin = node->GetValuePin();
	switch (asignValuePin.Type)
	{
	case PinType::Float: return new AsignVariableExecutorNode<float>(node->GetName(), m_PinEvaluator->EvaluateFloat(asignValuePin));
	case PinType::Float2: return new AsignVariableExecutorNode<Float2>(node->GetName(), m_PinEvaluator->EvaluateFloat2(asignValuePin));
	case PinType::Float3: return new AsignVariableExecutorNode<Float3>(node->GetName(), m_PinEvaluator->EvaluateFloat3(asignValuePin));
	case PinType::Float4: return new AsignVariableExecutorNode<Float4>(node->GetName(), m_PinEvaluator->EvaluateFloat4(asignValuePin));
	case PinType::Bool:	return new AsignVariableExecutorNode<bool>(node->GetName(), m_PinEvaluator->EvaluateBool(asignValuePin));
	default:
		NOT_IMPLEMENTED;
	}
	return new EmptyExecutorNode{};
}

ExecutorNode* NodeGraphCompiler::CompileCreateTextureNode(CreateTextureEditorNode* createTextureNode)
{
	return new CreateTextureExecutorNode{ createTextureNode->GetName(), createTextureNode->GetWidth(), createTextureNode->GetHeight(), createTextureNode->IsFramebuffer() };
}

ExecutorNode* NodeGraphCompiler::CompileClearRenderTargetNode(ClearRenderTargetEditorNode* clearRtNode)
{
	TextureValueNode* textureNode = m_PinEvaluator->EvaluateTexture(clearRtNode->GetTargeteTexturePin());
	Float4ValueNode* clearColorNode = m_PinEvaluator->EvaluateFloat4(clearRtNode->GetClearColorPin());
	return new ClearRenderTargetExecutorNode{ textureNode, clearColorNode };
}

ExecutorNode* NodeGraphCompiler::CompilePresentTextureTargetNode(PresentTextureEditorNode* presentTextureNode)
{
	TextureValueNode* textureNode = m_PinEvaluator->EvaluateTexture(presentTextureNode->GetTexturePin());
	return new PresentTextureExecutorNode{ textureNode };
}

ExecutorNode* NodeGraphCompiler::CompileLoadTextureNode(LoadTextureEditorNode* loadTextureNode)
{
	return new LoadTextureExecutorNode{ loadTextureNode->GetName(), loadTextureNode->GetPath() };
}

ExecutorNode* NodeGraphCompiler::CompileLoadShaderNode(LoadShaderEditorNode* loadShaderNode)
{
	return new LoadShaderExecutorNode{ loadShaderNode->GetName(), loadShaderNode->GetPath() };
}

ExecutorNode* NodeGraphCompiler::CompileDrawMeshNode(DrawMeshEditorNode* drawMeshNode)
{
	TextureValueNode* framebufferNode = m_PinEvaluator->EvaluateTexture(drawMeshNode->GetFrameBufferPin());
	ShaderValueNode* shaderNode = m_PinEvaluator->EvaluateShader(drawMeshNode->GetShaderPin());
	MeshValueNode* meshNode = m_PinEvaluator->EvaluateMesh(drawMeshNode->GetMeshPin());

	PinID bindTableOutputID = m_CurrentGraph->GetOutputPinForInput(drawMeshNode->GetBindTablePin().ID);
	BindTableValueNode* bindTableNode = nullptr;
	if (bindTableOutputID) bindTableNode = m_PinEvaluator->EvaluateBindTable(m_CurrentGraph->GetPinByID(bindTableOutputID));
	return new DrawMeshExecutorNode{ framebufferNode, shaderNode, meshNode, bindTableNode };
}

ExecutorNode* NodeGraphCompiler::CompileLoadMeshNode(LoadMeshEditorNode* loadMeshNode)
{
	return new LoadMeshExecutorNode{ loadMeshNode->GetName(), loadMeshNode->GetPath() };
}

ExecutorNode* NodeGraphCompiler::CompileEmptyNode(ExecutionEditorNode* node)
{
	return new EmptyExecutorNode{};
}
