#pragma once

#include <string>
#include <vector>

class NodeGraph;
class ExecutorNode;
struct EditorNodePin;
struct CompiledPipeline;

class BoolEditorNode;
class FloatEditorNode;
class IfEditorNode;
class PrintEditorNode;
class AddFloatEditorNode;
class ExecutionEditorNode;
class FloatBinaryOperatorEditorNode;

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
	ValueNode<float>* EvaluateFloatBinaryOperator(FloatBinaryOperatorEditorNode* floatOpNode);

	ExecutionEditorNode* GetNextExecutorNode(ExecutionEditorNode* executorNode);

	ExecutorNode* Compile(ExecutionEditorNode* executorNode);
	ExecutorNode* CompileExecutorNode(ExecutionEditorNode* executorNode);
	ExecutorNode* CompileIfNode(IfEditorNode* ifNode);
	ExecutorNode* CompilePrintNode(PrintEditorNode* printNode);

private:
	const NodeGraph* m_CurrentGraph = nullptr;

	std::vector<std::string> m_ErrorMessages;
};