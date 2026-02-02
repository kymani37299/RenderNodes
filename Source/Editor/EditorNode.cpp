#include "EditorNode.h"
#include "ExecutorEditorNode.h"
#include "EvaluationEditorNode.h"

#include "../App/App.h"
#include "../Common.h"
#include "../Util/FileDialog.h"
#include "Drawing/EditorWidgets.h"
#include "RenderPipelineEditor.h"

// Returns if we need to update pin
static void DrawPin(EditorNodePin pin, EditorNode* node)
{
    ImGui::PushID(pin.ID);
    ImNode::BeginPin(pin.ID, pin.IsInput ? ImNode::PinKind::Input : ImNode::PinKind::Output);
    if (!pin.HasConstantValue)
    {
		ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)GetPinColor(pin.Type));
		ImGui::Text(pin.Type == PinType::Execution ? ">>" : "->");
		ImGui::PopStyleColor();
    }
    else
    {
        bool needsPinUpdate = false;

        switch (pin.Type)
        {
        case PinType::Bool:
            needsPinUpdate = ImGui::Checkbox("", &pin.ConstantValue.B);
            break;
        case PinType::Int:
            ImGui::SetNextItemWidth(ImGui::ConstantSize(50.0f));
            needsPinUpdate = ImGui::DragInt("", &pin.ConstantValue.I);
            break;
        case PinType::Float:
            ImGui::SetNextItemWidth(ImGui::ConstantSize(50.0f));
            needsPinUpdate = ImGui::DragFloat("", &pin.ConstantValue.F);
            break;
        case PinType::Float2:
            ImGui::SetNextItemWidth(ImGui::ConstantSize(75.0f));
            needsPinUpdate = ImGui::DragFloat2("", pin.ConstantValue.F2);
            break;
        case PinType::Float3:
			ImGui::SetNextItemWidth(ImGui::ConstantSize(100.0f));
			needsPinUpdate = ImGui::DragFloat3("", pin.ConstantValue.F3);
			break;
        case PinType::Float4:
			ImGui::SetNextItemWidth(ImGui::ConstantSize(125.0f));
			needsPinUpdate = ImGui::DragFloat4("", pin.ConstantValue.F4);
		    break; 
        case PinType::String:
            ImGui::SetNextItemWidth(ImGui::ConstantSize(150.0f));
            needsPinUpdate = ImGui::InputText("", pin.ConstantValue.STR);
            break;
        default:
            NOT_IMPLEMENTED;
        }

        if (needsPinUpdate) node->UpdatePin(pin);
    }
    ImNode::EndPin();
    ImGui::PopID();
}

static void DrawPinLabel(const EditorNodePin& pin)
{
    ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)GetPinColor(pin.Type));
    ImGui::Text(pin.Label.c_str());
    ImGui::PopStyleColor();
}

EditorNodePin EditorNodePin::CreateConstantInputPin(const std::string& label, PinType type)
{
	EditorNodePin pin = CreateInputPin(label, type);
	pin.HasConstantValue = true;
	pin.ConstantValue.SetDefaultValue(type);
    return pin;
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

bool EditorNodePin::CanBeLinked(const EditorNodePin& a, const EditorNodePin& b)
{
    return (a.ID != b.ID) && (a.IsInput != b.IsInput) && (a.Type == b.Type || a.Type == PinType::Any || b.Type == PinType::Any);
}

std::unordered_map<std::type_index, EditorNode*> EditorNode::s_ClassRepresents;

EditorNode::EditorNode(const std::string& label, EditorNodeType nodeType) :
    m_Label(label),
    m_ID(IDGen::Generate()),
    m_Type(nodeType)
{ }

void EditorNode::UpdatePin(const EditorNodePin& newPin)
{
    for (auto& pin : m_Pins)
    {
        if (pin.ID == newPin.ID)
        {
            pin = newPin;
            return;
        }
    }

    for (auto& pin : m_CustomPins)
    {
        if (pin.ID == newPin.ID)
        {
            pin = newPin;
            return;
        }
    }
    ASSERT_M(0, "Pin to update not found!");
}

void EditorNode::RemovePin(PinID pinID)
{
    // Note: Links related to this pin also need to be handled when using this function

	for (unsigned i = 0; i < m_Pins.size(); i++)
	{
		const auto& pin = m_Pins[i];
		if (pin.ID == pinID)
		{
			m_Pins.erase(m_Pins.begin() + i);
			return;
		}
	}

	for (unsigned i = 0; i < m_CustomPins.size(); i++)
	{
		const auto& pin = m_CustomPins[i];
		if (pin.ID == pinID)
		{
			m_CustomPins.erase(m_CustomPins.begin() + i);
			return;
		}
	}

	ASSERT(0);
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
        for (const auto& pin : m_Pins) if (pin.Type == PinType::Execution &&  pin.IsInput) DrawPin(pin, this);
        for (const auto& pin : m_CustomPins) if (pin.Type == PinType::Execution &&  pin.IsInput) DrawPin(pin, this);
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
		for (const auto& pin : m_Pins) if (pin.Type == PinType::Execution && !pin.IsInput) DrawPin(pin, this);
		for (const auto& pin : m_CustomPins) if (pin.Type == PinType::Execution && !pin.IsInput) DrawPin(pin, this);
        ImGui::EndVertical();

        ImGui::EndHorizontal();
    }

	ImGui::Dummy({ ImGui::ConstantSize(5), ImGui::ConstantSize(5) });

    {
        ImGui::BeginHorizontal("Node body");

        ImGui::Spring(0);

        ImGui::BeginVertical("Input pins");
		for (const auto& pin : m_Pins) if (pin.Type != PinType::Execution && pin.IsInput) DrawPin(pin, this);
		for (const auto& pin : m_CustomPins) if (pin.Type != PinType::Execution && pin.IsInput) DrawPin(pin, this);
        ImGui::EndVertical();

		ImGui::BeginVertical("Input labels");
		for (const auto& pin : m_Pins) if (pin.Type != PinType::Execution && pin.IsInput) DrawPinLabel(pin);
		for (const auto& pin : m_CustomPins) if (pin.Type != PinType::Execution && pin.IsInput) DrawPinLabel(pin);
		ImGui::EndVertical();

        ImGui::Spring(1);

		ImGui::BeginVertical("Output labels");
		for (const auto& pin : m_Pins) if (pin.Type != PinType::Execution && !pin.IsInput) DrawPinLabel(pin);
		for (const auto& pin : m_CustomPins) if (pin.Type != PinType::Execution && !pin.IsInput) DrawPinLabel(pin);
		ImGui::EndVertical();

        ImGui::BeginVertical("Output pins");
        for (const auto& pin : m_Pins) if (pin.Type != PinType::Execution && !pin.IsInput) DrawPin(pin, this);
        for (const auto& pin : m_CustomPins) if (pin.Type != PinType::Execution && !pin.IsInput) DrawPin(pin, this);
        ImGui::EndVertical();

        ImGui::EndHorizontal();
    }

	RenderContent();

    ImGui::EndVertical();

    ImNode::EndNode();

    ImGui::PopID();
}

void EditorNode::RenderContent()
{
    ImGui::Dummy(ImVec2{ ImGui::ConstantSize(25), ImGui::ConstantSize(25) });
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

void IntEditorNode::RenderContent()
{
    ImGui::SetNextItemWidth(ImGui::ConstantSize(50.0f));
	ImGui::DragInt("", &m_Value);
}

void StringEditorNode::RenderContent()
{
    EditorWidgets::InputText("Value", m_Value);
}

void FloatNEditorNode::RenderContent()
{
    const std::string valueNames[] = { "X", "Y", "Z", "W" };

    for (unsigned i = 0; i < m_NumValues; i++)
    {
		ImGui::SetNextItemWidth(ImGui::ConstantSize(50.0f));
		ImGui::DragFloat(valueNames[i].c_str(), &m_Values[i]);
    }
}

void FloatNxNEditorNode::RenderContent()
{
    ImGui::BeginVertical("Matrix");
    for (unsigned i = 0; i < m_NumValuesX; i++)
    {
        ImGui::PushID(i);
        ImGui::BeginHorizontal("Matrix row");
		for (unsigned j = 0; j < m_NumValuesY; j++)
		{
            ImGui::PushID(j);
			ImGui::SetNextItemWidth(ImGui::ConstantSize(50.0f));
            ImGui::DragFloat("", &m_Values[i][j]);
            ImGui::PopID();
		}
        ImGui::EndHorizontal();
        ImGui::PopID();
    }
    ImGui::EndVertical();
}

void BinaryOperatorEditorNode::RenderContent()
{
    m_OperatorSelector.DrawBox();
}

void BinaryOperatorEditorNode::RenderPopups()
{
    m_OperatorSelector.DrawSelectionMenu();
}

void BindTableEditorNode::RenderContent()
{
    ImGui::Text("Binding name");

    if (m_TypeValue == "Texture")
    {
        ImGui::SetNextItemWidth(ImGui::ConstantSize(50.0f));
        ImGui::DragInt("", &m_InputInt, 1, 0, 31);
        m_InputName = std::to_string(m_InputInt);
    }
    else
    {
        EditorWidgets::InputText("", m_InputName);
    }
    
    ImGui::Dummy({ ImGui::ConstantSize(10.0f), ImGui::ConstantSize(10.0f) });

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

    m_TypeSelection.DrawBox();

    if (!canAdd)
    {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }

    EditorNodePin pinToAdd;
    if (ImGui::Button("Add binding"))
    {
        PinType pinType = PinType::Texture;

        if (m_TypeValue == "Texture") pinType = PinType::Texture;
        else if (m_TypeValue == "Float") pinType = PinType::Float;
        else if (m_TypeValue == "Float2") pinType = PinType::Float2;
        else if (m_TypeValue == "Float3") pinType = PinType::Float3;
        else if (m_TypeValue == "Float4") pinType = PinType::Float4;
        else if (m_TypeValue == "Float4x4") pinType = PinType::Float4x4;
        else NOT_IMPLEMENTED;

        AddCustomPin(EditorNodePin::CreateInputPin(m_InputName, pinType));

        m_InputInt = 0;
    }

	if (!canAdd)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}
}

void BindTableEditorNode::RenderPopups()
{
    m_TypeSelection.DrawSelectionMenu();
}

void GetMeshEditorNode::RenderContent()
{
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

void RenderStateEditorNode::RenderPopups()
{
    m_DepthTestModeCB.DrawSelectionMenu();
}

void RenderStateEditorNode::RenderContent()
{
    ImGui::Checkbox("Depth write", &m_DepthWrite);

    ImGui::Text("Depth test");
    ImGui::SameLine();
    
    m_DepthTestModeCB.DrawBox();
}

void PinEditorNode::RenderContent()
{
    EditorWidgets::InputText("Name", m_Name);
}

static std::string GetInputName(int key, int mods)
{
    std::string prefix = "";
    if (mods & GLFW_MOD_CONTROL) prefix += "Ctrl + ";
    if (mods & GLFW_MOD_ALT) prefix += "Alt + ";
    if (mods & GLFW_MOD_SHIFT) prefix += "Shift + ";
    if (mods & GLFW_MOD_SUPER) prefix += "Super + ";

    bool canBeCasted = key >= GLFW_KEY_A && key <= GLFW_KEY_Z;
    canBeCasted = canBeCasted || (key >= GLFW_KEY_0 && key <= GLFW_KEY_9);
    canBeCasted = canBeCasted || key == GLFW_KEY_APOSTROPHE;
    canBeCasted = canBeCasted || key == GLFW_KEY_COMMA;
    canBeCasted = canBeCasted || key == GLFW_KEY_MINUS;
    canBeCasted = canBeCasted || key == GLFW_KEY_PERIOD;
    canBeCasted = canBeCasted || key == GLFW_KEY_SLASH;
    canBeCasted = canBeCasted || key == GLFW_KEY_SEMICOLON;
    canBeCasted = canBeCasted || key == GLFW_KEY_EQUAL;
    canBeCasted = canBeCasted || key == GLFW_KEY_LEFT_BRACKET;
    canBeCasted = canBeCasted || key == GLFW_KEY_BACKSLASH;
    canBeCasted = canBeCasted || key == GLFW_KEY_RIGHT_BRACKET;
    canBeCasted = canBeCasted || key == GLFW_KEY_GRAVE_ACCENT;

    if (canBeCasted)
    {
        return prefix + std::string(1, key);
    }

    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25)
    {
        return prefix + "F" + std::to_string(key - GLFW_KEY_F1 + 1);
    }

    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9)
    {
        return prefix + "Numpad " + std::to_string(key - GLFW_KEY_KP_0);
    }

    switch (key)
    {
    case GLFW_KEY_SPACE: return prefix + "Space";
    case GLFW_KEY_ESCAPE             : return prefix + "Esc";
    case GLFW_KEY_ENTER              : return prefix + "Enter";
    case GLFW_KEY_TAB                : return prefix + "Tab";
    case GLFW_KEY_BACKSPACE          : return prefix + "Backspace";
    case GLFW_KEY_INSERT             : return prefix + "Insert";
    case GLFW_KEY_DELETE             : return prefix + "Delete";
    case GLFW_KEY_RIGHT              : return prefix + "Right";
    case GLFW_KEY_LEFT               : return prefix + "Left";
    case GLFW_KEY_DOWN               : return prefix + "Down";
    case GLFW_KEY_UP                 : return prefix + "Up";
    case GLFW_KEY_PAGE_UP            : return prefix + "PageUp";
    case GLFW_KEY_PAGE_DOWN          : return prefix + "PageDown";
    case GLFW_KEY_HOME               : return prefix + "Home";
    case GLFW_KEY_END                : return prefix + "End";
    case GLFW_KEY_CAPS_LOCK          : return prefix + "CapsLock";
    case GLFW_KEY_SCROLL_LOCK        : return prefix + "ScrollLock";
    case GLFW_KEY_NUM_LOCK           : return prefix + "NumLock";
    case GLFW_KEY_PRINT_SCREEN       : return prefix + "PrintSc";
    case GLFW_KEY_PAUSE              : return prefix + "Pause";
    case GLFW_KEY_KP_DECIMAL         : return prefix + "Numpad .";
    case GLFW_KEY_KP_DIVIDE          : return prefix + "Numpad /";
    case GLFW_KEY_KP_MULTIPLY        : return prefix + "Numpad *";
    case GLFW_KEY_KP_SUBTRACT        : return prefix + "Numpad -";
    case GLFW_KEY_KP_ADD             : return prefix + "Numpad +";
    case GLFW_KEY_KP_ENTER           : return prefix + "Numpad Enter";
    case GLFW_KEY_KP_EQUAL           : return prefix + "Numpad =";
    case GLFW_KEY_LEFT_SHIFT         : return prefix + "Left Shift";
    case GLFW_KEY_LEFT_CONTROL       : return prefix + "Left Ctrl";
    case GLFW_KEY_LEFT_ALT           : return prefix + "Left Alt";
    case GLFW_KEY_LEFT_SUPER         : return prefix + "Left Super";
    case GLFW_KEY_RIGHT_SHIFT        : return prefix + "Right Shift";
    case GLFW_KEY_RIGHT_CONTROL      : return prefix + "Right Ctrl";
    case GLFW_KEY_RIGHT_ALT          : return prefix + "Right Alt";
    case GLFW_KEY_RIGHT_SUPER        : return prefix + "Right Super";
    case GLFW_KEY_MENU               : return prefix + "Menu";
    }
    return "Unknown key";
}

void InputExecutionEditorNode::RenderContent()
{
	if (m_ListeningToInput)
	{
		ImGui::Text("Press any key...");
	}
	else
	{
		if (ImGui::Button("Set input"))
		{
			m_ListeningToInput = true;
			App::Get()->SubscribeToInput(this);
		}
	}

    if (m_Key == 0)
    {
        m_InputText = "No key assigned";
    }
    else
    {
        m_InputText = GetInputName(m_Key, m_Mods);
    }
    ImGui::Text(m_InputText.c_str());
}

void InputExecutionEditorNode::OnKeyReleased(int key, int mods)
{
	m_Key = key;
    m_Mods = mods;
	m_ListeningToInput = false;
	App::Get()->UnsubscribeToInput(this);
}

CustomEditorNode::CustomEditorNode(NodeGraph* parentGraph, const std::string& name, NodeGraph* nodeGraph, bool regneratePins) :
	ExecutionEditorNode(name, EditorNodeType::Custom, true, true),
    m_Name(name),
	m_NodeGraph(nodeGraph),
    m_ParentGraph(parentGraph)
{
    if (regneratePins)
    {
        RegeneratePins();
    }
}

void CustomEditorNode::RegeneratePins()
{
    // Detect pins to add
    std::vector<PinEditorNode*> pinsToAdd{};
    const auto fn = [this, &pinsToAdd](EditorNode* node)
    {
        if (node->GetType() == EditorNodeType::Pin)
        {
            bool hasPin = false;
            PinEditorNode* pinNode = static_cast<PinEditorNode*>(node);
            for (const auto& pin : GetCustomPins())
            {
                if (pin.LinkedNode == pinNode->GetID())
                {
                    hasPin = true;
                    break;
                }
            }

            if (!hasPin)
            {
                pinsToAdd.push_back(pinNode);
            }
        }
    };
    m_NodeGraph->ForEachNode(fn);

    // Detect pins to delete
    std::vector <PinID> toDelete{};
    for (const auto& pin : GetCustomPins())
	{
        if (!m_NodeGraph->ContainsNode(pin.LinkedNode))
        {
            toDelete.push_back(pin.ID);
        }
	}

    // Delete pins
    for (const auto& pinID : toDelete)
    {
        if (m_ParentGraph)
            m_ParentGraph->RemovePin(pinID);
        else
            RemovePin(pinID);
    }

    // Add new pins
    for (const auto pinNode : pinsToAdd)
    {
        const EditorNodePin& pin = pinNode->GetPin();

		EditorNodePin pinToAdd;
		if (pin.IsInput) pinToAdd = EditorNodePin::CreateOutputPin(pinNode->GetName(), pin.Type);
		else pinToAdd = EditorNodePin::CreateInputPin(pinNode->GetName(), pin.Type);

        pinToAdd.LinkedNode = pinNode->GetID();
		AddCustomPin(pinToAdd);
    }
}

std::vector<EditorNodePin> DeprecatedEditorNode::GetDeprecatedPins(EditorNodeType nodeType)
{
	std::vector<EditorNodePin> pins{};
	switch (nodeType)
	{
    case EditorNodeType::Deprecated:
        break;
	case EditorNodeType::DEPRECATED_VarFloat:
		pins.push_back(EditorNodePin::CreateOutputPin("Float", PinType::Float));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_VarFloat2:
		pins.push_back(EditorNodePin::CreateOutputPin("Float2", PinType::Float2));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_VarFloat3:
		pins.push_back(EditorNodePin::CreateOutputPin("Float3", PinType::Float3));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_VarFloat4:
		pins.push_back(EditorNodePin::CreateOutputPin("Float4", PinType::Float4));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_VarFloat4x4:
		pins.push_back(EditorNodePin::CreateOutputPin("Float4x4", PinType::Float4x4));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_VarBool:
		pins.push_back(EditorNodePin::CreateOutputPin("Bool", PinType::Bool));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_VarInt:
		pins.push_back(EditorNodePin::CreateOutputPin("Int", PinType::Int));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_GetScene:
		pins.push_back(EditorNodePin::CreateOutputPin("Scene", PinType::Scene));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_GetTexture:
		pins.push_back(EditorNodePin::CreateOutputPin("Texture", PinType::Texture));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
	case EditorNodeType::DEPRECATED_GetShader:
		pins.push_back(EditorNodePin::CreateOutputPin("Shader", PinType::Shader));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		break;
    case EditorNodeType::DEPRECATED_LoadScene:
        pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
        pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
        pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
	case EditorNodeType::DEPRECATED_LoadTexture:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
        pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
	case EditorNodeType::DEPRECATED_LoadShader:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
        pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
    case EditorNodeType::DEPRECATED_AsignBool:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateInputPin("Value", PinType::Bool));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
	case EditorNodeType::DEPRECATED_AsignInt:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateInputPin("Value", PinType::Int));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
	case EditorNodeType::DEPRECATED_AsignFloat:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateInputPin("Value", PinType::Float));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
	case EditorNodeType::DEPRECATED_AsignFloat2:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateInputPin("Value", PinType::Float2));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
	case EditorNodeType::DEPRECATED_AsignFloat3:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateInputPin("Value", PinType::Float3));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
	case EditorNodeType::DEPRECATED_AsignFloat4:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateInputPin("Value", PinType::Float4));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
	case EditorNodeType::DEPRECATED_AsignFloat4x4:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateInputPin("Value", PinType::Float4x4));
		pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        break;
    case EditorNodeType::DEPRECATED_CreateTexture:
		pins.push_back(EditorNodePin::CreateInputPin("", PinType::Execution));
		pins.push_back(EditorNodePin::CreateOutputPin("", PinType::Execution));
        pins.push_back(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
        pins.push_back(EditorNodePin::CreateConstantInputPin("Width", PinType::Int));
        pins.push_back(EditorNodePin::CreateConstantInputPin("Height", PinType::Int));
        pins.push_back(EditorNodePin::CreateConstantInputPin("Framebuffer", PinType::Bool));
        pins.push_back(EditorNodePin::CreateConstantInputPin("DepthStencil", PinType::Bool));
        break;
	default:
		NOT_IMPLEMENTED;
	}
	return pins;
}
