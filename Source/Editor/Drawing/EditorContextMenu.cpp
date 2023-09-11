#include "EditorContextMenu.h"

#include "../../Common.h"
#include "../../App/App.h"
#include "../../NodeGraph/NodeGraph.h"
#include "../../NodeGraph/NodeGraphCommands.h"

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

void EditorComboBox::DrawBox()
{
	if (ImGui::Button(m_ValueRef.c_str()))
	{
		EditorContextMenu::Open();
	}
}

void EditorComboBox::DrawSelectionMenu()
{
	EditorContextMenu::Draw();
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

static bool IsCustomNodeCompatible(const CustomEditorNode* node, const EditorNodePin& nodePin)
{
	if (nodePin.Type == PinType::Invalid)
		return true;

	EditorNodePin targetPin;
	for (const auto& pin : node->GetPins())
	{
		if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
		{
			targetPin = pin;
			break;
		}
	}

	for (const auto& pin : node->GetCustomPins())
	{
		if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
		{
			targetPin = pin;
			break;
		}
	}

	return targetPin.Type != PinType::Invalid;
}

template<typename T>
static bool IsCompatible_Impl(const EditorNodePin& nodePin)
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

	for (const auto& pin : node->GetCustomPins())
	{
		if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
		{
			targetPin = pin;
			break;
		}
	}

	return targetPin.Type != PinType::Invalid;
}

// Variadic template function, working thanks to u/IyeOnline https://www.reddit.com/r/cpp_questions/comments/14c4cxw/variadic_template_function_question/
template<typename... Args>
static bool IsCompatible(const EditorNodePin& nodePin)
{
	return (IsCompatible_Impl<Args>(nodePin) || ...);
}

#define BEGIN_MENU(Text, ...) if(IsCompatible<__VA_ARGS__>(nodePin) && ImGui::BeginMenu(Text))
#define MENU_ITEM(Text, NodeType)  if (IsCompatible<NodeType>(nodePin) && ImGui::MenuItem(Text)) newNode = new NodeType()
#define END_MENU() ImGui::EndMenu()

void NewNodeContextMenu::DrawContent()
{
	EditorNode* newNode = nullptr;
	
	EditorNodePin nodePin;
	if (m_DraggedPin) nodePin = m_CommandExecutor->GetNodeGraph()->GetPinByID(m_DraggedPin);

	if (nodePin.Type == PinType::Invalid)
	{
		const auto& copiedNodes = m_CommandExecutor->GetContext().CopiedNodes;
		if (!copiedNodes.empty())
		{
			if (ImGui::MenuItem("Paste nodes"))
			{
				m_CommandExecutor->ExecuteCommand(new PasteNodesNodeGraphCommand{});
			}
			ImGui::Dummy({ ImGui::ConstantSize(10), ImGui::ConstantSize(10) });
		}
	}

	if (m_CustomNodeEditor)
	{
		const auto pinsMenu = [this, &newNode, &nodePin](bool isInput)
		{
			bool excludeExecutionPin = false;
			if (!isInput)
			{
				const auto fn = [&excludeExecutionPin](EditorNode* node)
				{
					if (node->GetType() == EditorNodeType::Pin)
					{
						PinEditorNode* pinNode = static_cast<PinEditorNode*>(node);
						if (!pinNode->GetPin().IsInput && pinNode->GetPin().Type == PinType::Execution)
							excludeExecutionPin = true;
					}
				};
				m_CommandExecutor->GetNodeGraph()->ForEachNode(fn);
			}

			for (unsigned i = 1; i < EnumToInt(PinType::Count); i++)
			{
				const PinType pinType = IntToEnum<PinType>(i);
				if (excludeExecutionPin && pinType == PinType::Execution)
					continue;

				if (nodePin.Type != PinType::Invalid && pinType != nodePin.Type)
					continue;

				const std::string menuLabel = ToString(pinType);
				if (ImGui::MenuItem(menuLabel.c_str())) newNode = new PinEditorNode{ isInput, pinType };
			}
		};

		if ((nodePin.Type == PinType::Invalid || nodePin.IsInput) && ImGui::BeginMenu("Input Pins"))
		{
			pinsMenu(false);
			ImGui::EndMenu();
		}

		if ((nodePin.Type == PinType::Invalid || !nodePin.IsInput) && ImGui::BeginMenu("Output Pins"))
		{
			pinsMenu(true);
			ImGui::EndMenu();
		}
	}

	BEGIN_MENU("Constants", BoolEditorNode, IntEditorNode, StringEditorNode, FloatEditorNode, Float2EditorNode, Float3EditorNode, Float4EditorNode, Float4x4EditorNode)
	{
		MENU_ITEM("Bool", BoolEditorNode);
		MENU_ITEM("Int", IntEditorNode);
		MENU_ITEM("String", StringEditorNode);
		MENU_ITEM("Float", FloatEditorNode);
		MENU_ITEM("Float2", Float2EditorNode);
		MENU_ITEM("Float3", Float3EditorNode);
		MENU_ITEM("Float4", Float4EditorNode);
		MENU_ITEM("Float4x4", Float4x4EditorNode);
		END_MENU();
	}

	BEGIN_MENU("Assign variable", AsignBoolEditorNode, AsignIntEditorNode, AsignFloatEditorNode, AsignFloat2EditorNode, AsignFloat3EditorNode, AsignFloat4EditorNode, AsignFloat4x4EditorNode)
	{
		MENU_ITEM("Bool", AsignBoolEditorNode);
		MENU_ITEM("Int", AsignIntEditorNode);
		MENU_ITEM("Float", AsignFloatEditorNode);
		MENU_ITEM("Float2", AsignFloat2EditorNode);
		MENU_ITEM("Float3", AsignFloat3EditorNode);
		MENU_ITEM("Float4", AsignFloat4EditorNode);
		MENU_ITEM("Float4x4", AsignFloat4x4EditorNode);
		END_MENU();
	}

	BEGIN_MENU("Get variable", VarBoolEditorNode, VarIntEditorNode, VarFloatEditorNode, VarFloat2EditorNode, VarFloat3EditorNode, VarFloat4EditorNode, VarFloat4x4EditorNode)
	{
		MENU_ITEM("Bool", VarBoolEditorNode);
		MENU_ITEM("Int", VarIntEditorNode);
		MENU_ITEM("Float", VarFloatEditorNode);
		MENU_ITEM("Float2", VarFloat2EditorNode);
		MENU_ITEM("Float3", VarFloat3EditorNode);
		MENU_ITEM("Float4", VarFloat4EditorNode);
		MENU_ITEM("Float4x4", VarFloat4x4EditorNode);
		END_MENU();
	}

	BEGIN_MENU("Operator", BoolBinaryOperatorEditorNode, IntBinaryOperatorEditorNode, FloatBinaryOperatorEditorNode, Float2BinaryOperatorEditorNode, Float3BinaryOperatorEditorNode, Float4BinaryOperatorEditorNode, Float4x4BinaryOperatorEditorNode)
	{
		MENU_ITEM("Bool", BoolBinaryOperatorEditorNode);
		MENU_ITEM("Int", IntBinaryOperatorEditorNode);
		MENU_ITEM("Float", FloatBinaryOperatorEditorNode);
		MENU_ITEM("Float2", Float2BinaryOperatorEditorNode);
		MENU_ITEM("Float3", Float3BinaryOperatorEditorNode);
		MENU_ITEM("Float4", Float4BinaryOperatorEditorNode);
		MENU_ITEM("Float4x4", Float4x4BinaryOperatorEditorNode);
		END_MENU();
	}

	BEGIN_MENU("Transform", Float4x4RotationTransformEditorNode, Float4x4TranslationTransformEditorNode, Float4x4ScaleTransformEditorNode, Float4x4LookAtTransformEditorNode, Float4x4PerspectiveTransformEditorNode)
	{
		MENU_ITEM("Float4x4: Rotate", Float4x4RotationTransformEditorNode);
		MENU_ITEM("Float4x4: Translate", Float4x4TranslationTransformEditorNode);
		MENU_ITEM("Float4x4: Scale", Float4x4ScaleTransformEditorNode);
		MENU_ITEM("Float4x4: Look at", Float4x4LookAtTransformEditorNode);
		MENU_ITEM("Float4x4: Perspective projection", Float4x4PerspectiveTransformEditorNode);
		END_MENU();
	}

	BEGIN_MENU("Compare", FloatComparisonOperatorEditorNode, IntComparisonOperatorEditorNode)
	{
		MENU_ITEM("Float", FloatComparisonOperatorEditorNode);
		MENU_ITEM("Int", IntComparisonOperatorEditorNode);
		END_MENU();
	}

	BEGIN_MENU("Create", CreateTextureEditorNode, CreateFloat2EditorNode, CreateFloat3EditorNode, CreateFloat4EditorNode)
	{
		MENU_ITEM("Texture", CreateTextureEditorNode);
		MENU_ITEM("Float2", CreateFloat2EditorNode);
		MENU_ITEM("Float3", CreateFloat3EditorNode);
		MENU_ITEM("Float4", CreateFloat4EditorNode);
		END_MENU();
	}

	BEGIN_MENU("Split", SplitFloat2EditorNode, SplitFloat3EditorNode, SplitFloat4EditorNode)
	{
		MENU_ITEM("Float2", SplitFloat2EditorNode);
		MENU_ITEM("Float3", SplitFloat3EditorNode);
		MENU_ITEM("Float4", SplitFloat4EditorNode);
		END_MENU();
	}

	BEGIN_MENU("Load", LoadTextureEditorNode, LoadShaderEditorNode, LoadMeshEditorNode)
	{
		MENU_ITEM("Texture", LoadTextureEditorNode);
		MENU_ITEM("Shader", LoadShaderEditorNode);
		MENU_ITEM("Mesh", LoadMeshEditorNode);
		END_MENU();
	}

	BEGIN_MENU("Get", GetTextureEditorNode, GetShaderEditorNode, GetMeshEditorNode, GetCubeMeshEditorNode)
	{
		MENU_ITEM("Texture", GetTextureEditorNode);
		MENU_ITEM("Shader", GetShaderEditorNode);
		MENU_ITEM("Mesh", GetMeshEditorNode);
		MENU_ITEM("Cube mesh", GetCubeMeshEditorNode);
		END_MENU();
	}

	BEGIN_MENU("Render", ClearRenderTargetEditorNode, PresentTextureEditorNode, DrawMeshEditorNode)
	{
		MENU_ITEM("Clear framebuffer", ClearRenderTargetEditorNode);
		MENU_ITEM("Present texture", PresentTextureEditorNode);
		MENU_ITEM("Draw mesh", DrawMeshEditorNode);
		END_MENU();
	}

	if (!m_CustomNodeEditor)
	{
		BEGIN_MENU("Input", OnKeyPressedEditorNode, OnKeyReleasedEditorNode, OnKeyDownEditorNode)
		{
			MENU_ITEM("On key pressed", OnKeyPressedEditorNode);
			MENU_ITEM("On key released", OnKeyReleasedEditorNode);
			MENU_ITEM("On key down", OnKeyDownEditorNode);
			END_MENU();
		}
	}
	
	MENU_ITEM("BindTable", BindTableEditorNode);
	MENU_ITEM("RenderState", RenderStateEditorNode);
	MENU_ITEM("If condition", IfEditorNode);
	MENU_ITEM("Print", PrintEditorNode);

	const auto& customNodes = *App::Get()->GetCustomNodes();
	if (!customNodes.empty())
	{
		if (ImGui::BeginMenu("Custom"))
		{
			for (const auto& customNode : customNodes)
			{
				if (IsCustomNodeCompatible(customNode.get(), nodePin) && ImGui::MenuItem(customNode->GetName().c_str())) newNode = customNode->Instance(m_CommandExecutor->GetNodeGraph());
			}

			ImGui::EndMenu();
		}
	}

	if (newNode)
	{
		const auto nodePos = ImNode::ScreenToCanvas(ImGui::GetMousePos());
		m_CommandExecutor->ExecuteCommand(new AddNodeNodeGraphCommand{ newNode, {nodePos.x, nodePos.y}, m_DraggedPin });
	}
}

void NodeContextMenu::DrawContent()
{
	ASSERT(m_NodeID);

	ImNode::SelectNode(m_NodeID, true);

	EditorNode* node = m_CommandExecutor->GetNodeGraph()->GetNodeByID(m_NodeID);
	if (node->GetType() == EditorNodeType::Custom)
	{
		if (ImGui::MenuItem("Open"))
		{
			App::Get()->OpenCustomNode(static_cast<CustomEditorNode*>(node));
		}
	}

	if (ImGui::MenuItem("Copy"))
	{
		m_CommandExecutor->ExecuteCommand(new CopySelectedNodesNodeGraphCommand{});
	}

	if (ImGui::MenuItem("Delete"))
	{
		m_CommandExecutor->ExecuteCommand(new RemoveSelectedNodesNodeGraphCommand{});
	}
}

void LinkContextMenu::DrawContent()
{
	ASSERT(m_LinkID);
	if (ImGui::MenuItem("Delete")) m_CommandExecutor->ExecuteCommand(new RemoveLinkNodeGraphCommand{ m_LinkID });
}

void PinContextMenu::DrawContent()
{
	ASSERT(m_PinID);
	const bool isCustomPin = m_CommandExecutor->GetNodeGraph()->IsCustomPin(m_PinID);
	if (isCustomPin && ImGui::MenuItem("Delete")) m_CommandExecutor->ExecuteCommand(new RemovePinNodeGraphCommand{ m_PinID });

	const auto& pin = m_CommandExecutor->GetNodeGraph()->GetPinByID(m_PinID);
	static PinType allowedConstantTypes[] = { PinType::Bool, PinType::Float, PinType::Float2, PinType::Float3, PinType::Float4, PinType::String, PinType::Int };
	bool canBeConstant = false;

	if (pin.IsInput)
	{
		for (uint32_t i = 0; i < STATIC_ARRAY_SIZE(allowedConstantTypes); i++)
		{
			if (pin.Type == allowedConstantTypes[i])
			{
				canBeConstant = true;
				break;
			}
		}
	}
	
	if (canBeConstant)
	{
		if (pin.HasConstantValue && ImGui::MenuItem("Make as pin")) m_CommandExecutor->ExecuteCommand(new MakeConstantToPinCommand{ m_PinID });
		if(!pin.HasConstantValue && ImGui::MenuItem("Make as constant")) m_CommandExecutor->ExecuteCommand(new MakePinConstantCommand{ m_PinID });
	}
}