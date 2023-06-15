#include "EditorNode.h"
#include "ExecutorEditorNode.h"
#include "EvaluationEditorNode.h"

#include "../Common.h"
#include "../Util/FileDialog.h"
#include "RenderPipelineEditor.h"

#include <imgui_internal.h>

static void DrawPin(const EditorNodePin& pin)
{
    ImNode::BeginPin(pin.ID, pin.IsInput ? ImNode::PinKind::Input : ImNode::PinKind::Output);
    ImGui::PushStyleColor(ImGuiCol_Text, (ImU32) GetPinColor(pin.Type));
    ImGui::Text(pin.Type == PinType::Execution ? ">>" : "->");
    ImGui::PopStyleColor();
    ImNode::EndPin();
}

static void DrawPinLabel(const EditorNodePin& pin)
{
    ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)GetPinColor(pin.Type));
    ImGui::Text(pin.Label.c_str());
    ImGui::PopStyleColor();
}

static void ImGuiInputText(const char* label, std::string& text)
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

static void ImGuiComboBox(const std::string& id, std::string& value, bool& shown, const std::vector<std::string>& values)
{
	if (shown) ImGui::OpenPopup(id.c_str());

	if (ImGui::BeginPopup(id.c_str()))
	{
		for (const std::string& v : values)
		{
			if (ImGui::MenuItem(v.c_str()))
			{
                shown = false;
                value = v;
			}
		}
		ImGui::EndPopup();
	}
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

std::unordered_map<std::type_index, EditorNode*> EditorNode::s_ClassRepresents;

EditorNode::EditorNode(const std::string& label, EditorNodeType nodeType) :
    m_Label(label),
    m_ID(IDGen::Generate()),
    m_Type(nodeType)
{ }

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
        for (const auto& pin : m_Pins) if (pin.Type == PinType::Execution &&  pin.IsInput) DrawPin(pin);
        for (const auto& pin : m_CustomPins) if (pin.Type == PinType::Execution &&  pin.IsInput) DrawPin(pin);
        ImGui::EndVertical();

        ImGui::BeginVertical("Input execution labels");
        for (const auto& pin : m_Pins) if (pin.Type == PinType::Execution && pin.IsInput) DrawPinLabel(pin);
        for (const auto& pin : m_CustomPins) if (pin.Type == PinType::Execution && pin.IsInput) DrawPinLabel(pin);
        ImGui::EndVertical();

        ImGui::Spring(0.5f);

        ImGui::Text(m_Label.c_str());

        ImGui::Spring(1.0f);

		ImGui::BeginVertical("Output execution labels");
		for (const auto& pin : m_Pins) if (pin.Type == PinType::Execution && !pin.IsInput) DrawPinLabel(pin);
		for (const auto& pin : m_CustomPins) if (pin.Type == PinType::Execution && !pin.IsInput) DrawPinLabel(pin);
		ImGui::EndVertical();

        ImGui::BeginVertical("Output execution pins");
		for (const auto& pin : m_Pins) if (pin.Type == PinType::Execution && !pin.IsInput) DrawPin(pin);
		for (const auto& pin : m_CustomPins) if (pin.Type == PinType::Execution && !pin.IsInput) DrawPin(pin);
        ImGui::EndVertical();

        ImGui::EndHorizontal();
    }

	ImGui::Dummy({ 5, 5 });

    {
        ImGui::BeginHorizontal("Node body");

        ImGui::Spring(0);

        ImGui::BeginVertical("Input pins");
		for (const auto& pin : m_Pins) if (pin.Type != PinType::Execution && pin.IsInput) DrawPin(pin);
		for (const auto& pin : m_CustomPins) if (pin.Type != PinType::Execution && pin.IsInput) DrawPin(pin);
        ImGui::EndVertical();

		ImGui::BeginVertical("Input labels");
		for (const auto& pin : m_Pins) if (pin.Type != PinType::Execution && pin.IsInput) DrawPinLabel(pin);
		for (const auto& pin : m_CustomPins) if (pin.Type != PinType::Execution && pin.IsInput) DrawPinLabel(pin);
		ImGui::EndVertical();

        ImGui::BeginVertical("Render content");
        RenderContent();
        ImGui::EndVertical();

        ImGui::Spring(1);

		ImGui::BeginVertical("Output labels");
		for (const auto& pin : m_Pins) if (pin.Type != PinType::Execution && !pin.IsInput) DrawPinLabel(pin);
		for (const auto& pin : m_CustomPins) if (pin.Type != PinType::Execution && !pin.IsInput) DrawPinLabel(pin);
		ImGui::EndVertical();

        ImGui::BeginVertical("Output pins");
        for (const auto& pin : m_Pins) if (pin.Type != PinType::Execution && !pin.IsInput) DrawPin(pin);
        for (const auto& pin : m_CustomPins) if (pin.Type != PinType::Execution && !pin.IsInput) DrawPin(pin);
        ImGui::EndVertical();

        ImGui::EndHorizontal();
    }

    ImGui::EndVertical();

    ImNode::EndNode();

    ImGui::PopID();
}

void EditorNode::RenderContent()
{
    ImGui::Dummy(ImVec2{ 25, 25 });
}

unsigned EditorNode::AddCustomPin(const EditorNodePin& pin)
{
	m_CustomPins.push_back(pin);
	return m_CustomPins.size() - 1;
}

unsigned EditorNode::AddPin(const EditorNodePin& pin)
{
    m_Pins.push_back(pin);
    return m_Pins.size() - 1;
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

void BinaryOperatorEditorNode::RenderContent()
{
    if (ImGui::Button(m_Op.c_str()))
    {
        m_ShowSelectionBox = true;
    }
}

void BinaryOperatorEditorNode::RenderPopups()
{
	static const std::vector<std::string> arithmeticOperators = { "+", "-", "*", "/" };
    static const std::vector<std::string> comparisonOperators = { "==", "!=", ">", "<", ">=", "<=" };
    static const std::vector<std::string> logicOperators = { "AND", "OR", "XOR" };

    const std::vector<std::string>* operators = nullptr;
    switch (m_OpType)
    {
    case BinaryOperatorType::Arithemtic:
        operators = &arithmeticOperators;
        break;
    case BinaryOperatorType::Logic:
        operators = &logicOperators;
        break;
    case BinaryOperatorType::Comparison:
        operators = &comparisonOperators;
        break;
    default:
        operators = &arithmeticOperators;
        NOT_IMPLEMENTED;
    }
	ImGuiComboBox(m_SelectionBoxID, m_Op, m_ShowSelectionBox, *operators);
}

void AsignVariableEditorNode::RenderContent()
{
	ExecutionEditorNode::RenderContent();
    ImGuiInputText("Name", m_Name);
}

void VariableEditorNode::RenderContent()
{
    ImGuiInputText("", m_VariableName);
}

void CreateTextureEditorNode::RenderContent()
{
    ExecutionEditorNode::RenderContent();

    ImGuiInputText("Name", m_Name);

    ImGui::SetNextItemWidth(50.0f);
    ImGui::DragInt("Width", &m_Width, 1, 1);
    ImGui::SetNextItemWidth(50.0f);
    ImGui::DragInt("Height", &m_Height, 1, 1);
    ImGui::Checkbox("FrameBuffer", &m_Framebuffer);
}

void NameAndPathExecutionEditorNode::RenderContent()
{
	ExecutionEditorNode::RenderContent();
	ImGuiInputText("Name", m_Name);
	if (ImGui::Button("Select file"))
	{
		std::string texPath;
        bool fileOpened = false;
        switch (GetType())
        {
        case EditorNodeType::LoadTexture: fileOpened = FileDialog::OpenTextureFile(texPath); break;
        case EditorNodeType::LoadShader: fileOpened = FileDialog::OpenShaderFile(texPath); break;
        case EditorNodeType::LoadMesh: fileOpened = FileDialog::OpenSceneFile(texPath); break;
        default: NOT_IMPLEMENTED;
        }

		if (fileOpened)
		{
			m_Path = texPath;
		}
	}
	ImGui::Text(m_Path.c_str());
}

void BindTableEditorNode::RenderContent()
{
    ImGui::Text("Binding name");
    ImGuiInputText("", m_InputName);
    ImGui::Dummy({ 10.0f, 10.0f });

    bool canAdd = true;
    if (m_InputName.empty())
    {
        canAdd = false;
    }

    if (canAdd)
    {
        for (const auto& customPin : GetCustomPins())
        {
            if (customPin.Label == m_InputName)
            {
                canAdd = false;
                break;
            }
        }
    }

    if (!canAdd)
    {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }

    EditorNodePin pinToAdd;
    if (ImGui::Button("Add texture")) pinToAdd = EditorNodePin::CreateInputPin(m_InputName, PinType::Texture);
    // if (ImGui::Button("Add buffer")) pinToAdd = EditorNodePin::CreateInputPin(m_InputName, PinType::Buffer);

	if (!canAdd)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

    if (pinToAdd.Type != PinType::Invalid)
    {
        ASSERT(canAdd);
        m_InputName = "";
        AddCustomPin(pinToAdd);
    }
}

void GetMeshEditorNode::RenderContent()
{
    Super::RenderContent();

    ImGui::Checkbox("Positions", &m_PositionBit);
    ImGui::Checkbox("Texcoords", &m_TexcoordBit);
    ImGui::Checkbox("Normals", &m_NormalBit);
    ImGui::Checkbox("Tangents", &m_TangentBit);
}



void GetCubeMeshEditorNode::RenderContent()
{
    static bool b[4] = {true, false, false, false};

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

	ImGui::Checkbox("Positions", &b[0]);
	ImGui::Checkbox("Texcoords", &b[1]);
	ImGui::Checkbox("Normals", &b[2]);
	ImGui::Checkbox("Tangents", &b[3]);

	ImGui::PopItemFlag();
	ImGui::PopStyleVar();
}
