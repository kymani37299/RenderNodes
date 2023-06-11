#include "EditorNode.h"

#include "../Common.h"
#include "RenderPipelineEditor.h"

static void DrawPin(const EditorNodePin& pin)
{
    ImNode::BeginPin(pin.ID, pin.IsInput ? ImNode::PinKind::Input : ImNode::PinKind::Output);
    ImGui::Text(pin.Label.c_str());
    ImNode::EndPin();
}

EditorNodePin EditorNodePin::CreateInputPin(const std::string& label, PinType type)
{
    EditorNodePin pin;
    pin.IsInput = true;
    pin.Type = type;
    pin.ID = EditorPrivate::GenerateID();
    pin.Label = label;
    return pin;
}

EditorNodePin EditorNodePin::CreateOutputPin(const std::string& label, PinType type)
{
    EditorNodePin pin;
    pin.IsInput = false;
    pin.Type = type;
    pin.ID = EditorPrivate::GenerateID();
    pin.Label = label;
    return pin;
}

EditorNode::EditorNode(const std::string& label, EditorNodeType nodeType) :
    m_Label(label),
    m_ID(EditorPrivate::GenerateID()),
    m_Type(nodeType)
{ }

void EditorNode::Render()
{
    ImGui::PushID(m_ID);

    ImNode::BeginNode(m_ID);
    ImGui::Text(m_Label.c_str());

    for (const auto pin : m_Executions)
    {
        DrawPin(pin);
    }

    for (const auto pin : m_Inputs)
    {
        DrawPin(pin);
    }

    for (const auto pin : m_Outputs)
    {
        DrawPin(pin);
    }

    RenderContent();

    ImNode::EndNode();

    ImGui::PopID();
}

PinID EditorNode::AddInputPin(const EditorNodePin& pin)
{
    m_Inputs.push_back(pin);
    return pin.ID;
}

PinID EditorNode::AddOutputPin(const EditorNodePin& pin)
{
    m_Outputs.push_back(pin);
    return pin.ID;
}

PinID EditorNode::AddExecutionPin(const EditorNodePin& pin)
{
    m_Executions.push_back(pin);
    return pin.ID;
}

void BoolEditorNode::RenderContent()
{
    ImGui::Checkbox("", &m_Value);
}

void FloatEditorNode::RenderContent()
{
    ImGui::SetNextItemWidth(50.0f);
    ImGui::DragFloat("Value", &m_Value);
}

void RenderBinaryOperatorNode(char& op)
{
    std::string opStr{ op };

    static const std::vector<std::string> operators = { "+", "-", "*", "/" };

    ImGui::SetNextItemWidth(50.0f);
    if (ImGui::BeginCombo("Op", opStr.c_str()))
    {
        for (const std::string& opSelection : operators)
        {
            bool isSelected = false;
            ImGui::Selectable(opSelection.c_str(), &isSelected);
            if (isSelected)
            {
                op = opSelection[0];
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}