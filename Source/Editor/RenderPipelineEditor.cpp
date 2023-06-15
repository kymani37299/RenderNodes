#include "RenderPipelineEditor.h"

#include <atomic>

#include "../Common.h"
#include "../IDGen.h"

RenderPipelineEditor::~RenderPipelineEditor()
{
    Unload();
}

void RenderPipelineEditor::InitializeDefaultNodePositions()
{
    ImNode::SetCurrentEditor(m_EditorContext);
    ImNode::SetNodePosition(m_NodeGraph->GetOnStartNode()->GetID(), ImVec2{ 100, 200 });
    ImNode::SetNodePosition(m_NodeGraph->GetOnUpdateNode()->GetID(), ImVec2{ 100, 600 });
    ImNode::SetCurrentEditor(nullptr);
}

void RenderPipelineEditor::Render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImNode::SetCurrentEditor(m_EditorContext);
    ImNode::Begin("Render pipeline editor", ImVec2(0.0, 0.0f));

    UpdateEditor();
    RenderEditor();

    RenderNodePopups();
    RenderContextMenus();

    ImNode::End();
    ImNode::SetCurrentEditor(nullptr);
}

void RenderPipelineEditor::Unload()
{
    ImNode::DestroyEditor(m_EditorContext);
}

void RenderPipelineEditor::Load(NodeGraph* nodeGraph)
{
    ImNode::Config config;
    config.SettingsFile = "NodeEditor.json";
    m_EditorContext = ImNode::CreateEditor(&config);

    m_NodeGraph = Ptr<NodeGraph>(nodeGraph);
}

void RenderPipelineEditor::UpdateEditor()
{
    ImNode::PinId startPinID, endPinID, pin;
    ImNode::LinkId link;
    ImNode::NodeId node;

    if (ImNode::BeginCreate(ImColor(255, 255, 255), 2.0f))
    {
        if (ImNode::QueryNewLink(&startPinID, &endPinID))
        {
            if (startPinID && endPinID)
            {
                auto startPin = m_NodeGraph->GetPinByID(startPinID.Get());
                auto endPin = m_NodeGraph->GetPinByID(endPinID.Get());

                // Make sure start is always output pin
                if (startPin.IsInput)
                {
                    const auto tmpPin = startPin;
                    startPin = endPin;
                    endPin = tmpPin;
                }

                bool validLink = startPin.ID != endPin.ID;
                validLink = validLink && startPin.IsInput != endPin.IsInput;
                validLink = validLink && startPin.Type == endPin.Type;

                if (!validLink)
                {
                    ImNode::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (ImNode::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                {
                    m_NodeGraph->AddLink(EditorNodeLink{ IDGen::Generate(), startPin.ID, endPin.ID });
                }
            }
        }

        if (ImNode::QueryNewNode(&pin))
        {
            if (ImNode::AcceptNewItem())
            {
                m_NewNodePinID = pin.Get();

                ImNode::Suspend();
                ImGui::OpenPopup("New Node Context Menu");
                ImNode::Resume();
            }
        }
    }
    ImNode::EndCreate();

    if (ImNode::BeginDelete())
    {
        while (ImNode::QueryDeletedLink(&link))
        {
            if (ImNode::AcceptDeletedItem())
            {
                m_NodeGraph->RemoveLink(link.Get());
            }
        }

        while (ImNode::QueryDeletedNode(&node))
        {
            if (node.Get() == m_NodeGraph->GetOnStartNode()->GetID() || 
                node.Get() == m_NodeGraph->GetOnUpdateNode()->GetID())
            {
                ImNode::RejectDeletedItem();
            }
            else if (ImNode::AcceptDeletedItem())
            {
                m_NodeGraph->RemoveNode(node.Get());
            }
        }
    }
    ImNode::EndDelete();

    ImNode::Suspend();
    if (ImNode::ShowNodeContextMenu(&m_ContextNodeID)) ImGui::OpenPopup("Node Context Menu");
    else if (ImNode::ShowPinContextMenu(&m_ContextPinID)) ImGui::OpenPopup("Pin Context Menu");
    else if (ImNode::ShowLinkContextMenu(&m_ContextLinkID)) ImGui::OpenPopup("Link Context Menu");
    else if (ImNode::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("New Node Context Menu");
        m_ContextLinkID = 0;
    }
    ImNode::Resume();
}

void RenderPipelineEditor::RenderEditor()
{
    const auto renderNode = [](EditorNode* node) {
        node->Render();
    };
    m_NodeGraph->ForEachNode(renderNode);

    const auto renderLink = [this](const EditorNodeLink& link) {
        const auto& outputPin = m_NodeGraph->GetPinByID(link.Start);
        const bool isExecution = outputPin.Type == PinType::Execution;
        ImNode::Link(link.ID, link.Start, link.End, GetPinColor(outputPin.Type), isExecution ? 3.0f : 1.0f);
    };
    m_NodeGraph->ForEachLink(renderLink);
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

void RenderPipelineEditor::RenderContextMenus()
{
    ImNode::Suspend();

    if (ImGui::BeginPopup("New Node Context Menu"))
    {
        EditorNode* newNode = nullptr;

        EditorNodePin nodePin;
        if (m_NewNodePinID) 
            nodePin = m_NodeGraph->GetPinByID(m_NewNodePinID);

#define ADD_NODE(Text, NodeType)  if (IsCompatible<NodeType>(nodePin) && ImGui::MenuItem(Text)) newNode = new NodeType()

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
            m_NodeGraph->AddNode(newNode);
            ImNode::SetNodePosition(newNode->GetID(), ImNode::ScreenToCanvas(ImGui::GetMousePos()));

            if (m_NewNodePinID)
            {
                const auto nodePin = m_NodeGraph->GetPinByID(m_NewNodePinID);
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
                    m_NodeGraph->AddLink({ IDGen::Generate(), startPin, endPin });
                }
            }

			// TODO: This is also not good solution, we will keep the node context if the user cancels the new node operation
			m_NewNodePinID = 0;
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Node Context Menu"))
    {
        ImGui::Text("Node context - TODO");
        ImGui::EndPopup();
    }

	if (ImGui::BeginPopup("Pin Context Menu"))
	{
		ImGui::Text("Pin context - TODO");
        ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Link Context Menu"))
	{
		ImGui::Text("Link context - TODO");
        ImGui::EndPopup();
	}

    ImNode::Resume();
}

void RenderPipelineEditor::RenderNodePopups()
{
    ImNode::Suspend();

	const auto renderNode = [](EditorNode* node) {
		node->RenderPopups();
	};
	m_NodeGraph->ForEachNode(renderNode);

    ImNode::Resume();
}
