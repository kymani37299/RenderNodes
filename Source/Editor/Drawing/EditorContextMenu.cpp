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
		if (EditorNodePin::CanBeLinked(pin, nodePin))
		{
			targetPin = pin;
			break;
		}
	}

	for (const auto& pin : node->GetCustomPins())
	{
		if (EditorNodePin::CanBeLinked(pin, nodePin))
		{
			targetPin = pin;
			break;
		}
	}

	return targetPin.Type != PinType::Invalid;
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
		if (EditorNodePin::CanBeLinked(pin, nodePin))
		{
			targetPin = pin;
			break;
		}
	}

	for (const auto& pin : node->GetCustomPins())
	{
		if (EditorNodePin::CanBeLinked(pin, nodePin))
		{
			targetPin = pin;
			break;
		}
	}

	return targetPin.Type != PinType::Invalid;
}

template<typename T>
class NewNodeMenuItem : public EditorWidgets::MenuItem
{
public:
	NewNodeMenuItem(const std::string& label, EditorNode** newNodePtr) :
		EditorWidgets::MenuItem(label),
		m_NewNodePtr(newNodePtr) {}

	virtual void OnClick() const override
	{
		*m_NewNodePtr = new T{};
	}

private:
	EditorNode** m_NewNodePtr;
};

class NewCustomNodeMenuItem : public EditorWidgets::MenuItem
{
public:
	NewCustomNodeMenuItem(const std::string& label, EditorNode** newNodePtr, CustomEditorNode* customNode, NodeGraph* parentGraph):
		EditorWidgets::MenuItem(label),
		m_NewNodePtr(newNodePtr),
		m_CustomNode(customNode),
		m_ParentGraph(parentGraph)
	{}

	virtual void OnClick() const override
	{
		*m_NewNodePtr = m_CustomNode->Instance(m_ParentGraph);
	}

private:
	EditorNode** m_NewNodePtr;
	CustomEditorNode* m_CustomNode;
	NodeGraph* m_ParentGraph;
};

class NewPinNodeMenuItem : public EditorWidgets::MenuItem
{
public:
	NewPinNodeMenuItem(const std::string& label, EditorNode** newNodePtr, PinType pinType, bool isInput):
		EditorWidgets::MenuItem(label),
		m_NewNodePtr(newNodePtr),
		m_PinType(pinType),
		m_IsInput(isInput)
	{}

	virtual void OnClick() const override
	{
		*m_NewNodePtr = new PinEditorNode{ m_IsInput, m_PinType };
	}

private:
	EditorNode** m_NewNodePtr;
	PinType m_PinType;
	bool m_IsInput;
};

template<typename T>
void AddIfCompatible(EditorWidgets::Menu& menu, const std::string& label, EditorNode** newNode, const EditorNodePin& nodePin)
{
	if (IsCompatible<T>(nodePin))
		menu.AddItem(new NewNodeMenuItem<T>(label, newNode));
}

void NewNodeContextMenu::DrawContent()
{
	if (m_ShouldRebuildMenus)
	{
		RebuildMenus();
		m_ShouldRebuildMenus = false;
	}

	if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
		ImGui::SetKeyboardFocusHere(0);

	ImGui::InputText("Search", m_SearchFilter);

	m_NewNode = nullptr;

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

	m_CreationMenu.Render(m_SearchFilter);

	if (m_NewNode)
	{
		const auto nodePos = ImNode::ScreenToCanvas(ImGui::GetMousePos());
		m_CommandExecutor->ExecuteCommand(new AddNodeNodeGraphCommand{ m_NewNode, {nodePos.x, nodePos.y}, m_DraggedPin });
	}
}

void NewNodeContextMenu::RebuildMenus()
{
	EditorNodePin nodePin;
	if (m_DraggedPin) nodePin = m_CommandExecutor->GetNodeGraph()->GetPinByID(m_DraggedPin);

	m_CreationMenu = EditorWidgets::Menu{ "", true };

	if (m_CustomNodeEditor)
	{
		const auto pinsMenu = [this, &nodePin](bool isInput, EditorWidgets::Menu& pinMenu)
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

				pinMenu.AddItem(new NewPinNodeMenuItem{ ToString(pinType), &m_NewNode, pinType, isInput });
			}
		};

		if (nodePin.Type == PinType::Invalid || nodePin.IsInput)
		{
			EditorWidgets::Menu pinInputMenu{ "Input Pins" };
			pinsMenu(false, pinInputMenu);
			m_CreationMenu.AddMenu(pinInputMenu);
		}

		if (nodePin.Type == PinType::Invalid || !nodePin.IsInput)
		{
			EditorWidgets::Menu pinOutputMenu{ "Output Pins" };
			pinsMenu(true, pinOutputMenu);
			m_CreationMenu.AddMenu(pinOutputMenu);
		}
	}

	EditorWidgets::Menu constantsMenu{ "Constants" };
	AddIfCompatible<BoolEditorNode>(constantsMenu, "Bool", &m_NewNode, nodePin);
	AddIfCompatible<StringEditorNode>(constantsMenu, "String", &m_NewNode, nodePin);
	AddIfCompatible<FloatEditorNode>(constantsMenu, "Float", &m_NewNode, nodePin);
	AddIfCompatible<Float2EditorNode>(constantsMenu, "Float2", &m_NewNode, nodePin);
	AddIfCompatible<Float3EditorNode>(constantsMenu, "Float3", &m_NewNode, nodePin);
	AddIfCompatible<Float4EditorNode>(constantsMenu, "Float4", &m_NewNode, nodePin);
	AddIfCompatible<Float4x4EditorNode>(constantsMenu, "Float4x4", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(constantsMenu);

	EditorWidgets::Menu assignMenu{ "Assign variable" };
	AddIfCompatible<AsignBoolEditorNode>(assignMenu, "Bool", &m_NewNode, nodePin);
	AddIfCompatible<AsignIntEditorNode>(assignMenu, "Int", &m_NewNode, nodePin);
	AddIfCompatible<AsignFloatEditorNode>(assignMenu, "Float", &m_NewNode, nodePin);
	AddIfCompatible<AsignFloat2EditorNode>(assignMenu, "Float2", &m_NewNode, nodePin);
	AddIfCompatible<AsignFloat3EditorNode>(assignMenu, "Float3", &m_NewNode, nodePin);
	AddIfCompatible<AsignFloat4EditorNode>(assignMenu, "Float4", &m_NewNode, nodePin);
	AddIfCompatible<AsignFloat4x4EditorNode>(assignMenu, "Float4x4", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(assignMenu);

	EditorWidgets::Menu getVarMenu{ "Get variable" };
	AddIfCompatible<VarBoolEditorNode>(getVarMenu, "Bool", &m_NewNode, nodePin);
	AddIfCompatible<VarIntEditorNode>(getVarMenu, "Int", &m_NewNode, nodePin);
	AddIfCompatible<VarFloatEditorNode>(getVarMenu, "Float", &m_NewNode, nodePin);
	AddIfCompatible<VarFloat2EditorNode>(getVarMenu, "Float2", &m_NewNode, nodePin);
	AddIfCompatible<VarFloat3EditorNode>(getVarMenu, "Float3", &m_NewNode, nodePin);
	AddIfCompatible<VarFloat4EditorNode>(getVarMenu, "Float4", &m_NewNode, nodePin);
	AddIfCompatible<VarFloat4x4EditorNode>(getVarMenu, "Float4x4", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(getVarMenu);

	EditorWidgets::Menu operatorMenu{ "Operator" };
	AddIfCompatible<BoolBinaryOperatorEditorNode>(operatorMenu, "Bool", &m_NewNode, nodePin);
	AddIfCompatible<IntBinaryOperatorEditorNode>(operatorMenu, "Int", &m_NewNode, nodePin);
	AddIfCompatible<FloatBinaryOperatorEditorNode>(operatorMenu, "Float", &m_NewNode, nodePin);
	AddIfCompatible<Float2BinaryOperatorEditorNode>(operatorMenu, "Float2", &m_NewNode, nodePin);
	AddIfCompatible<Float3BinaryOperatorEditorNode>(operatorMenu, "Float3", &m_NewNode, nodePin);
	AddIfCompatible<Float4BinaryOperatorEditorNode>(operatorMenu, "Float4", &m_NewNode, nodePin);
	AddIfCompatible<Float4x4BinaryOperatorEditorNode>(operatorMenu, "Float4x4", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(operatorMenu);

	EditorWidgets::Menu vectorsMenu{ "Vectors" };
	AddIfCompatible<NormalizeFloat2EditorNode>(vectorsMenu, "Normalize Float2", &m_NewNode, nodePin);
	AddIfCompatible<NormalizeFloat3EditorNode>(vectorsMenu, "Normalize Float3", &m_NewNode, nodePin);
	AddIfCompatible<NormalizeFloat4EditorNode>(vectorsMenu, "Normalize Float4", &m_NewNode, nodePin);
	AddIfCompatible<CrossProductOperationEditorNode>(vectorsMenu, "Cross product Float3", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(vectorsMenu);

	EditorWidgets::Menu transformMenu{ "Transform" };
	AddIfCompatible<Float4x4RotationTransformEditorNode>(transformMenu, "Float4x4: Rotate", &m_NewNode, nodePin);
	AddIfCompatible<Float4x4TranslationTransformEditorNode>(transformMenu, "Float4x4: Translate", &m_NewNode, nodePin);
	AddIfCompatible<Float4x4ScaleTransformEditorNode>(transformMenu, "Float4x4: Scale", &m_NewNode, nodePin);
	AddIfCompatible<Float4x4LookAtTransformEditorNode>(transformMenu, "Float4x4: Look at", &m_NewNode, nodePin);
	AddIfCompatible<Float4x4PerspectiveTransformEditorNode>(transformMenu, "Float4x4: Perspective projection", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(transformMenu);

	EditorWidgets::Menu compareMenu{ "Compare" };
	AddIfCompatible<FloatComparisonOperatorEditorNode>(compareMenu, "Float", &m_NewNode, nodePin);
	AddIfCompatible<IntComparisonOperatorEditorNode>(compareMenu, "Int", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(compareMenu);

	EditorWidgets::Menu createMenu{ "Create" };
	AddIfCompatible<CreateTextureEditorNode>(createMenu, "Texture", &m_NewNode, nodePin);
	AddIfCompatible<CreateFloat2EditorNode>(createMenu, "Float2", &m_NewNode, nodePin);
	AddIfCompatible<CreateFloat3EditorNode>(createMenu, "Float3", &m_NewNode, nodePin);
	AddIfCompatible<CreateFloat4EditorNode>(createMenu, "Float4", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(createMenu);

	EditorWidgets::Menu splitMenu{ "Split" };
	AddIfCompatible<SplitFloat2EditorNode>(splitMenu, "Float2", &m_NewNode, nodePin);
	AddIfCompatible<SplitFloat3EditorNode>(splitMenu, "Float3", &m_NewNode, nodePin);
	AddIfCompatible<SplitFloat4EditorNode>(splitMenu, "Float4", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(splitMenu);

	EditorWidgets::Menu loadMenu{ "Load" };
	AddIfCompatible<LoadTextureEditorNode>(loadMenu, "Texture", &m_NewNode, nodePin);
	AddIfCompatible<LoadShaderEditorNode>(loadMenu, "Shader", &m_NewNode, nodePin);
	AddIfCompatible<LoadSceneEditorNode>(loadMenu, "Scene object", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(loadMenu);

	EditorWidgets::Menu getMenu{ "Get" };
	AddIfCompatible<GetTextureEditorNode>(getMenu, "Texture", &m_NewNode, nodePin);
	AddIfCompatible<GetShaderEditorNode>(getMenu, "Shader", &m_NewNode, nodePin);
	AddIfCompatible<GetMeshEditorNode>(getMenu, "Mesh", &m_NewNode, nodePin);
	AddIfCompatible<GetCubeMeshEditorNode>(getMenu, "Cube mesh", &m_NewNode, nodePin);
	AddIfCompatible<GetSceneEditorNode>(getMenu, "Scene", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(getMenu);

	EditorWidgets::Menu renderMenu{ "Render" };
	AddIfCompatible<ClearRenderTargetEditorNode>(renderMenu, "Clear framebuffer", &m_NewNode, nodePin);
	AddIfCompatible<PresentTextureEditorNode>(renderMenu, "Present texture", &m_NewNode, nodePin);
	AddIfCompatible<DrawMeshEditorNode>(renderMenu, "Draw mesh", &m_NewNode, nodePin);
	m_CreationMenu.AddMenu(renderMenu);

	if (!m_CustomNodeEditor)
	{
		EditorWidgets::Menu inputMenu{ "Input" };
		AddIfCompatible<OnKeyPressedEditorNode>(inputMenu, "On key pressed", &m_NewNode, nodePin);
		AddIfCompatible<OnKeyReleasedEditorNode>(inputMenu, "On key released", &m_NewNode, nodePin);
		AddIfCompatible<OnKeyDownEditorNode>(inputMenu, "On key down", &m_NewNode, nodePin);
		m_CreationMenu.AddMenu(inputMenu);
	}

	AddIfCompatible<BindTableEditorNode>(m_CreationMenu, "BindTable", &m_NewNode, nodePin);
	AddIfCompatible<RenderStateEditorNode>(m_CreationMenu, "RenderState", &m_NewNode, nodePin);
	AddIfCompatible<IfEditorNode>(m_CreationMenu, "If condition", &m_NewNode, nodePin);
	AddIfCompatible<ForEachSceneObjectEditorNode>(m_CreationMenu, "For each scene object", &m_NewNode, nodePin);
	AddIfCompatible<PrintEditorNode>(m_CreationMenu, "Print", &m_NewNode, nodePin);

	const auto& customNodes = *App::Get()->GetCustomNodes();
	if (!customNodes.empty())
	{
		EditorWidgets::Menu customMenu{ "Custom" };

		for (const auto& customNode : customNodes)
		{
			if (IsCustomNodeCompatible(customNode.get(), nodePin))
				customMenu.AddItem(new NewCustomNodeMenuItem{ customNode->GetName(), &m_NewNode, customNode.get(), m_CommandExecutor->GetNodeGraph() });
		}
		m_CreationMenu.AddMenu(customMenu);
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
			App::Get()->OpenCustomNode(static_cast<CustomEditorNode*>(node)->GetName());
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