#pragma once

#include <string>
#include <vector>

#include "../Editor/EditorNode.h"
#include "../Editor/ExecutorEditorNode.h"
#include "../Editor/EvaluationEditorNode.h"
#include "PinEvaluator.h"

class NodeGraph;
class ExecutorNode;
struct CompiledPipeline;

class NodeGraphCompiler
{
public:
	CompiledPipeline Compile(const NodeGraph& graph);

	const std::vector<std::string> GetErrorMessages() const { return m_ErrorMessages; }

private:


	ExecutionEditorNode* GetNextExecutorNode(ExecutionEditorNode* executorNode);

	ExecutorNode* Compile(ExecutionEditorNode* executorNode);
	ExecutorNode* CompileExecutorNode(ExecutionEditorNode* executorNode);

	ExecutorNode* CompileEmptyNode(ExecutionEditorNode* node);
	ExecutorNode* CompileIfNode(IfEditorNode* ifNode);
	ExecutorNode* CompilePrintNode(PrintEditorNode* printNode);
	ExecutorNode* CompileAsignVariableNode(AsignVariableEditorNode* asignFloatNode);
	ExecutorNode* CompileCreateTextureNode(CreateTextureEditorNode* createTextureNode);
	ExecutorNode* CompileClearRenderTargetNode(ClearRenderTargetEditorNode* clearRtNode);
	ExecutorNode* CompilePresentTextureTargetNode(PresentTextureEditorNode* presentTextureNode);
	ExecutorNode* CompileLoadTextureNode(LoadTextureEditorNode* loadTextureNode);
	ExecutorNode* CompileLoadShaderNode(LoadShaderEditorNode* loadShaderNode);
	ExecutorNode* CompileDrawMeshNode(DrawMeshEditorNode* drawMeshNode);
	ExecutorNode* CompileLoadMeshNode(LoadMeshEditorNode* loadMeshNode);

private:
	Ptr<PinEvaluator> m_PinEvaluator = nullptr;
	const NodeGraph* m_CurrentGraph = nullptr;

	std::vector<std::string> m_ErrorMessages;
};