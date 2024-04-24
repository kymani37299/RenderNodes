#pragma once

#include "EditorNode.h"
#include "Drawing/EditorContextMenu.h"

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

	EditorNode* Clone() const override
	{
		BoolEditorNode* node = new BoolEditorNode();
		node->m_Value = m_Value;
		return node;
	}

	bool GetValue() const { return m_Value; }

protected:
	virtual void RenderContent() override;

private:
	bool m_Value = false;
};

class IntEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	IntEditorNode() :
		EvaluationEditorNode("Int", EditorNodeType::Int)
	{
		AddPin(EditorNodePin::CreateOutputPin("", PinType::Int));
	}

	EditorNode* Clone() const override
	{
		IntEditorNode* node = new IntEditorNode();
		node->m_Value = m_Value;
		return node;
	}

	int GetValue() const { return m_Value; }

protected:
	virtual void RenderContent() override;

private:
	int m_Value = false;
};

class StringEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	StringEditorNode() :
		EvaluationEditorNode("String", EditorNodeType::String)
	{
		AddPin(EditorNodePin::CreateOutputPin("", PinType::String));
	}

	EditorNode* Clone() const override
	{
		StringEditorNode* node = new StringEditorNode();
		node->m_Value = m_Value;
		return node;
	}

	const std::string& GetValue() const { return m_Value; }

protected:
	virtual void RenderContent() override;

private:
	std::string m_Value;
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
	Float2 GetFloat2() const { ASSERT(m_NumValues == 2); return Float2(m_Values[0], m_Values[1]); }
	Float3 GetFloat3() const { ASSERT(m_NumValues == 3); return Float3{ m_Values[0], m_Values[1], m_Values[2] }; }
	Float4 GetFloat4() const { ASSERT(m_NumValues == 4); return Float4{ m_Values[0], m_Values[1], m_Values[2], m_Values[3] }; }

protected:
	virtual void RenderContent() override;

private:
	unsigned m_NumValues;

protected: // Should be private
	float m_Values[4];
};

template<EditorNodeType nodeType, PinType pinType, unsigned numFloats>
class FloatNEditorNodeT : public FloatNEditorNode
{
	static_assert(numFloats <= 4 && numFloats > 0);

	SERIALIZEABLE_EDITOR_NODE();
public:
	FloatNEditorNodeT() :
		FloatNEditorNode(numFloats, nodeType, pinType) {}

	EditorNode* Clone() const override
	{
		FloatNEditorNodeT* node = new FloatNEditorNodeT<nodeType, pinType, numFloats>();
		for (unsigned i = 0; i < 4; i++)
			node->m_Values[i] = m_Values[i];
		return node;
	}
};

class FloatNxNEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	FloatNxNEditorNode(unsigned numFloatsX, unsigned numFloatsY, EditorNodeType nodeType, PinType pinType) :
		EvaluationEditorNode("Float" + std::to_string(numFloatsX) + "x" + std::to_string(numFloatsY), nodeType),
		m_NumValuesX(numFloatsX),
		m_NumValuesY(numFloatsY)
	{
		AddPin(EditorNodePin::CreateOutputPin("", pinType));

		for (unsigned i = 0; i < 4; i++)
			for(unsigned j = 0; j < 4; j++)
				m_Values[i][j] = i == j ? 1.0f : 0.0f;
	}

	// TODO: Check if this is correct row/column order
	Float4x4 GetFloat4x4() const { ASSERT(m_NumValuesX == 4 && m_NumValuesY == 4); return Float4x4{	m_Values[0][0], m_Values[0][1], m_Values[0][2], m_Values[0][3],
																									m_Values[1][0], m_Values[1][1], m_Values[1][2], m_Values[1][3],
																									m_Values[2][0], m_Values[2][1], m_Values[2][2], m_Values[2][3],
																									m_Values[3][0], m_Values[3][1], m_Values[3][2], m_Values[3][3], }; }
protected:
	virtual void RenderContent() override;

private:
	unsigned m_NumValuesX;
	unsigned m_NumValuesY;

protected: // Should be private
	float m_Values[4][4];
};

template<EditorNodeType nodeType, PinType pinType, unsigned numFloatsX, unsigned numFloatsY>
class FloatNxNEditorNodeT : public FloatNxNEditorNode
{
	static_assert(numFloatsX <= 4 && numFloatsX > 0);
	static_assert(numFloatsY <= 4 && numFloatsY > 0);

	SERIALIZEABLE_EDITOR_NODE();
public:
	FloatNxNEditorNodeT() :
		FloatNxNEditorNode(numFloatsX, numFloatsY, nodeType, pinType) {}

	EditorNode* Clone() const override
	{
		FloatNxNEditorNodeT* node = new FloatNxNEditorNodeT<nodeType, pinType, numFloatsX, numFloatsY>();
		for (unsigned i = 0; i < 4; i++)
			for(unsigned j = 0; j < 4; j++)
				node->m_Values[i][j] = m_Values[i][j];
		return node;
	}
};

class SplitVectorEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	SplitVectorEditorNode(EditorNodeType nodeType, PinType inputPinType, PinType outputPinType, unsigned numOutputs) :
		EvaluationEditorNode("Split " + ToString(inputPinType), nodeType),
		m_NumOutputs(numOutputs)
	{
		static std::vector<std::string> outputLabels = { "x", "y", "z", "w" };
		m_InputVectorPin = AddPin(EditorNodePin::CreateInputPin("", inputPinType));
		for (unsigned i = 0; i < numOutputs; i++)
			m_OutputPins[i] = AddPin(EditorNodePin::CreateOutputPin(outputLabels[i], outputPinType));
	}

	unsigned GetOutputPinIndex(PinID pinID)
	{
		for (unsigned i = 0; i < m_NumOutputs; i++)
		{
			if (pinID == GetPins()[m_OutputPins[i]].ID)
				return i;
		}
		ASSERT(0);
		return 0;
	}

	const EditorNodePin& GetInputVectorPin() const { return GetPins()[m_InputVectorPin]; }

private:
	unsigned m_NumOutputs;
	unsigned m_InputVectorPin;
	unsigned m_OutputPins[4];
};

template<EditorNodeType nodeType, PinType inputPinType, PinType outputPinType, unsigned numOutputs>
class SplitVectorEditorNodeT : public SplitVectorEditorNode
{
	static_assert(numOutputs <= 4 && numOutputs > 0);

	SERIALIZEABLE_EDITOR_NODE();
public:
	SplitVectorEditorNodeT() :
		SplitVectorEditorNode(nodeType, inputPinType, outputPinType, numOutputs)
	{ }

	EditorNode* Clone() const override
	{
		return new SplitVectorEditorNodeT<nodeType, inputPinType, outputPinType, numOutputs>();
	}
};

class CreateVectorEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	CreateVectorEditorNode(EditorNodeType nodeType, PinType inputPinType, PinType outputPinType, unsigned numInputs) :
		EvaluationEditorNode("Create " + ToString(outputPinType), nodeType)
	{
		static std::vector<std::string> inputLabels = { "x", "y", "z", "w" };

		for (unsigned i = 0; i < numInputs; i++)
			m_InputPins[i] = AddPin(EditorNodePin::CreateInputPin(inputLabels[i], inputPinType));
		AddPin(EditorNodePin::CreateOutputPin("", outputPinType));
	}

	const EditorNodePin& GetInputPin(unsigned index) { return GetPins()[m_InputPins[index]]; }

private:
	unsigned m_InputPins[4];
};

template<EditorNodeType nodeType, PinType inputPinType, PinType outputPinType, unsigned numInputs>
class CreateVectorEditorNodeT : public CreateVectorEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	CreateVectorEditorNodeT() :
		CreateVectorEditorNode(nodeType, inputPinType, outputPinType, numInputs) {}

	EditorNode* Clone() const override
	{
		return new CreateVectorEditorNodeT<nodeType, inputPinType, outputPinType, numInputs>();
	}

	static_assert(numInputs <= 4 && numInputs > 0);
};

class VariableEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	VariableEditorNode(EditorNodeType nodeType, PinType outputType) :
		EvaluationEditorNode("Get " + ToString(outputType), nodeType)
	{
		AddPin(EditorNodePin::CreateOutputPin(ToString(outputType), outputType));
		m_NamePin = AddPin(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
	}

	const EditorNodePin& GetNamePin() const { return GetPins()[m_NamePin]; }

private:
	unsigned m_NamePin;
};

template<EditorNodeType nodeType, PinType outputType>
class VariableEditorNodeT : public VariableEditorNode
{
public:
	VariableEditorNodeT() :
		VariableEditorNode(nodeType, outputType) {}

	// Check other inherited classes if changing this method
	EditorNode* Clone() const override
	{
		return new VariableEditorNodeT<nodeType, outputType>();
	}
};

enum class BinaryOperatorType
{
	Arithemtic,
	Logic,
	Comparison
};

class BinaryOperatorEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();

	static std::string GetDefaultOperation(BinaryOperatorType opType)
	{
		switch (opType)
		{
		case BinaryOperatorType::Arithemtic: return "+";
		case BinaryOperatorType::Logic:		return "AND";
		case BinaryOperatorType::Comparison: return "==";
		}
		return "";
	}

	static const std::vector<std::string> GetOperatorOptions(BinaryOperatorType opType)
	{
		static const std::vector<std::string> noOperators = {};
		static const std::vector<std::string> arithmeticOperators = { "+", "-", "*", "/" };
		static const std::vector<std::string> comparisonOperators = { "==", "!=", ">", "<", ">=", "<=" };
		static const std::vector<std::string> logicOperators = { "AND", "OR", "XOR" };

		const std::vector<std::string>* operators = nullptr;
		switch (opType)
		{
		case BinaryOperatorType::Arithemtic: return arithmeticOperators;
		case BinaryOperatorType::Logic: return logicOperators;
		case BinaryOperatorType::Comparison: return comparisonOperators;
		default:
			NOT_IMPLEMENTED;
		}
		return noOperators;
	}

public:
	BinaryOperatorEditorNode(EditorNodeType nodeType, BinaryOperatorType opType, PinType pinType, PinType outputPinType):
		EvaluationEditorNode(ToString(pinType) + " operator", nodeType),
		m_OperatorSelector("Binary operator " + std::to_string(GetID()), m_Op, GetOperatorOptions(opType)),
		m_Op(GetDefaultOperation(opType))
	{
		m_Apin = AddPin(EditorNodePin::CreateInputPin("A", pinType));
		m_Bpin = AddPin(EditorNodePin::CreateInputPin("B", pinType));
		AddPin(EditorNodePin::CreateOutputPin("Result", outputPinType));
	}

	const EditorNodePin& GetAPin() const { return GetPins()[m_Apin]; }
	const EditorNodePin& GetBPin() const { return GetPins()[m_Bpin]; }
	const std::string& GetOp() const { return m_Op; }

public:
	virtual void RenderPopups() override;

protected:
	virtual void RenderContent() override;

private:
	unsigned m_Apin;
	unsigned m_Bpin;

	EditorComboBox m_OperatorSelector;

protected: // Should be private
	std::string m_Op;
};

template<EditorNodeType nodeType, BinaryOperatorType opType, PinType pinType, PinType outputPinType>
class BinaryOperatorEditorNodeT : public BinaryOperatorEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	BinaryOperatorEditorNodeT() :
		BinaryOperatorEditorNode(nodeType, opType, pinType, outputPinType) {}

	EditorNode* Clone() const override
	{
		BinaryOperatorEditorNodeT* node = new BinaryOperatorEditorNodeT<nodeType, opType, pinType, outputPinType>();
		node->m_Op = m_Op;
		return node;
	}
};

class MatrixTransformEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	MatrixTransformEditorNode(const std::string& label, EditorNodeType nodeType, PinType pinType, bool hasLastTransform = true) :
		EvaluationEditorNode(label, nodeType)
	{
		if(hasLastTransform) m_LastTransformPin = AddPin(EditorNodePin::CreateInputPin("Last transform (optional)", pinType));
		AddPin(EditorNodePin::CreateOutputPin("", pinType));
	}

	const EditorNodePin& GetLastTransformPin() const { ASSERT(m_LastTransformPin != -1); return GetPins()[m_LastTransformPin]; }

private:
	unsigned m_LastTransformPin = -1;
};

class Float4x4RotationTransformEditorNode : public MatrixTransformEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	Float4x4RotationTransformEditorNode() :
		MatrixTransformEditorNode("Float4x4: Rotate", EditorNodeType::Transform_Rotate_Float4x4, PinType::Float4x4)
	{
		m_AnglePin = AddPin(EditorNodePin::CreateInputPin("Angle", PinType::Float));
		m_AxisPin = AddPin(EditorNodePin::CreateInputPin("Axis", PinType::Float3));
	}

	EditorNode* Clone() const override
	{
		return new Float4x4RotationTransformEditorNode{};
	}

	const EditorNodePin& GetAnglePin() const { return GetPins()[m_AnglePin]; }
	const EditorNodePin& GetAxisPin() const { return GetPins()[m_AxisPin]; }

private:
	unsigned m_AnglePin;
	unsigned m_AxisPin;
};

class Float4x4TranslationTransformEditorNode : public MatrixTransformEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	Float4x4TranslationTransformEditorNode() :
		MatrixTransformEditorNode("Float4x4: Translate", EditorNodeType::Transform_Translate_Float4x4, PinType::Float4x4)
	{
		m_ValuePin = AddPin(EditorNodePin::CreateInputPin("Value", PinType::Float3));
	}

	EditorNode* Clone() const override
	{
		return new Float4x4TranslationTransformEditorNode{};
	}

	const EditorNodePin& GetValuePin() const { return GetPins()[m_ValuePin]; }

private:
	unsigned m_ValuePin;
};

class Float4x4ScaleTransformEditorNode : public MatrixTransformEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	Float4x4ScaleTransformEditorNode() :
		MatrixTransformEditorNode("Float4x4: Scale", EditorNodeType::Transform_Scale_Float4x4, PinType::Float4x4)
	{
		m_ValuePin = AddPin(EditorNodePin::CreateInputPin("Value", PinType::Float3));
	}

	EditorNode* Clone() const override
	{
		return new Float4x4ScaleTransformEditorNode{};
	}

	const EditorNodePin& GetValuePin() const { return GetPins()[m_ValuePin]; }

private:
	unsigned m_ValuePin;
};

class Float4x4LookAtTransformEditorNode : public MatrixTransformEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	Float4x4LookAtTransformEditorNode() :
		MatrixTransformEditorNode("Float4x4: Look at", EditorNodeType::Transform_LookAt_Float4x4, PinType::Float4x4, false)
	{
		m_EyePin = AddPin(EditorNodePin::CreateInputPin("Eye", PinType::Float3));
		m_CenterPin = AddPin(EditorNodePin::CreateInputPin("Center", PinType::Float3));
		m_UpPin = AddPin(EditorNodePin::CreateInputPin("Up", PinType::Float3));
	}

	EditorNode* Clone() const override
	{
		return new Float4x4LookAtTransformEditorNode{};
	}

	const EditorNodePin& GetEyePin() const { return GetPins()[m_EyePin]; }
	const EditorNodePin& GetCenterPin() const { return GetPins()[m_CenterPin]; }
	const EditorNodePin& GetUpPin() const { return GetPins()[m_UpPin]; }

private:
	unsigned m_EyePin;
	unsigned m_CenterPin;
	unsigned m_UpPin;
};

class Float4x4PerspectiveTransformEditorNode : public MatrixTransformEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	Float4x4PerspectiveTransformEditorNode() :
		MatrixTransformEditorNode("Float4x4: Perspective", EditorNodeType::Transform_PerspectiveProjection_Float4x4, PinType::Float4x4, false)
	{
		m_FOVPin = AddPin(EditorNodePin::CreateConstantInputPin("FOV", PinType::Float));
		m_AspectRatioPin = AddPin(EditorNodePin::CreateConstantInputPin("Aspect ratio", PinType::Float));
		m_ZNearPin = AddPin(EditorNodePin::CreateConstantInputPin("Z-Near", PinType::Float));
		m_ZFarPin = AddPin(EditorNodePin::CreateConstantInputPin("Z-Far", PinType::Float));
	}

	EditorNode* Clone() const override
	{
		return new Float4x4PerspectiveTransformEditorNode{};
	}

	const EditorNodePin& GetFOVPin() const { return GetPins()[m_FOVPin]; }
	const EditorNodePin& GetAspectRatioPin() const { return GetPins()[m_AspectRatioPin]; }
	const EditorNodePin& GetZNearPin() const { return GetPins()[m_ZNearPin]; }
	const EditorNodePin& GetZFarPin() const { return GetPins()[m_ZFarPin]; }

private:
	unsigned m_FOVPin;
	unsigned m_AspectRatioPin;
	unsigned m_ZNearPin;
	unsigned m_ZFarPin;
};

using VarBoolEditorNode = VariableEditorNodeT<EditorNodeType::VarBool, PinType::Bool>;
using BoolBinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::BoolBinaryOperator, BinaryOperatorType::Logic, PinType::Bool, PinType::Bool>;

using VarIntEditorNode = VariableEditorNodeT<EditorNodeType::VarInt, PinType::Int>;
using IntBinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::IntBinaryOperator, BinaryOperatorType::Arithemtic, PinType::Int, PinType::Int>;
using IntComparisonOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::IntComparisonOperator, BinaryOperatorType::Comparison, PinType::Int, PinType::Bool>;

using FloatEditorNode = FloatNEditorNodeT<EditorNodeType::Float, PinType::Float, 1>;
using VarFloatEditorNode = VariableEditorNodeT<EditorNodeType::VarFloat, PinType::Float>;
using FloatBinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::FloatBinaryOperator, BinaryOperatorType::Arithemtic, PinType::Float, PinType::Float>;
using FloatComparisonOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::FloatComparisonOperator, BinaryOperatorType::Comparison, PinType::Float, PinType::Bool>;

using Float2EditorNode = FloatNEditorNodeT<EditorNodeType::Float2, PinType::Float2, 2>;
using CreateFloat2EditorNode = CreateVectorEditorNodeT<EditorNodeType::CreateFloat2, PinType::Float, PinType::Float2, 2>;
using SplitFloat2EditorNode = SplitVectorEditorNodeT<EditorNodeType::SplitFloat2, PinType::Float2, PinType::Float, 2>;
using VarFloat2EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat2, PinType::Float2>;
using Float2BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float2BinaryOperator, BinaryOperatorType::Arithemtic, PinType::Float2, PinType::Float2>;

using Float3EditorNode = FloatNEditorNodeT<EditorNodeType::Float3, PinType::Float3, 3>;
using CreateFloat3EditorNode = CreateVectorEditorNodeT<EditorNodeType::CreateFloat3, PinType::Float, PinType::Float3, 3>;
using SplitFloat3EditorNode = SplitVectorEditorNodeT<EditorNodeType::SplitFloat3, PinType::Float3, PinType::Float, 3>;
using VarFloat3EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat3, PinType::Float3>;
using Float3BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float3BinaryOperator, BinaryOperatorType::Arithemtic, PinType::Float3, PinType::Float3>;

using Float4EditorNode = FloatNEditorNodeT<EditorNodeType::Float4, PinType::Float4, 4>;
using CreateFloat4EditorNode = CreateVectorEditorNodeT<EditorNodeType::CreateFloat4, PinType::Float, PinType::Float4, 4>;
using SplitFloat4EditorNode = SplitVectorEditorNodeT<EditorNodeType::SplitFloat4, PinType::Float4, PinType::Float, 4>;
using VarFloat4EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat4, PinType::Float4>;
using Float4BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float4BinaryOperator, BinaryOperatorType::Arithemtic, PinType::Float4, PinType::Float4>;

using Float4x4EditorNode = FloatNxNEditorNodeT<EditorNodeType::Float4x4, PinType::Float4x4, 4, 4>;
using VarFloat4x4EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat4x4, PinType::Float4x4>;
using Float4x4BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float4x4BinaryOperator, BinaryOperatorType::Arithemtic, PinType::Float4x4, PinType::Float4x4>;

using GetTextureEditorNode = VariableEditorNodeT<EditorNodeType::GetTexture, PinType::Texture>;
using GetShaderEditorNode = VariableEditorNodeT<EditorNodeType::GetShader, PinType::Shader>;
using GetSceneEditorNode = VariableEditorNodeT<EditorNodeType::GetScene, PinType::Scene>;

class GetMeshEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();

public:
	GetMeshEditorNode():
		EvaluationEditorNode("Get mesh", EditorNodeType::GetMesh)
	{
		m_SceneObjectPin = AddPin(EditorNodePin::CreateInputPin("Scene object", PinType::SceneObject));
		AddPin(EditorNodePin::CreateOutputPin("Mesh", PinType::Mesh));
	}

	const EditorNodePin& GetSceneObjectPin() const { return GetPins()[m_SceneObjectPin]; }

	bool GetPositionBit() const { return m_PositionBit; }
	bool GetTexcoordBit() const { return m_TexcoordBit; }
	bool GetNormalBit() const { return m_NormalBit; }
	bool GetTangentBit() const { return m_TangentBit; }

	EditorNode* Clone() const override
	{
		GetMeshEditorNode* node = new GetMeshEditorNode();
		node->m_PositionBit = m_PositionBit;
		node->m_TexcoordBit = m_TexcoordBit;
		node->m_NormalBit = m_NormalBit;
		node->m_TangentBit = m_TangentBit;
		return node;
	}

protected:
	void RenderContent() override;

private:
	unsigned m_SceneObjectPin;

	bool m_PositionBit, m_TexcoordBit, m_NormalBit, m_TangentBit;
};

class GetCubeMeshEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	GetCubeMeshEditorNode() :
		EvaluationEditorNode("Get cube mesh", EditorNodeType::GetCubeMesh)
	{
		AddPin(EditorNodePin::CreateOutputPin("", PinType::Mesh));
	}

	EditorNode* Clone() const override
	{
		return new GetCubeMeshEditorNode();
	}

protected:
	void RenderContent() override;
};

class BindTableEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	BindTableEditorNode():
		EvaluationEditorNode("Bind table", EditorNodeType::BindTable),
		m_TypeSelection("Bind table type selector " + std::to_string(GetID()), m_TypeValue, { "Texture", "Float", "Float2", "Float3", "Float4", "Float4x4" })

	{
		AddPin(EditorNodePin::CreateOutputPin("Table", PinType::BindTable));
	}

	EditorNode* Clone() const override
	{
		BindTableEditorNode* node = new BindTableEditorNode();
		for (const EditorNodePin& customPin : GetCustomPins())
		{
			ASSERT(customPin.IsInput);
			node->AddCustomPin(EditorNodePin::CreateInputPin(customPin.Label, customPin.Type));
		}
		return node;
	}

	virtual void RenderPopups() override;

protected:
	virtual void RenderContent() override;

private:
	EditorComboBox m_TypeSelection;
	std::string m_TypeValue = "Texture";
	
	std::string m_InputName;
	int m_InputInt = 0;
};

class RenderStateEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	RenderStateEditorNode() :
		EvaluationEditorNode("Render state", EditorNodeType::RenderState),
		m_DepthTestModeCB("Depth test mode cb " + std::to_string(GetID()), m_DepthTestMode, { "Always", "Never", "Less", "Equal", "LessEqual", "Greater", "GreaterEqual", "NotEqual" })
	{
		AddPin(EditorNodePin::CreateOutputPin("Render State", PinType::RenderState));
	}

	EditorNode* Clone() const override
	{
		RenderStateEditorNode* node = new RenderStateEditorNode{};
		node->m_DepthWrite = m_DepthWrite;
		node->m_DepthTestMode = m_DepthTestMode;
		return node;
	}

	virtual void RenderPopups() override;

	bool IsDepthWrite() const { return m_DepthWrite; }
	const std::string& GetDepthTestMode() const { return m_DepthTestMode; }

protected:
	virtual void RenderContent() override;

private:
	bool m_DepthWrite = false;

	EditorComboBox m_DepthTestModeCB;
	std::string m_DepthTestMode = "Always";
};
