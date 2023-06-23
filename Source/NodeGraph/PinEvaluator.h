#pragma once

#include <vector>
#include <string>
#include <stack>

#include "NodeGraph.h"
#include "../Execution/ValueNode.h"
#include "../Editor/EvaluationEditorNode.h"

inline EditorNodePin GetOutputPinIfInput(const NodeGraph& nodeGraph, EditorNodePin pin)
{
	if (pin.IsInput)
	{
		const PinID outPinID = nodeGraph.GetOutputPinForInput(pin.ID);
		if (!outPinID) return EditorNodePin{};
		pin = nodeGraph.GetPinByID(outPinID);
	}
	return pin;
}

class PinEvaluator;

struct NodeGraphCompilerContext
{
	NodeGraphCompilerContext(const NodeGraph* graph, CustomEditorNode* parent = nullptr):
		Graph(graph),
		Parent(parent) {}

	const NodeGraph* Graph;
	CustomEditorNode* Parent = nullptr;
};
using NodeGraphCompilerContextStack = std::stack<NodeGraphCompilerContext>;

class PinEvaluator
{
public:
	PinEvaluator(const NodeGraphCompilerContextStack& contextStack)
	{
		m_ContextStack = contextStack;
	}

	PinEvaluator(const NodeGraph& graph)
	{
		m_ContextStack.push({&graph});
	}

	template<typename ReturnType, ReturnType* (PinEvaluator::* func)(EditorNodePin)>
	ReturnType* EvaluatePinOptional(EditorNodePin pin)
	{
		pin = GetOutputPinIfInput(*GetNodeGraph(), pin);
		if (pin.Type == PinType::Invalid) return nullptr;
		else return (this->*func)(pin);
	}

	template<typename ReturnType>
	ReturnType* EvaluatePinOptional(EditorNodePin pin)
	{
		return nullptr;
	}

	template<typename ReturnType>
	ReturnType* EvaluateCustomNode(CustomEditorNode* node, const EditorNodePin& pin)
	{
		m_ContextStack.push({ node->GetNodeGraph(), node });

		PinEditorNode* pinNode = static_cast<PinEditorNode*>(GetNodeGraph()->GetNodeByID(node->GetPinNode(pin.ID)));
		ReturnType* returnValue = EvaluatePin<ReturnType>(pinNode->GetPin());

		m_ContextStack.pop();

		return returnValue;
	}

	template<typename ReturnType>
	ReturnType* EvaluatePinNode(PinEditorNode* node)
	{
		ASSERT(m_ContextStack.top().Parent);

		const PinID customNodePinID = GetParentNode()->GetPin(node->GetID());

		const NodeGraphCompilerContext currentContext = m_ContextStack.top();
		m_ContextStack.pop();

		const EditorNodePin customNodePin = GetNodeGraph()->GetPinByID(customNodePinID);
		ReturnType* value = EvaluatePin<ReturnType>(customNodePin);

		m_ContextStack.push(currentContext);

		return value;
	}

	BoolValueNode* EvaluateBool(EditorNodePin pin);
	StringValueNode* EvaluateString(EditorNodePin pin);
	FloatValueNode* EvaluateFloat(EditorNodePin pin);
	Float2ValueNode* EvaluateFloat2(EditorNodePin pin);
	Float3ValueNode* EvaluateFloat3(EditorNodePin pin);
	Float4ValueNode* EvaluateFloat4(EditorNodePin pin);
	TextureValueNode* EvaluateTexture(EditorNodePin pin);
	BufferValueNode* EvaluateBuffer(EditorNodePin pin);
	MeshValueNode* EvaluateMesh(EditorNodePin pin);
	ShaderValueNode* EvaluateShader(EditorNodePin pin);
	BindTableValueNode* EvaluateBindTable(EditorNodePin pin);
	RenderStateValueNode* EvaluateRenderState(EditorNodePin pin);

	template<typename ReturnType> ReturnType* EvaluatePin(EditorNodePin pin);
	template<> BoolValueNode* EvaluatePin<BoolValueNode>(EditorNodePin pin) { return EvaluateBool(pin); }
	template<> StringValueNode* EvaluatePin<StringValueNode>(EditorNodePin pin) { return EvaluateString(pin); }
	template<> FloatValueNode* EvaluatePin<FloatValueNode>(EditorNodePin pin) { return EvaluateFloat(pin); }
	template<> Float2ValueNode* EvaluatePin<Float2ValueNode>(EditorNodePin pin) { return EvaluateFloat2(pin); }
	template<> Float3ValueNode* EvaluatePin<Float3ValueNode>(EditorNodePin pin) { return EvaluateFloat3(pin); }
	template<> Float4ValueNode* EvaluatePin<Float4ValueNode>(EditorNodePin pin) { return EvaluateFloat4(pin); }
	template<> TextureValueNode* EvaluatePin<TextureValueNode>(EditorNodePin pin) { return EvaluateTexture(pin); }
	template<> BufferValueNode* EvaluatePin<BufferValueNode>(EditorNodePin pin) { return EvaluateBuffer(pin); }
	template<> MeshValueNode* EvaluatePin<MeshValueNode>(EditorNodePin pin) { return EvaluateMesh(pin); }
	template<> ShaderValueNode* EvaluatePin<ShaderValueNode>(EditorNodePin pin) { return EvaluateShader(pin); }
	template<> BindTableValueNode* EvaluatePin<BindTableValueNode>(EditorNodePin pin) { return EvaluateBindTable(pin); }
	template<> RenderStateValueNode* EvaluatePin<RenderStateValueNode>(EditorNodePin pin) { return EvaluateRenderState(pin); }

private:
	BoolValueNode* EvaluateBool(BoolEditorNode* node);
	BoolValueNode* EvaluateVarBool(VarBoolEditorNode* node);
	BoolValueNode* EvaluateBoolBinaryOperator(BoolBinaryOperatorEditorNode* node);
	BoolValueNode* EvaluateFloatComparisonOperator(FloatComparisonOperatorEditorNode* node);

	StringValueNode* EvaluateString(StringEditorNode* node);

	FloatValueNode* EvaluateFloat(FloatEditorNode* node);
	FloatValueNode* EvaluateVarFloat(VarFloatEditorNode* node);
	FloatValueNode* EvaluateSplitFloat2(SplitFloat2EditorNode* node, const EditorNodePin& pin);
	FloatValueNode* EvaluateSplitFloat3(SplitFloat3EditorNode* node, const EditorNodePin& pin);
	FloatValueNode* EvaluateSplitFloat4(SplitFloat4EditorNode* node, const EditorNodePin& pin);
	FloatValueNode* EvaluateFloatBinaryOperator(FloatBinaryOperatorEditorNode* node);
	FloatValueNode* EvaluateDeltaTime(OnUpdateEditorNode* node);

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

	RenderStateValueNode* EvaluateRenderState(RenderStateEditorNode* node);

private:
	const NodeGraph* GetNodeGraph() const { return m_ContextStack.top().Graph; }
	CustomEditorNode* GetParentNode() const { return m_ContextStack.top().Parent; }

private:
	NodeGraphCompilerContextStack m_ContextStack;
	std::vector<std::string> m_ErrorMessages;
};