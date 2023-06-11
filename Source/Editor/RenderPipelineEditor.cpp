#include "RenderPipelineEditor.h"

#include <atomic>

#include "../Common.h"
#include "../IDGen.h"

RenderPipelineEditor::~RenderPipelineEditor()
{
    ImNode::DestroyEditor(m_EditorContext);
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

void RenderPipelineEditor::Load(const std::string& settingsPath, NodeGraph* nodeGraph)
{
    ImNode::Config config;
    config.SettingsFile = settingsPath.c_str();
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
                ImNode::Suspend();
                ImGui::OpenPopup("Create New Node");
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

        if (ImGui::MenuItem("If node")) newNode = new IfEditorNode();
        if (ImGui::MenuItem("Bool node")) newNode = new BoolEditorNode();
        if (ImGui::MenuItem("Float node")) newNode = new FloatEditorNode();
        if (ImGui::MenuItem("Print node")) newNode = new PrintEditorNode();
        if (ImGui::MenuItem("Float binary operator node")) newNode = new FloatBinaryOperatorEditorNode();
        if (ImGui::MenuItem("Asign float")) newNode = new AsignFloatEditorNode();
        if (ImGui::MenuItem("Var float")) newNode = new VarFloatEditorNode();

        if (newNode)
        {
            m_NodeGraph->AddNode(newNode);
            ImNode::SetNodePosition(newNode->GetID(), ImNode::ScreenToCanvas(ImGui::GetMousePos()));
        }

        ImGui::EndPopup();
    }

    ImNode::Resume();
}