#include "EditorContextMenu.h"

#include "../../Common.h"
#include "../../NodeGraph/NodeGraph.h"

void EditorContextMenu::Draw()
{
	ImNode::Suspend();

	if (m_ShouldOpen)
	{
		ImGui::OpenPopup(m_ID.c_str());
		m_ShouldOpen = false;
	}

	if (ImGui::BeginPopup(m_ID.c_str()))
	{
		DrawContent();
		ImGui::EndPopup();
	}

	ImNode::Resume();
}

void EditorComboBox::DrawContent()
{
	for (const std::string& value : m_Values)
	{
		if (ImGui::MenuItem(value.c_str()))
		{
			m_ValueRef = value;
		}
	}
}

template<typename T>
static bool IsCompatible(const EditorNodePin& nodePin)
{
	if (nodePin.Type == PinType::Invalid)
		return true;

	EditorNode* node = EditorNode::GetClassRepresent<T>();
	EditorNodePin targetPin;
	for (const auto& pin : node->GetPins())
	{
		if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
		{
			targetPin = pin;
			break;
		}
	}
	return targetPin.Type != PinType::Invalid;
}
#define ADD_NODE(Text, NodeType)  if (IsCompatible<NodeType>(nodePin) && ImGui::MenuItem(Text)) newNode = new NodeType()

void NewNodeContextMenu::DrawContent()
{
	EditorNode* newNode = nullptr;

	EditorNodePin nodePin;
	if (m_DraggedPin) nodePin = m_NodeGraph.GetPinByID(m_DraggedPin);

	if (ImGui::BeginMenu("Constants"))
	{
		ADD_NODE("Bool", BoolEditorNode);
		ADD_NODE("Float", FloatEditorNode);
		ADD_NODE("Float2", Float2EditorNode);
		ADD_NODE("Float3", Float3EditorNode);
		ADD_NODE("Float4", Float4EditorNode);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Assign variable"))
	{
		ADD_NODE("Bool", AsignBoolEditorNode);
		ADD_NODE("Float", AsignFloatEditorNode);
		ADD_NODE("Float2", AsignFloat2EditorNode);
		ADD_NODE("Float3", AsignFloat3EditorNode);
		ADD_NODE("Float4", AsignFloat4EditorNode);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Get variable"))
	{
		ADD_NODE("Bool", VarBoolEditorNode);
		ADD_NODE("Float", VarFloatEditorNode);
		ADD_NODE("Float2", VarFloat2EditorNode);
		ADD_NODE("Float3", VarFloat3EditorNode);
		ADD_NODE("Float4", VarFloat4EditorNode);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Operator"))
	{
		ADD_NODE("Bool", BoolBinaryOperatorEditorNode);
		ADD_NODE("Float", FloatBinaryOperatorEditorNode);
		ADD_NODE("Float2", Float2BinaryOperatorEditorNode);
		ADD_NODE("Float3", Float3BinaryOperatorEditorNode);
		ADD_NODE("Float4", Float4BinaryOperatorEditorNode);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Compare"))
	{
		ADD_NODE("Float", FloatComparisonOperatorEditorNode);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Create"))
	{
		ADD_NODE("Texture", CreateTextureEditorNode);
		ADD_NODE("Float2", CreateFloat2EditorNode);
		ADD_NODE("Float3", CreateFloat3EditorNode);
		ADD_NODE("Float4", CreateFloat4EditorNode);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Split"))
	{
		ADD_NODE("Float2", SplitFloat2EditorNode);
		ADD_NODE("Float3", SplitFloat3EditorNode);
		ADD_NODE("Float4", SplitFloat4EditorNode);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Load"))
	{
		ADD_NODE("Texture", LoadTextureEditorNode);
		ADD_NODE("Shader", LoadShaderEditorNode);
		ADD_NODE("Mesh", LoadMeshEditorNode);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Get"))
	{
		ADD_NODE("Texture", GetTextureEditorNode);
		ADD_NODE("Shader", GetShaderEditorNode);
		ADD_NODE("Mesh", GetMeshEditorNode);
		ADD_NODE("Cube mesh", GetCubeMeshEditorNode);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Render"))
	{
		ADD_NODE("Clear framebuffer", ClearRenderTargetEditorNode);
		ADD_NODE("Present texture", PresentTextureEditorNode);
		ADD_NODE("Draw mesh", DrawMeshEditorNode);
		ImGui::EndMenu();
	}

	ADD_NODE("BindTable", BindTableEditorNode);
	ADD_NODE("If condition", IfEditorNode);
	ADD_NODE("Print", PrintEditorNode);

	if (newNode)
	{
		m_NodeGraph.AddNode(newNode);
		ImNode::SetNodePosition(newNode->GetID(), ImNode::ScreenToCanvas(ImGui::GetMousePos()));

		if (nodePin.Type != PinType::Invalid)
		{
			EditorNodePin targetPin;
			for (const auto& pin : newNode->GetPins())
			{
				if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
				{
					targetPin = pin;
					break;
				}
			}

			if (targetPin.Type != PinType::Invalid)
			{
				const PinID startPin = nodePin.IsInput ? targetPin.ID : nodePin.ID;
				const PinID endPin = nodePin.IsInput ? nodePin.ID : targetPin.ID;
				m_NodeGraph.AddLink({ IDGen::Generate(), startPin, endPin });
			}
		}
	}
}

void NodeContextMenu::DrawContent()
{
	ASSERT(m_NodeID);
	if (ImGui::MenuItem("Delete")) m_NodeGraph.RemoveNode(m_NodeID);
}

void LinkContextMenu::DrawContent()
{
	ASSERT(m_LinkID);
	if (ImGui::MenuItem("Delete")) m_NodeGraph.RemoveLink(m_LinkID);
}

void PinContextMenu::DrawContent()
{
	ASSERT(m_PinID);
	const bool isCustomPin = m_NodeGraph.IsCustomPin(m_PinID);
	if (!isCustomPin)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}
	if (ImGui::MenuItem("Delete")) m_NodeGraph.RemovePin(m_PinID);
	if (!isCustomPin)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}
}