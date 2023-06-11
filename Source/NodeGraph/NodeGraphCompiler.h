#pragma once

#include <string>
#include <vector>

#include "Editor/EditorNode.h"

class NodeGraph;
class ExecutorNode;
struct EditorNodePin;
struct CompiledPipeline;

template<typename T>
class ValueNode;

class NodeGraphCompiler
{
public:
	CompiledPipeline Compile(const NodeGraph& graph);

private:
	ValueNode<bool>* EvaluateBoolPin(EditorNodePin pin);
	ValueNode<bool>* EvaluateBool(BoolEditorNode* boolNode);

	ValueNode<float>* EvaluateFloatPin(EditorNodePin pin);
	ValueNode<float>* EvaluateFloat(FloatEditorNode* floatNode);
	ValueNode<float>* EvaluateVarFloat(VarFloatEditorNode* floatNode);
	ValueNode<float>* EvaluateFloatBinaryOperator(FloatBinaryOperatorEditorNode* floatOpNode);

	ExecutionEditorNode* GetNextExecutorNode(ExecutionEditorNode* executorNode);

	ExecutorNode* Compile(ExecutionEditorNode* executorNode);
	ExecutorNode* CompileExecutorNode(ExecutionEditorNode* executorNode);
	ExecutorNode* CompileIfNode(IfEditorNode* ifNode);
	ExecutorNode* CompilePrintNode(PrintEditorNode* printNode);
	ExecutorNode* CompileAsignFloatNode(AsignFloatEditorNode* asignFloatNode);

private:
	const NodeGraph* m_CurrentGraph = nullptr;

	std::vector<std::string> m_ErrorMessages;
};