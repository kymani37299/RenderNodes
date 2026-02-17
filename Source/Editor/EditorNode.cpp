#include "EditorNode.h"
#include "ExecutorEditorNode.h"
#include "EvaluationEditorNode.h"

#include "../App/App.h"
#include "../Common.h"
#include "../Util/FileDialog.h"
#include "../Util/GLFWUtils.h"
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

    ExecutionEditorNode* exNode = dynamic_cast<ExecutionEditorNode*>(this);
    const bool isEnabled = exNode == nullptr ? true : exNode->IsEnabled();

    if(!isEnabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

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

    if(!isEnabled) ImGui::PopStyleVar();

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
        m_InputText = GLFWUtils::ToString(m_Key, m_Mods, GLFWUtils::StringRepresentation::UI);
    }
    ImGui::Text(m_InputText.c_str());
}

void InputExecutionEditorNode::OnKeyInputEvent(const KeyInput& input)
{
    if (input.Action == KeyInputAction::Released)
    {
		m_Key = input.Key;
		m_Mods = input.Mods;
		m_ListeningToInput = false;
		App::Get()->UnsubscribeToInput(this);
    }
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
