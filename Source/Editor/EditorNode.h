#pragma once

#include "../Common.h"

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

    Bool,
    Float,
    FloatBinaryOperator,
    If,
    Print,
    OnUpdate,
    OnStart,
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
};

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

class FloatEditorNode : public EvaluationEditorNode
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    FloatEditorNode():
        EvaluationEditorNode("Float node", EditorNodeType::Float)
    {
        AddOutputPin(EditorNodePin::CreateOutputPin("    ->", PinType::Float));
    }

    float GetValue() const { return m_Value; }

protected:
    virtual void RenderContent() override;

private:
    float m_Value = 0.0f;
};

void RenderBinaryOperatorNode(char& op);

template<typename T>
class BinaryOperatorEditorNode : public EvaluationEditorNode
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    BinaryOperatorEditorNode(const std::string& labelPrefix, PinType pinType, EditorNodeType nodeType) :
        EvaluationEditorNode(labelPrefix + " Binary operator node", nodeType)
    {
        m_Apin = AddInputPin(EditorNodePin::CreateInputPin("A", pinType));
        m_Bpin = AddInputPin(EditorNodePin::CreateInputPin("B", pinType));
        AddOutputPin(EditorNodePin::CreateOutputPin("   Result", pinType));
    }

    PinID GetAPin() const { return m_Apin; }
    PinID GetBPin() const { return m_Bpin; }
    char GetOp() const { return m_Op; }

protected:
    virtual void RenderContent() override
    {
        RenderBinaryOperatorNode(m_Op);
    }

private:
    PinID m_Apin;
    PinID m_Bpin;

    char m_Op = '+';
};

class FloatBinaryOperatorEditorNode : public BinaryOperatorEditorNode<float> 
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    FloatBinaryOperatorEditorNode():
        BinaryOperatorEditorNode<float>("Float", PinType::Float, EditorNodeType::FloatBinaryOperator) {}
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
        ExecutionEditorNode("On update", EditorNodeType::OnUpdate, true) {}
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
    }

    PinID GetFloatInputPin() const { return m_FloatInputPin; }

private:
    PinID m_FloatInputPin;
};