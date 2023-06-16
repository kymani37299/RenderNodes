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

    m_NewNodeMenu = Ptr<NewNodeContextMenu>(new NewNodeContextMenu{ "New node menu", *nodeGraph });
    m_NodeMenu = Ptr<NodeContextMenu>(new NodeContextMenu{ "Node menu", *nodeGraph });
    m_LinkMenu = Ptr<LinkContextMenu>(new LinkContextMenu{ "Link menu", *nodeGraph });
    m_PinMenu = Ptr<PinContextMenu>(new PinContextMenu{ "Pin menu", *nodeGraph });
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
                m_NewNodeMenu->Open(pin.Get());
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

    if (ImNode::ShowNodeContextMenu(&node)) m_NodeMenu->Open(node.Get());
    else if (ImNode::ShowPinContextMenu(&pin)) m_PinMenu->Open(pin.Get());
    else if (ImNode::ShowLinkContextMenu(&link)) m_LinkMenu->Open(link.Get());
    else if (ImNode::ShowBackgroundContextMenu()) m_NewNodeMenu->Open(0);
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

void RenderPipelineEditor::RenderContextMenus()
{
    m_NewNodeMenu->Draw();
    m_NodeMenu->Draw();
    m_LinkMenu->Draw();
    m_PinMenu->Draw();
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
