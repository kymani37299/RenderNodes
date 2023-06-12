#include "EditorNode.h"

#include "../Common.h"
#include "RenderPipelineEditor.h"

#include <imgui_internal.h>

static void DrawPin(const EditorNodePin& pin)
{
    ImNode::BeginPin(pin.ID, pin.IsInput ? ImNode::PinKind::Input : ImNode::PinKind::Output);
    ImGui::Text("->");
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

inline ImRect ImGui_GetItemRect()
{
	return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

void EditorNode::Render()
{
    ImGui::PushID(m_ID);

    ImNode::BeginNode(m_ID);

    ImGui::BeginVertical("Node");
    ImGui::Spring();
    {
        ImGui::BeginHorizontal("Node header");

        ImGui::Spring(0);

        ImGui::BeginVertical("Input execution pins");
        for (const auto& pin : m_Executions) if (pin.IsInput) DrawPin(pin);
        ImGui::EndVertical();

        ImGui::BeginVertical("Input execution labels");
        for (const auto& pin : m_Executions) if (pin.IsInput) ImGui::Text(pin.Label.c_str());
        ImGui::EndVertical();

        ImGui::Spring(0.5f);

        ImGui::Text(m_Label.c_str());

        ImGui::Spring(1.0f);

		ImGui::BeginVertical("Output execution labels");
		for (const auto& pin : m_Executions) if (!pin.IsInput) ImGui::Text(pin.Label.c_str());
		ImGui::EndVertical();

        ImGui::BeginVertical("Output execution pins");
		for (const auto& pin : m_Executions) if (!pin.IsInput) DrawPin(pin);
        ImGui::EndVertical();

        ImGui::EndHorizontal();
    }

	ImGui::Dummy({ 5, 5 });

    {
        ImGui::BeginHorizontal("Node body");

        ImGui::Spring(0);

        ImGui::BeginVertical("Input pins");
		for (const auto& pin : m_Inputs) DrawPin(pin);
        ImGui::EndVertical();

		ImGui::BeginVertical("Input labels");
		for (const auto& pin : m_Inputs) ImGui::Text(pin.Label.c_str());
		ImGui::EndVertical();

        ImGui::BeginVertical("Render content");
        RenderContent();
        ImGui::EndVertical();

        ImGui::Spring(1);

		ImGui::BeginVertical("Output labels");
		for (const auto& pin : m_Outputs) ImGui::Text(pin.Label.c_str());
		ImGui::EndVertical();

        ImGui::BeginVertical("Output pins");
        for (const auto& pin : m_Outputs) DrawPin(pin);
        ImGui::EndVertical();

        ImGui::EndHorizontal();
    }

    ImGui::EndVertical();

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

void EditorNode::RenderContent()
{
    ImGui::Dummy(ImVec2{ 25, 25 });
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
