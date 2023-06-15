#pragma once

#include <vector>
#include <string>

#include "NodeGraph.h"
#include "../Execution/ValueNode.h"
#include "../Editor/EvaluationEditorNode.h"

class PinEvaluator
{
public:
	PinEvaluator(const NodeGraph& nodeGraph, std::vector<std::string>& errorMessages):
		m_NodeGraph(nodeGraph),
	m_ErrorMessages(errorMessages) {}

	BoolValueNode* EvaluateBool(EditorNodePin pin);
	FloatValueNode* EvaluateFloat(EditorNodePin pin);
	Float2ValueNode* EvaluateFloat2(EditorNodePin pin);
	Float3ValueNode* EvaluateFloat3(EditorNodePin pin);
	Float4ValueNode* EvaluateFloat4(EditorNodePin pin);
	TextureValueNode* EvaluateTexture(EditorNodePin pin);
	BufferValueNode* EvaluateBuffer(EditorNodePin pin);
	MeshValueNode* EvaluateMesh(EditorNodePin pin);
	ShaderValueNode* EvaluateShader(EditorNodePin pin);
	BindTableValueNode* EvaluateBindTable(EditorNodePin pin);

private:
	BoolValueNode* EvaluateBool(BoolEditorNode* node);
	BoolValueNode* EvaluateVarBool(VarBoolEditorNode* node);
	BoolValueNode* EvaluateBoolBinaryOperator(BoolBinaryOperatorEditorNode* node);
	BoolValueNode* EvaluateFloatComparisonOperator(FloatComparisonOperatorEditorNode* node);

	FloatValueNode* EvaluateFloat(FloatEditorNode* node);
	FloatValueNode* EvaluateVarFloat(VarFloatEditorNode* node);
	FloatValueNode* EvaluateSplitFloat2(SplitFloat2EditorNode* node, const EditorNodePin& pin);
	FloatValueNode* EvaluateSplitFloat3(SplitFloat3EditorNode* node, const EditorNodePin& pin);
	FloatValueNode* EvaluateSplitFloat4(SplitFloat4EditorNode* node, const EditorNodePin& pin);
	FloatValueNode* EvaluateFloatBinaryOperator(FloatBinaryOperatorEditorNode* node);

	Float2ValueNode* EvaluateFloat2(Float2EditorNode* node);
	Float2ValueNode* EvaluateCreateFloat2(CreateFloat2EditorNode* node);
	Float2ValueNode* EvaluateVarFloat2(VarFloat2EditorNode* node);
	Float2ValueNode* EvaluateFloat2BinaryOperator(Float2BinaryOperatorEditorNode* node);

	Float3ValueNode* EvaluateFloat3(Float3EditorNode* node);
	Float3ValueNode* EvaluateCreateFloat3(CreateFloat3EditorNode* node);
	Float3ValueNode* EvaluateVarFloat3(VarFloat3EditorNode* node);
	Float3ValueNode* EvaluateFloat3BinaryOperator(Float3BinaryOperatorEditorNode* node);

	Float4ValueNode* EvaluateFloat4(Float4EditorNode* node);
	Float4ValueNode* EvaluateCreateFloat4(CreateFloat4EditorNode* node);
	Float4ValueNode* EvaluateVarFloat4(VarFloat4EditorNode* node);
	Float4ValueNode* EvaluateFloat4BinaryOperator(Float4BinaryOperatorEditorNode* node);

	TextureValueNode* EvaluateGetTexture(GetTextureEditorNode* node);

	MeshValueNode* EvaluateGetMesh(GetMeshEditorNode* node);
	MeshValueNode* EvaluateGetCubeMesh(GetCubeMeshEditorNode* node);

	ShaderValueNode* EvaluateGetShader(GetShaderEditorNode* node);

	BindTableValueNode* EvaluateBindTable(BindTableEditorNode* node);

private:
	const NodeGraph& m_NodeGraph;
	std::vector<std::string>& m_ErrorMessages;
};