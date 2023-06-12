#pragma once

#include "../Common.h"
#include "../IDGen.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <inttypes.h>

// DO NOT CHANGE ORDER OF VALUES
// IT WILL AFFECT HOW WE LOAD OLD SAVE FILES
// ALWAYS APPEND ON END
enum class EditorNodeType
{
    Invalid,

	OnUpdate,
	OnStart,
    Bool,
    Float,
	Float2,
	Float3,
	Float4,
    AsignFloat,
    AsignFloat2,
    AsignFloat3,
    AsignFloat4,
    VarFloat,
    VarFloat2,
    VarFloat3,
    VarFloat4,
	FloatBinaryOperator,
	Float2BinaryOperator,
	Float3BinaryOperator,
	Float4BinaryOperator,
	If,
	Print,
    ClearRenderTarget,
};

// DO NOT CHANGE ORDER OF VALUES
// IT WILL AFFECT HOW WE LOAD OLD SAVE FILES
// ALWAYS APPEND ON END
enum class PinType
{
    Invalid,

    Execution,
    Bool,
    Float,
    Float2,
    Float3,
    Float4,
};

static const std::string ToString(PinType pinType)
{
    switch (pinType)
    {
    case PinType::Invalid: return "Invalid";
    case PinType::Execution: return "Execution";
    case PinType::Bool: return "Bool";
    case PinType::Float: return "Float";
    case PinType::Float2: return "Float2";
    case PinType::Float3: return "Float3";
    case PinType::Float4: return "Float4";
    default: NOT_IMPLEMENTED;
    }
    return "<unknown>";
}

struct EditorNodePin
{
    bool IsInput = false;
    PinType Type = PinType::Invalid;
    PinID ID = 0;
    std::string Label = "";

    static EditorNodePin CreateInputPin(const std::string& label, PinType type);
    static EditorNodePin CreateOutputPin(const std::string& label, PinType type);
};

struct EditorNodeLink
{
    LinkID ID;
    PinID Start;
    PinID End;
};

class NodeGraphSerializer;
#define SERIALIZEABLE_EDITOR_NODE() friend class NodeGraphSerializer

class EditorNode
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    EditorNode(const std::string& label, EditorNodeType nodeType);

    NodeID GetID() const { return m_ID; }
    EditorNodeType GetType() const { return m_Type; }

    void Render();

    template<typename Fn>
    void ForEachPin(Fn& fn)
    {
        for (const auto& pin : m_Executions) fn(pin);
        for (const auto& pin : m_Inputs) fn(pin);
        for (const auto& pin : m_Outputs) fn(pin);
    }

protected:
    virtual void RenderContent() {}

    PinID AddInputPin(const EditorNodePin& pin);
    PinID AddOutputPin(const EditorNodePin& pin);
    PinID AddExecutionPin(const EditorNodePin& pin);

private:
    std::vector<EditorNodePin> m_Inputs;
    std::vector<EditorNodePin> m_Outputs;
    std::vector<EditorNodePin> m_Executions;

    std::string m_Label;
    EditorNodeType m_Type;
    NodeID m_ID = 0;
};

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
        EvaluationEditorNode("Bool node", EditorNodeType::Bool)
    {
        AddOutputPin(EditorNodePin::CreateOutputPin("    ->", PinType::Bool));
    }

    bool GetValue() const { return m_Value; }

protected:
    virtual void RenderContent() override;

private:
    bool m_Value = false;
};

class ExecutionEditorNode : public EditorNode
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    ExecutionEditorNode(const std::string& label, EditorNodeType nodeType, bool skipInput = false):
        EditorNode(label, nodeType)
    {
        if (!skipInput) m_ExectuionPinInput = AddExecutionPin(EditorNodePin::CreateInputPin("-> EXECUTION", PinType::Execution));
        m_ExectuionPinOutput = AddExecutionPin(EditorNodePin::CreateOutputPin("EXECUTION ->", PinType::Execution));
    }

    PinID GetExecutionInput() const { return m_ExectuionPinInput; }
    PinID GetExecutionOutput() const {  return m_ExectuionPinOutput; }

private:
    PinID m_ExectuionPinInput;
    PinID m_ExectuionPinOutput;
};

class OnStartEditorNode : public ExecutionEditorNode
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    OnStartEditorNode() :
        ExecutionEditorNode("On start", EditorNodeType::OnStart, true) {}
};

class OnUpdateEditorNode : public ExecutionEditorNode
{
public:
    OnUpdateEditorNode() :
        ExecutionEditorNode("On update", EditorNodeType::OnUpdate, true) 
    {
        AddOutputPin(EditorNodePin::CreateOutputPin("DT ->", PinType::Float));
    }
};

class IfEditorNode : public ExecutionEditorNode
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    IfEditorNode():
        ExecutionEditorNode("If node", EditorNodeType::If) 
    {
        m_ExectuionPinElse = AddExecutionPin(EditorNodePin::CreateOutputPin("ELSE    ->", PinType::Execution));
        m_ConditionPin = AddInputPin(EditorNodePin::CreateInputPin("Condition", PinType::Bool));
    }

    PinID GetExecutionElse() const { return m_ExectuionPinElse; }
    PinID GetConditionPin() const { return m_ConditionPin; }

private:
    PinID m_ExectuionPinElse;
    PinID m_ConditionPin;
};

class PrintEditorNode : public ExecutionEditorNode
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    PrintEditorNode():
        ExecutionEditorNode("Print node", EditorNodeType::Print)
    {
        m_FloatInputPin = AddInputPin(EditorNodePin::CreateInputPin("<- Float", PinType::Float));
        m_Float2InputPin = AddInputPin(EditorNodePin::CreateInputPin("<- Float2", PinType::Float2));
        m_Float3InputPin = AddInputPin(EditorNodePin::CreateInputPin("<- Float3", PinType::Float3));
        m_Float4InputPin = AddInputPin(EditorNodePin::CreateInputPin("<- Float4", PinType::Float4));
    }

    PinID GetFloatInputPin() const { return m_FloatInputPin; }
    PinID GetFloat2InputPin() const { return m_Float2InputPin; }
    PinID GetFloat3InputPin() const { return m_Float3InputPin; }
    PinID GetFloat4InputPin() const { return m_Float4InputPin; }

private:
    PinID m_FloatInputPin;
    PinID m_Float2InputPin;
    PinID m_Float3InputPin;
    PinID m_Float4InputPin;
};

class ClearRenderTargetEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
    ClearRenderTargetEditorNode():
        ExecutionEditorNode("Clear render target", EditorNodeType::ClearRenderTarget)
    {
        m_ClearColorPin = AddInputPin(EditorNodePin::CreateInputPin("Clear color", PinType::Float4));
    }

    PinID GetClearColorPin() const { return m_ClearColorPin; }

private:
    PinID m_ClearColorPin;
};

class FloatNEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	FloatNEditorNode(unsigned numFloats, EditorNodeType nodeType, PinType pinType) :
		EvaluationEditorNode("Float" + (numFloats == 1 ? "" : std::to_string(numFloats)) + " node", nodeType),
        m_NumValues(numFloats)
	{
        m_Pin = AddOutputPin(EditorNodePin::CreateOutputPin("    ->", pinType));

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
	PinID m_Pin;
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

class AsignVariableEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	AsignVariableEditorNode(EditorNodeType nodeType, PinType inputType) :
        ExecutionEditorNode("Asign " + ToString(inputType) + " node", nodeType)
	{
		m_ValuePin = AddInputPin(EditorNodePin::CreateInputPin("Value", inputType));
	}

	PinID GetValuePin() const { return m_ValuePin; }
	const std::string& GetName() const { return m_Name; }

protected:
    virtual void RenderContent() override;

private:
	PinID m_ValuePin;
	std::string m_Name;
};

template<EditorNodeType nodeType, PinType pinType>
class AsignVariableEditorNodeT : public AsignVariableEditorNode
{
public:
    AsignVariableEditorNodeT() :
        AsignVariableEditorNode(nodeType, pinType) {}
};

class VariableEditorNode : public EvaluationEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	VariableEditorNode(EditorNodeType nodeType, PinType outputType) :
		EvaluationEditorNode("Var " + ToString(outputType) + " node", nodeType)
	{
		AddOutputPin(EditorNodePin::CreateOutputPin("    ->", outputType));
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
		m_Apin = AddInputPin(EditorNodePin::CreateInputPin("A", pinType));
		m_Bpin = AddInputPin(EditorNodePin::CreateInputPin("B", pinType));
		AddOutputPin(EditorNodePin::CreateOutputPin("   Result", pinType));
	}

	PinID GetAPin() const { return m_Apin; }
	PinID GetBPin() const { return m_Bpin; }
	char GetOp() const { return m_Op; }

protected:
	virtual void RenderContent() override;

private:
	PinID m_Apin;
	PinID m_Bpin;

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
using AsignFloatEditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat, PinType::Float>;
using FloatBinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::FloatBinaryOperator, PinType::Float>;

using Float2EditorNode = FloatNEditorNodeT<EditorNodeType::Float2, PinType::Float2, 2>;
using VarFloat2EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat2, PinType::Float2>;
using AsignFloat2EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat, PinType::Float2>;
using Float2BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float2BinaryOperator, PinType::Float2>;

using Float3EditorNode = FloatNEditorNodeT<EditorNodeType::Float3, PinType::Float3, 3>;
using VarFloat3EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat3, PinType::Float3>;
using AsignFloat3EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat3, PinType::Float3>;
using Float3BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float3BinaryOperator, PinType::Float3>;

using Float4EditorNode = FloatNEditorNodeT<EditorNodeType::Float4, PinType::Float4, 4>;
using VarFloat4EditorNode = VariableEditorNodeT<EditorNodeType::VarFloat4, PinType::Float4>;
using AsignFloat4EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat4, PinType::Float4>;
using Float4BinaryOperatorEditorNode = BinaryOperatorEditorNodeT<EditorNodeType::Float4BinaryOperator, PinType::Float4>;