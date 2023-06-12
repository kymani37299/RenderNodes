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

    const auto renderLink = [](const EditorNodeLink& link) {
        ImNode::Link(link.ID, link.Start, link.End);
    };
    m_NodeGraph->ForEachLink(renderLink);
}

void RenderPipelineEditor::RenderContextMenus()
{
    ImNode::Suspend();

    if (ImGui::BeginPopup("New Node Context Menu"))
    {
        EditorNode* newNode = nullptr;

        if (ImGui::BeginMenu("Constants"))
		{
			if (ImGui::MenuItem("Bool")) newNode = new BoolEditorNode();
			if (ImGui::MenuItem("Float")) newNode = new FloatEditorNode();
			if (ImGui::MenuItem("Float2")) newNode = new Float2EditorNode();
			if (ImGui::MenuItem("Float3")) newNode = new Float3EditorNode();
			if (ImGui::MenuItem("Float4")) newNode = new Float4EditorNode();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Assign variable"))
        {
            if (ImGui::MenuItem("Float")) newNode = new AsignFloatEditorNode();
            if (ImGui::MenuItem("Float2")) newNode = new AsignFloat2EditorNode();
            if (ImGui::MenuItem("Float3")) newNode = new AsignFloat3EditorNode();
            if (ImGui::MenuItem("Float4")) newNode = new AsignFloat4EditorNode();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Get variable"))
		{
			if (ImGui::MenuItem("Float")) newNode = new VarFloatEditorNode();
			if (ImGui::MenuItem("Float2")) newNode = new VarFloat2EditorNode();
			if (ImGui::MenuItem("Float3")) newNode = new VarFloat3EditorNode();
			if (ImGui::MenuItem("Float4")) newNode = new VarFloat4EditorNode();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Operator"))
        {
            if (ImGui::MenuItem("Float")) newNode = new FloatBinaryOperatorEditorNode();
            if (ImGui::MenuItem("Float2")) newNode = new Float2BinaryOperatorEditorNode();
            if (ImGui::MenuItem("Float3")) newNode = new Float3BinaryOperatorEditorNode();
            if (ImGui::MenuItem("Float4")) newNode = new Float4BinaryOperatorEditorNode();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render"))
        {
            if (ImGui::MenuItem("Clear render target")) newNode = new ClearRenderTargetEditorNode();
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("If condition")) newNode = new IfEditorNode();
        if (ImGui::MenuItem("Print")) newNode = new PrintEditorNode();
       
        if (newNode)
        {
            if (m_NewNodePinID)
            {
                EditorNodePin nodePin = m_NodeGraph->GetPinByID(m_NewNodePinID);
                unsigned pinCounts = 0;
                EditorNodePin targetPin;
                const auto fn = [&pinCounts, &nodePin, &targetPin, this](const EditorNodePin& pin)
                {
                    if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
                    {
                        targetPin = pin;
                        pinCounts++;
                    }
                };
                newNode->ForEachPin(fn);
                if (pinCounts == 1)
                {
                    const PinID startPin = nodePin.IsInput ? targetPin.ID : nodePin.ID;
                    const PinID endPin = nodePin.IsInput ? nodePin.ID : targetPin.ID;
                    m_NodeGraph->AddLink({ IDGen::Generate(), startPin, endPin });
                }
            }

            m_NodeGraph->AddNode(newNode);
            ImNode::SetNodePosition(newNode->GetID(), ImNode::ScreenToCanvas(ImGui::GetMousePos()));
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