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

	ValueNode<glm::vec2>* EvaluateFloat2Pin(EditorNodePin pin);
	ValueNode<glm::vec2>* EvaluateFloat2(Float2EditorNode* floatNode);
	ValueNode<glm::vec2>* EvaluateVarFloat2(VarFloat2EditorNode* floatNode);
	ValueNode<glm::vec2>* EvaluateFloat2BinaryOperator(Float2BinaryOperatorEditorNode* floatOpNode);

	ValueNode<glm::vec3>* EvaluateFloat3Pin(EditorNodePin pin);
	ValueNode<glm::vec3>* EvaluateFloat3(Float3EditorNode* floatNode);
	ValueNode<glm::vec3>* EvaluateVarFloat3(VarFloat3EditorNode* floatNode);
	ValueNode<glm::vec3>* EvaluateFloat3BinaryOperator(Float3BinaryOperatorEditorNode* floatOpNode);

	ValueNode<glm::vec4>* EvaluateFloat4Pin(EditorNodePin pin);
	ValueNode<glm::vec4>* EvaluateFloat4(Float4EditorNode* floatNode);
	ValueNode<glm::vec4>* EvaluateVarFloat4(VarFloat4EditorNode* floatNode);
	ValueNode<glm::vec4>* EvaluateFloat4BinaryOperator(Float4BinaryOperatorEditorNode* floatOpNode);

	ExecutionEditorNode* GetNextExecutorNode(ExecutionEditorNode* executorNode);

	ExecutorNode* Compile(ExecutionEditorNode* executorNode);
	ExecutorNode* CompileExecutorNode(ExecutionEditorNode* executorNode);
	ExecutorNode* CompileIfNode(IfEditorNode* ifNode);
	ExecutorNode* CompilePrintNode(PrintEditorNode* printNode);
	ExecutorNode* CompileAsignVariableNode(AsignVariableEditorNode* asignFloatNode);
	ExecutorNode* CompileClearRenderTargetNode(ClearRenderTargetEditorNode* clearRtNode);

private:
	const NodeGraph* m_CurrentGraph = nullptr;

	std::vector<std::string> m_ErrorMessages;
};