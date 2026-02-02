#pragma once

#include <string>
#include <vector>
#include <stack>

#include "../Editor/EditorNode.h"
#include "../Editor/ExecutorEditorNode.h"
#include "../Editor/EvaluationEditorNode.h"
#include "PinEvaluator.h"

class NodeGraph;
class ExecutorNode;
struct CompiledPipeline;

class NodeGraphCompiler
{
	struct CompilerError
	{
		std::string Message;
		NodeID Node;
	};

	struct Context
	{
		std::unordered_map<ExecutorNode*, NodeID> EditorLinks;
	};

public:
	CompiledPipeline Compile(const NodeGraph& graph, const VariablePool& variablePool);
	const std::vector<CompilerError>& GetCompileErrors() const { return m_CompilationErrors; }

private:
	ExecutionEditorNode* GetNextExecutorNode(ExecutionEditorNode* executorNode);

	ExecutorNode* Compile(ExecutionEditorNode* executorNode, Context& context);
	ExecutorNode* CompileExecutorNode(ExecutionEditorNode* executorNode, Context& context);

	ExecutorNode* CompileEmptyNode(ExecutionEditorNode* node, Context& context);
	ExecutorNode* CompileIfNode(IfEditorNode* ifNode, Context& context);
	ExecutorNode* CompilePrintNode(PrintEditorNode* printNode, Context& context);
	ExecutorNode* CompileAsignVariableNode(AsignVariableEditorNode* asignFloatNode, Context& context);
	ExecutorNode* CompileClearRenderTargetNode(ClearRenderTargetEditorNode* clearRtNode, Context& context);
	ExecutorNode* CompilePresentTextureTargetNode(PresentTextureEditorNode* presentTextureNode, Context& context);
	ExecutorNode* CompileDrawMeshNode(DrawMeshEditorNode* drawMeshNode, Context& context);
	ExecutorNode* CompileForEachSceneObjectNode(ForEachSceneObjectEditorNode* forEachSceneObjectNode, Context& context);
	
private:
	const NodeGraph* GetNodeGraph() const { return m_ContextStack.top().Graph; }
	const VariablePool* GetVariablePool() const { return m_ContextStack.top().VariablePool; }
	CustomEditorNode* GetParentNode() const { return m_ContextStack.top().Parent; }

private:
	NodeGraphCompilerContextStack m_ContextStack;
	std::vector<CompilerError> m_CompilationErrors;
};