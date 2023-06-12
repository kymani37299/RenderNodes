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
    pin.ID = IDGen::Generate();
    pin.Label = label;
    return pin;
}

EditorNodePin EditorNodePin::CreateOutputPin(const std::string& label, PinType type)
{
    EditorNodePin pin;
    pin.IsInput = false;
    pin.Type = type;
    pin.ID = IDGen::Generate();
    pin.Label = label;
    return pin;
}

EditorNode::EditorNode(const std::string& label, EditorNodeType nodeType) :
    m_Label(label),
    m_ID(IDGen::Generate()),
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

void FloatNEditorNode::RenderContent()
{
    const std::string valueNames[] = { "X", "Y", "Z", "W" };

    for (unsigned i = 0; i < m_NumValues; i++)
    {
		ImGui::SetNextItemWidth(50.0f);
		ImGui::DragFloat(valueNames[i].c_str(), &m_Values[i]);
    }
}

void ImGuiInputText(const char* label, std::string& text)
{
	constexpr unsigned BUF_SIZE = 50;
	char buf[BUF_SIZE];
	strcpy_s(buf, text.c_str());

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::InputText(label, buf, BUF_SIZE))
	{
		text = std::string{ buf };
	}
}

void BinaryOperatorEditorNode::RenderContent()
{
    std::string opStr{ m_Op };

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
                m_Op = opSelection[0];
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void AsignVariableEditorNode::RenderContent()
{
    ImGuiInputText("Name", m_Name);
}

void VariableEditorNode::RenderContent()
{
    ImGuiInputText("Name", m_VariableName);
}
