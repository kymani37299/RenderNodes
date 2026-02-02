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

		m_ContextStack.push({ customNode->GetNodeGraph(), GetVariablePool(), customNode });
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
		exNode = new IfEditorNode{}; // Some random node so we don't crash
		m_CompilationErrors.push_back({ "Error in tracing flow of node execution.", executorNode->GetID() });
	}

	return exNode;
}

CompiledPipeline NodeGraphCompiler::Compile(const NodeGraph& graph, const VariablePool& variablePool)
{
	m_CompilationErrors.clear();

	Context context;

	m_ContextStack.push(NodeGraphCompilerContext{ &graph, &variablePool });

	CompiledPipeline pipeline{};
	pipeline.OnStartNode = Compile(GetNodeGraph()->GetOnStartNode(), context);
	pipeline.OnUpdateNode = Compile(GetNodeGraph()->GetOnUpdateNode(), context);

	const auto compileInputNodes = [this, &graph, &context](
		EditorNodeType nodeType,
		std::unordered_map<uint32_t, ExecutorNode*>& inputMap)
	{
		const auto getInputNodes = [&graph](EditorNodeType nodeType)
		{
			std::vector<InputExecutionEditorNode*> inputNodes{};
			const auto fn = [&inputNodes, &nodeType](EditorNode* node) {
				if (node->GetType() == nodeType)
					inputNodes.push_back(static_cast<InputExecutionEditorNode*>(node));
			};
			graph.ForEachNode(fn);
			return inputNodes;
		};

		const auto inputNodes = getInputNodes(nodeType);

		for (const auto& inputNode : inputNodes)
		{
			const uint32_t inputHash = ExecutorInputState::GetInputHash(inputNode->GetKey(), inputNode->GetMods());
			inputMap[inputHash] = Compile(inputNode, context);
		}
	};

	compileInputNodes(EditorNodeType::OnKeyPressed, pipeline.OnKeyPressedNodes);
	compileInputNodes(EditorNodeType::OnKeyReleased, pipeline.OnKeyReleasedNodes);
	compileInputNodes(EditorNodeType::OnKeyDown, pipeline.OnKeyDownNodes);

	m_ContextStack.pop();

	pipeline.EditorLinks = context.EditorLinks;
	pipeline.VariablePool = variablePool;
	return pipeline;
}

ExecutorNode* NodeGraphCompiler::Compile(ExecutionEditorNode* executorNode, Context& context)
{
	ExecutorNode* firstNode = new EmptyExecutorNode();
	ExecutorNode* currentNode = firstNode;
	ExecutionEditorNode* currentEditorNode = executorNode;

	while (currentEditorNode)
	{
		ExecutorNode* nextNode = CompileExecutorNode(currentEditorNode, context);
		currentNode->SetNextNode(nextNode);
		currentNode = nextNode;
		currentEditorNode = GetNextExecutorNode(currentEditorNode);
	}
	return firstNode;
}

#define COMPILE_NODE(NodeType, Compiler, SpecificType) case EditorNodeType::NodeType: compiledNode = Compiler(static_cast<SpecificType*>(executorNode), context); break

ExecutorNode* NodeGraphCompiler::CompileExecutorNode(ExecutionEditorNode* executorNode, Context& context)
{
	ExecutorNode* compiledNode = nullptr;
	switch (executorNode->GetType())
	{
		COMPILE_NODE(Print, CompilePrintNode, PrintEditorNode);
		COMPILE_NODE(If, CompileIfNode, IfEditorNode);
		COMPILE_NODE(ClearRenderTarget, CompileClearRenderTargetNode, ClearRenderTargetEditorNode);
		COMPILE_NODE(PresentTexture, CompilePresentTextureTargetNode, PresentTextureEditorNode);
		COMPILE_NODE(DrawMesh, CompileDrawMeshNode, DrawMeshEditorNode);
		COMPILE_NODE(ForEachSceneObject, CompileForEachSceneObjectNode, ForEachSceneObjectEditorNode);
		COMPILE_NODE(AsignVariable, CompileAsignVariableNode, AsignVariableEditorNode);
		COMPILE_NODE(OnStart, CompileEmptyNode, ExecutionEditorNode);
		COMPILE_NODE(OnUpdate, CompileEmptyNode, ExecutionEditorNode);
		COMPILE_NODE(OnKeyPressed, CompileEmptyNode, ExecutionEditorNode);
		COMPILE_NODE(OnKeyReleased, CompileEmptyNode, ExecutionEditorNode);
		COMPILE_NODE(OnKeyDown, CompileEmptyNode, ExecutionEditorNode);
		COMPILE_NODE(Pin, CompileEmptyNode, ExecutionEditorNode);
		COMPILE_NODE(Custom, CompileEmptyNode, ExecutionEditorNode);
	default:
		compiledNode = new EmptyExecutorNode();
		m_CompilationErrors.push_back({ "[NodeGraphCompiler::CompileExecutorNode] internal error!", executorNode->GetID() });
		NOT_IMPLEMENTED;
	}

	context.EditorLinks[compiledNode] = executorNode->GetID();

	return compiledNode;
}

ExecutorNode* NodeGraphCompiler::CompileIfNode(IfEditorNode* ifNode, Context& context)
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
			elseBranch = Compile(exElseEditorNode, context);
			m_ContextStack = contextStack;
		}

	}
	return new IfExecutorNode{ conditionNode, elseBranch };
}

ExecutorNode* NodeGraphCompiler::CompilePrintNode(PrintEditorNode* printNode, Context& context)
{
	const auto getValuePin = [this](const EditorNodePin& pin)
	{
		if (pin.HasConstantValue)
			return pin.ID;
		return GetNodeGraph()->GetOutputPinForInput(pin.ID);
	};

	PinID inputPinID = GetNodeGraph()->GetOutputPinForInput(printNode->GetInputPin().ID);

	if (!inputPinID)
	{
		m_CompilationErrors.push_back({ "Print node missing outputs!", printNode->GetID() });
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
	case PinType::Int: return new PrintExecutorNode{ pinEvaluator.EvaluateInt(inputPin) };
	case PinType::Bool: return new PrintExecutorNode{ pinEvaluator.EvaluateBool(inputPin) };
	case PinType::String: return new PrintExecutorNode{ pinEvaluator.EvaluateString(inputPin) };
	default:
		m_CompilationErrors.push_back({ "Not supported type for printing!", printNode->GetID() });
	}
	return new EmptyExecutorNode{};
}

ExecutorNode* NodeGraphCompiler::CompileAsignVariableNode(AsignVariableEditorNode* node, Context& context)
{
	PinEvaluator pinEvaluator{ m_ContextStack };

	const VariableID varID = node->GetVariableID();
	const EditorNodePin& valuePin = node->GetValuePin();
	const Variable& variable = GetVariablePool()->GetRef(varID);
	ASSERT(variable.Type != VariableType::Invalid);

	switch (variable.Type)
	{
	case VariableType::Bool:		return new AsignVariableExecutorNode<bool>(varID, pinEvaluator.EvaluateBool(valuePin));
	case VariableType::Int:			return new AsignVariableExecutorNode<int>(varID, pinEvaluator.EvaluateInt(valuePin));
	case VariableType::Float:		return new AsignVariableExecutorNode<float>(varID, pinEvaluator.EvaluateFloat(valuePin));
	case VariableType::Float2:		return new AsignVariableExecutorNode<Float2>(varID, pinEvaluator.EvaluateFloat2(valuePin));
	case VariableType::Float3:		return new AsignVariableExecutorNode<Float3>(varID, pinEvaluator.EvaluateFloat3(valuePin));
	case VariableType::Float4:		return new AsignVariableExecutorNode<Float4>(varID, pinEvaluator.EvaluateFloat4(valuePin));
	case VariableType::Float4x4:	return new AsignVariableExecutorNode<Float4x4>(varID, pinEvaluator.EvaluateFloat4x4(valuePin));

	case VariableType::Shader:
	case VariableType::Texture:
	case VariableType::Scene:
		ASSERT_M(0, "Tring to compile AsignVariableEditorNode for the variable types that shouldn't be asigned.");
		break;
	case VariableType::Invalid:
		break;
	default:
		NOT_IMPLEMENTED;
		break;
	}
	return new EmptyExecutorNode{};
}

ExecutorNode* NodeGraphCompiler::CompileClearRenderTargetNode(ClearRenderTargetEditorNode* clearRtNode, Context& context)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	TextureValueNode* textureNode = pinEvaluator.EvaluateTexture(clearRtNode->GetTargeteTexturePin());
	Float4ValueNode* clearColorNode = pinEvaluator.EvaluateFloat4(clearRtNode->GetClearColorPin());
	return new ClearRenderTargetExecutorNode{ textureNode, clearColorNode };
}

ExecutorNode* NodeGraphCompiler::CompilePresentTextureTargetNode(PresentTextureEditorNode* presentTextureNode, Context& context)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	TextureValueNode* textureNode = pinEvaluator.EvaluateTexture(presentTextureNode->GetTexturePin());
	return new PresentTextureExecutorNode{ textureNode };
}

ExecutorNode* NodeGraphCompiler::CompileDrawMeshNode(DrawMeshEditorNode* drawMeshNode, Context& context)
{
	PinEvaluator pinEvaluator{ m_ContextStack };
	TextureValueNode* framebufferNode = pinEvaluator.EvaluateTexture(drawMeshNode->GetFrameBufferPin());
	ShaderValueNode* shaderNode = pinEvaluator.EvaluateShader(drawMeshNode->GetShaderPin());
	MeshValueNode* meshNode = pinEvaluator.EvaluateMesh(drawMeshNode->GetMeshPin());
	BindTableValueNode* bindTableNode = pinEvaluator.EvaluatePinOptional<BindTableValueNode, &PinEvaluator::EvaluateBindTable>(drawMeshNode->GetBindTablePin());
	RenderStateValueNode* renderStateNode = pinEvaluator.EvaluatePinOptional<RenderStateValueNode, &PinEvaluator::EvaluateRenderState>(drawMeshNode->GetRenderStatePin());

	return new DrawMeshExecutorNode{ framebufferNode, shaderNode, meshNode, bindTableNode, renderStateNode };
}

ExecutorNode* NodeGraphCompiler::CompileForEachSceneObjectNode(ForEachSceneObjectEditorNode* forEachSceneObjectNode, Context& context)
{
	PinEvaluator pinEvaluator{ m_ContextStack };

	const NodeID executionLoopOutput = GetNodeGraph()->GetInputPinFromOutput(forEachSceneObjectNode->GetLoopPin().ID);
	if (!executionLoopOutput)
		return new EmptyExecutorNode{};

	EditorNode* executionLoopFirstNode = GetNodeGraph()->GetPinOwner(executionLoopOutput);
	ExecutionEditorNode* exExecutionLoopFirstNode = dynamic_cast<ExecutionEditorNode*>(executionLoopFirstNode);
	ASSERT(exExecutionLoopFirstNode);

	if (!exExecutionLoopFirstNode)
		return new EmptyExecutorNode{};

	SceneValueNode* sceneNode = pinEvaluator.EvaluateScene(forEachSceneObjectNode->GetScenePin());

	const auto contextStack = m_ContextStack;
	ExecutorNode* executionLoopNode = Compile(exExecutionLoopFirstNode, context);
	m_ContextStack = contextStack;

	return new ForEachSceneObjectExecutorNode{ sceneNode, forEachSceneObjectNode->GetSceneObjectPin().ID, executionLoopNode };
}

ExecutorNode* NodeGraphCompiler::CompileEmptyNode(ExecutionEditorNode* node, Context& context)
{
	return new EmptyExecutorNode{};
}