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

private:
	BoolValueNode* EvaluateBool(BoolEditorNode* boolNode);

	FloatValueNode* EvaluateFloat(FloatEditorNode* floatNode);
	FloatValueNode* EvaluateVarFloat(VarFloatEditorNode* floatNode);
	FloatValueNode* EvaluateFloatBinaryOperator(FloatBinaryOperatorEditorNode* floatOpNode);

	Float2ValueNode* EvaluateFloat2(Float2EditorNode* floatNode);
	Float2ValueNode* EvaluateVarFloat2(VarFloat2EditorNode* floatNode);
	Float2ValueNode* EvaluateFloat2BinaryOperator(Float2BinaryOperatorEditorNode* floatOpNode);

	Float3ValueNode* EvaluateFloat3(Float3EditorNode* floatNode);
	Float3ValueNode* EvaluateVarFloat3(VarFloat3EditorNode* floatNode);
	Float3ValueNode* EvaluateFloat3BinaryOperator(Float3BinaryOperatorEditorNode* floatOpNode);

	Float4ValueNode* EvaluateFloat4(Float4EditorNode* floatNode);
	Float4ValueNode* EvaluateVarFloat4(VarFloat4EditorNode* floatNode);
	Float4ValueNode* EvaluateFloat4BinaryOperator(Float4BinaryOperatorEditorNode* floatOpNode);

	TextureValueNode* EvaluateGetTexture(GetTextureEditorNode* getTextureNode);

	MeshValueNode* EvaluateGetCubeMesh(GetCubeMeshEditorNode* getCubeMeshNode);

	ShaderValueNode* EvaluateGetShader(GetShaderEditorNode* getShaderNode);

private:
	const NodeGraph& m_NodeGraph;
	std::vector<std::string>& m_ErrorMessages;
};