#include "NodeGraphCompiler.h"

#include "../Execution/ExecutorNode.h"
#include "../Editor/EditorNode.h"
#include "../Editor/RenderPipelineEditor.h"

ExecutionEditorNode* NodeGraphCompiler::GetNextExecutorNode(ExecutionEditorNode* executorNode)
{
	ExecutionEditorNode* exNode = nullptr;

	switch (executorNode->GetType())
	{
	case EditorNodeType::Custom:
	{
		CustomEditorNode* customNode = static_cast<CustomEditorNode*>(executorNode);
		const auto fn = [&exNode](EditorNode* graphNode) {
			if (graphNode->GetType() == EditorNodeType::Pin) {
				PinEditorNode* pinNode = static_cast<PinEditorNode*>(graphNode);
				if (pinNode->GetPin().Type == PinType::Execution && !pinNode->GetPin().IsInput)
				{
					exNode = pinNode;
				}
			};
		};
		customNode->GetNodeGraph()->ForEachNode(fn);

		m_ContextStack.push({ customNode->GetNodeGraph(), customNode });
	} break;
	case EditorNodeType::Pin:
	{
		PinEditorNode* pinNode = static_cast<PinEditorNode*>(executorNode);
		ASSERT(pinNode->GetPin().Type == PinType::Execution);

		if (pinNode->GetPin().IsInput)
		{
			CustomEditorNode* parentNode = GetParentNode();
			m_ContextStack.pop();

			if (!parentNode) break;

			const PinID& outputExecutionPinID = parentNode->GetPin(pinNode->GetID());
			if (!outputExecutionPinID) break;

			const PinID inputExecutionPinID = GetNodeGraph()->GetInputPinFromOutput(outputExecutionPinID);
			if(!inputExecutionPinID) return (ExecutionEditorNode*) nullptr;

			// TODO: Do not use dynamic cast, but check node by type if it is execution
			exNode = dynamic_cast<ExecutionEditorNode*>(GetNodeGraph()->GetPinOwner(inputExecutionPinID));
		}
		else
		{
			const auto nextNodePin = GetNodeGraph()->GetInputPinFromOutput(pinNode->GetPin().ID);
			if (!nextNodePin) return (ExecutionEditorNode*) nullptr;
			EditorNode* node = GetNodeGraph()->GetPinOwner(nextNodePin);

			// TODO: Do not use dynamic cast, but check node by type if it is execution
			exNode = dynamic_cast<ExecutionEditorNode*>(node);
		}
	} break;
	default:
	{
		const auto nextNodePin = GetNodeGraph()->GetInputPinFromOutput(executorNode->GetExecutionOutput().ID);
		if (!nextNodePin) return (ExecutionEditorNode*) nullptr;
		EditorNode* node = GetNodeGraph()->GetPinOwner(nextNodePin);

		// TODO: Do not use dynamic cast, but check node by type if it is execution
		exNode = dynamic_cast<ExecutionEditorNode*>(node);
	}
	}

	if (!exNode)
	{
		ASSERT(0);
		exNode = new AsignFloatEditorNode{}; // Some random node so we don't crash
		m_ErrorMessages.push_back("Error in tracing flow of node execution.");
	}

	return exNode;
}

CompiledPipeline NodeGraphCompiler::Compile(const NodeGraph& graph)
{
	m_ErrorMessages.clear();

	m_ContextStack.push(NodeGraphCompilerContext{ &graph });

	CompiledPipeline pipeline{};
	pipeline.OnStartNode = Compile(GetNodeGraph()->GetOnStartNode());
	pipeline.OnUpdateNode = Compile(GetNodeGraph()->GetOnUpdateNode());

	m_ContextStack.pop();

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
		COMPILE_NODE(Pin, CompileEmptyNode, ExecutionEditorNode);
		COMPILE_NODE(Custom, CompileEmptyNode, ExecutionEditorNode);
	default:
		m_ErrorMessages.push_back("[NodeGraphCompiler::CompileExecutorNode] internal error!");
		NOT_IMPLEMENTED;
	}
	return new EmptyExecutorNode();
}

ExecutorNode* NodeGraphCompiler::CompileIfNode(IfEditorNode* ifNode)
{
	PinEvaluator pinEvaluator{ m_ContextStack };

	BoolValueNode* conditionNode = pinEvaluator.EvaluateBool(ifNode->GetConditionPin());
	
	ExecutorNode* elseBranch = nullptr;
	if (const NodeID elsePinInput = GetNodeGraph()->GetInputPinFromOutput(ifNode->GetExecutionElse().ID))
	{
		// TODO: Do not use dynamic cast, but check node by type if it is execution
		EditorNode* elseEditorNode = GetNodeGraph()->GetPinOwner(elsePinInput);
		ExecutionEditorNode* exElseEditorNode = dynamic_cast<ExecutionEditorNode*>(elseEditorNode);
		ASSERT(exElseEditorNode);
		
		if (exElseEditorNode)
		{
			const auto contextStack = m_ContextStack;
			elseBranch = Compile(exElseEditorNode);
			m_ContextStack = contextStack;
		}

	}
	return new IfExecutorNode{ conditionNode, elseBranch };
}

ExecutorNode* NodeGraphCompiler::CompilePrintNode(PrintEditorNode* printNode)
{
	PinID inputPinID = GetNodeGraph()->GetOutputPinForInput(printNode->GetFloatInputPin().ID);
	if (!inputPinID) inputPinID = GetNodeGraph()->GetOutputPinForInput(printNode->GetFloat2InputPin().ID);
	if (!inputPinID) inputPinID = GetNodeGraph()->GetOutputPinForInput(printNode->GetFloat3InputPin().ID);
	if (!inputPinID) inputPinID = GetNodeGraph()->GetOutputPinForInput(printNode->GetFloat4InputPin().ID);

	if (!inputPinID)
	{
		m_ErrorMessages.push_back("Print node missing outputs!");
		return new EmptyExecutorNode{};
	}

	PinEvaluator pinEvaluator{ m_ContextStack };
	EditorNodePin inputPin = GetNodeGraph()->GetPinByID(inputPinID);
	switch (inputPin.Type)
	{
	case PinType::Float: return new PrintExecutorNode{ pinEvaluator.EvaluateFloat(inputPin) };
	case PinType::Float2: return new PrintExecutorNode{ pinEvaluator.EvaluateFloat2(inputPin) };
	case PinType::Float3: return new PrintExecutorNode{ pinEvaluator.EvaluateFloat3(inputPin) };
	case PinType::Float4: return new PrintExecutorNode{ pinEvaluator.EvaluateFloat4(inputPin) };
	default:
		NOT_IMPLEMENTED;
	}
	return new EmptyExecutorNode{};
}

ExecutorNode* NodeGraphCompiler::CompileAsignVariableNode(AsignVariableEditorNode* node)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	StringValueNode* nameNode = pinEvaluator.EvaluateString(node->GetNamePin());

	const auto& asignValuePin = node->GetValuePin();
	switch (asignValuePin.Type)
	{
	case PinType::Float: return new AsignVariableExecutorNode<float>(nameNode, pinEvaluator.EvaluateFloat(asignValuePin));
	case PinType::Float2: return new AsignVariableExecutorNode<Float2>(nameNode, pinEvaluator.EvaluateFloat2(asignValuePin));
	case PinType::Float3: return new AsignVariableExecutorNode<Float3>(nameNode, pinEvaluator.EvaluateFloat3(asignValuePin));
	case PinType::Float4: return new AsignVariableExecutorNode<Float4>(nameNode, pinEvaluator.EvaluateFloat4(asignValuePin));
	case PinType::Bool:	return new AsignVariableExecutorNode<bool>(nameNode, pinEvaluator.EvaluateBool(asignValuePin));
	default:
		NOT_IMPLEMENTED;
	}
	return new EmptyExecutorNode{};
}

ExecutorNode* NodeGraphCompiler::CompileCreateTextureNode(CreateTextureEditorNode* createTextureNode)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	StringValueNode* nameNode = pinEvaluator.EvaluateString(createTextureNode->GetNamePin());
	return new CreateTextureExecutorNode{ nameNode, createTextureNode->GetWidth(), createTextureNode->GetHeight(), createTextureNode->IsFramebuffer(), createTextureNode->IsDepthStencil() };
}

ExecutorNode* NodeGraphCompiler::CompileClearRenderTargetNode(ClearRenderTargetEditorNode* clearRtNode)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	TextureValueNode* textureNode = pinEvaluator.EvaluateTexture(clearRtNode->GetTargeteTexturePin());
	Float4ValueNode* clearColorNode = pinEvaluator.EvaluateFloat4(clearRtNode->GetClearColorPin());
	return new ClearRenderTargetExecutorNode{ textureNode, clearColorNode };
}

ExecutorNode* NodeGraphCompiler::CompilePresentTextureTargetNode(PresentTextureEditorNode* presentTextureNode)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	TextureValueNode* textureNode = pinEvaluator.EvaluateTexture(presentTextureNode->GetTexturePin());
	return new PresentTextureExecutorNode{ textureNode };
}

ExecutorNode* NodeGraphCompiler::CompileLoadTextureNode(LoadTextureEditorNode* loadTextureNode)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	StringValueNode* nameNode = pinEvaluator.EvaluateString(loadTextureNode->GetNamePin());
	return new LoadTextureExecutorNode{ nameNode, loadTextureNode->GetPath() };
}

ExecutorNode* NodeGraphCompiler::CompileLoadShaderNode(LoadShaderEditorNode* loadShaderNode)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	StringValueNode* nameNode = pinEvaluator.EvaluateString(loadShaderNode->GetNamePin());
	return new LoadShaderExecutorNode{ nameNode, loadShaderNode->GetPath() };
}

ExecutorNode* NodeGraphCompiler::CompileDrawMeshNode(DrawMeshEditorNode* drawMeshNode)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	TextureValueNode* framebufferNode = pinEvaluator.EvaluateTexture(drawMeshNode->GetFrameBufferPin());
	ShaderValueNode* shaderNode = pinEvaluator.EvaluateShader(drawMeshNode->GetShaderPin());
	MeshValueNode* meshNode = pinEvaluator.EvaluateMesh(drawMeshNode->GetMeshPin());
	BindTableValueNode* bindTableNode = pinEvaluator.EvaluatePinOptional<BindTableValueNode, &PinEvaluator::EvaluateBindTable>(drawMeshNode->GetBindTablePin());
	RenderStateValueNode* renderStateNode = pinEvaluator.EvaluatePinOptional<RenderStateValueNode, &PinEvaluator::EvaluateRenderState>(drawMeshNode->GetRenderStatePin());

	return new DrawMeshExecutorNode{ framebufferNode, shaderNode, meshNode, bindTableNode, renderStateNode };
}

ExecutorNode* NodeGraphCompiler::CompileLoadMeshNode(LoadMeshEditorNode* loadMeshNode)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	StringValueNode* nameNode = pinEvaluator.EvaluateString(loadMeshNode->GetNamePin());
	return new LoadMeshExecutorNode{ nameNode, loadMeshNode->GetPath() };
}

ExecutorNode* NodeGraphCompiler::CompileEmptyNode(ExecutionEditorNode* node)
{
	return new EmptyExecutorNode{};
}