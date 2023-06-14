#pragma once

#include "EditorNode.h"

class EvaluationEditorNode : public EditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	using EditorNode::EditorNode;
};

class BoolEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	BoolEditorNode() :
		EvaluationEditorNode("Bool", EditorNodeType::Bool)
	{
		AddPin(EditorNodePin::CreateOutputPin("", PinType::Bool));
	}

	bool GetValue() const { return m_Value; }

protected:
	virtual void RenderContent() override;

private:
	bool m_Value = false;
};

class FloatNEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	FloatNEditorNode(unsigned numFloats, EditorNodeType nodeType, PinType pinType) :
		EvaluationEditorNode("Float" + (numFloats == 1 ? "" : std::to_string(numFloats)), nodeType),
		m_NumValues(numFloats)
	{
		AddPin(EditorNodePin::CreateOutputPin("", pinType));

		for (unsigned i = 0; i < 4; i++)
			m_Values[i] = 0.0f;
	}

	float GetFloat() const { ASSERT(m_NumValues == 1); return m_Values[0]; }
	glm::vec2 GetFloat2() const { ASSERT(m_NumValues == 2); return glm::vec2(m_Values[0], m_Values[1]); }
	glm::vec3 GetFloat3() const { ASSERT(m_NumValues == 3); return glm::vec3{ m_Values[0], m_Values[1], m_Values[2] }; }
	glm::vec4 GetFloat4() const { ASSERT(m_NumValues == 4); return glm::vec4{ m_Values[0], m_Values[1], m_Values[2], m_Values[3] }; }

protected:
	virtual void RenderContent() override;

private:
	unsigned m_NumValues;
	float m_Values[4];
};

template<EditorNodeType nodeType, PinType pinType, unsigned numFloats>
class FloatNEditorNodeT : public FloatNEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	FloatNEditorNodeT() :
		FloatNEditorNode(numFloats, nodeType, pinType) {}
};

class VariableEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	VariableEditorNode(EditorNodeType nodeType, PinType outputType) :
		EvaluationEditorNode("Get " + ToString(outputType), nodeType)
	{
		AddPin(EditorNodePin::CreateOutputPin("", outputType));
	}

	const std::string& GetVaraibleName() const { return m_VariableName; }

protected:
	virtual void RenderContent() override;

private:
	std::string m_VariableName;
};

template<EditorNodeType nodeType, PinType outputType>
class VariableEditorNodeT : public VariableEditorNode
{
public:
	VariableEditorNodeT() :
		VariableEditorNode(nodeType, outputType) {}
};

class BinaryOperatorEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	BinaryOperatorEditorNode(EditorNodeType nodeType, PinType pinType) :
		EvaluationEditorNode(ToString(pinType) + " bin op", nodeType)
	{
		m_Apin = AddPin(EditorNodePin::CreateInputPin("A", pinType));
		m_Bpin = AddPin(EditorNodePin::CreateInputPin("B", pinType));
		AddPin(EditorNodePin::CreateOutputPin("Result", pinType));
	}

	const EditorNodePin& GetAPin() const { return GetPins()[m_Apin]; }
	const EditorNodePin& GetBPin() const { return GetPins()[m_Bpin]; }
	char GetOp() const { return m_Op; }

protected:
	virtual void RenderContent() override;

private:
	unsigned m_Apin;
	unsigned m_Bpin;

	char m_Op = '+';
};

template<EditorNodeType nodeType, PinType pinType>
class BinaryOperatorEditorNodeT : public BinaryOperatorEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	BinaryOperatorEditorNodeT() :
		BinaryOperatorEditorNode(nodeType, pinType) {}
};

using FloatEditorNode = FloatNEditorNodeT<EditorNodeType::Float, PinType::Float, 1>;
using VarFloatEditorNode = VariableEditorNodeT<EditorNodeType::VarFloat, PinType::Float>;
using FloatBinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::FloatBinaryOperator, PinType::Float>;

using Float2EditorNode = FloatNEditorNodeT<EditorNodeType::Float2, PinType::Float2, 2>;
using VarFloat2EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat2, PinType::Float2>;
using Float2BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float2BinaryOperator, PinType::Float2>;

using Float3EditorNode = FloatNEditorNodeT<EditorNodeType::Float3, PinType::Float3, 3>;
using VarFloat3EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat3, PinType::Float3>;
using Float3BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float3BinaryOperator, PinType::Float3>;

using Float4EditorNode = FloatNEditorNodeT<EditorNodeType::Float4, PinType::Float4, 4>;
using VarFloat4EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat4, PinType::Float4>;
using Float4BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float4BinaryOperator, PinType::Float4>;

using GetTextureEditorNode = VariableEditorNodeT<EditorNodeType::GetTexture, PinType::Texture>;
using GetShaderEditorNode = VariableEditorNodeT<EditorNodeType::GetShader, PinType::Shader>;

class GetCubeMeshEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	GetCubeMeshEditorNode() :
		EvaluationEditorNode("Get cube mesh", EditorNodeType::GetCubeMesh) 
	{
		AddPin(EditorNodePin::CreateOutputPin("", PinType::Mesh));
	}
};
