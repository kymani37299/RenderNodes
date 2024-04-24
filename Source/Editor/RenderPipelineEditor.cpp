#include "RenderPipelineEditor.h"

#include <atomic>

#include "../Common.h"
#include "../IDGen.h"
#include "../App/App.h"
#include "../NodeGraph/NodeGraphCommands.h"
#include "EditorErrorHandler.h"

RenderPipelineEditor::RenderPipelineEditor(NodeGraphCommandExecutor* commandExecutor) :
	m_CommandExecutor(commandExecutor)
{
}

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

    m_CommandExecutor->ExecutePendingCommands();

    ImNode::Begin("Render pipeline editor", ImVec2(0.0, 0.0f));

    UpdateEditor();
    RenderEditor();

    RenderNodePopups();
    RenderContextMenus();

    ImNode::End();
    ImNode::SetCurrentEditor(nullptr);
}

void RenderPipelineEditor::HandleKeyPressed(int key, int mods)
{
	if (mods == 0)
	{
		switch (key)
		{
		case GLFW_KEY_DELETE:
			m_CommandExecutor->ExecuteCommand(new RemoveSelectedNodesNodeGraphCommand{});
			break;
		}
	}

	if (mods & GLFW_MOD_CONTROL)
	{
		switch (key)
		{
		case GLFW_KEY_C:
			m_CommandExecutor->ExecuteCommand(new CopySelectedNodesNodeGraphCommand{});
			break;
		case GLFW_KEY_V:
			m_CommandExecutor->ExecuteCommand(new PasteNodesNodeGraphCommand{});
			break;
		case GLFW_KEY_D:
			m_CommandExecutor->ExecuteCommand(new RemoveSelectedNodesNodeGraphCommand{});
			break;
		case GLFW_KEY_Z:
			m_CommandExecutor->UndoCommand();
			break;
		}
	}
}

void RenderPipelineEditor::Unload()
{
    ImNode::DestroyEditor(m_EditorContext);
    m_CommandExecutor->SetNodeGraph(nullptr);
    m_NodeGraph = nullptr;
}

void RenderPipelineEditor::Load(NodeGraph* nodeGraph)
{
	m_NodeGraph = nodeGraph;
	m_CommandExecutor->SetNodeGraph(m_NodeGraph);

	m_NewNodeMenu = Ptr<NewNodeContextMenu>(new NewNodeContextMenu{ "New node menu", m_CommandExecutor.get(), m_CustomNodeEditor});
	m_NodeMenu = Ptr<NodeContextMenu>(new NodeContextMenu{ "Node menu", m_CommandExecutor.get()});
	m_LinkMenu = Ptr<LinkContextMenu>(new LinkContextMenu{ "Link menu", m_CommandExecutor.get()});
	m_PinMenu = Ptr<PinContextMenu>(new PinContextMenu{ "Pin menu", m_CommandExecutor.get()});

	ImNode::Config config;
	config.SettingsFile = m_CustomNodeEditor ? "CustomNodeEditor.json" : "NodeEditor.json";
	m_EditorContext = ImNode::CreateEditor(&config);
}

void RenderPipelineEditor::LoadNodePositions()
{
    if (!m_NodeGraph)
    {
        ASSERT(0);
        return;
    }

    ImNode::SetCurrentEditor(m_EditorContext);

    const auto& nodePositions = m_NodeGraph->GetNodePositions();
    const auto fn = [&nodePositions](EditorNode* node)
    {
        const NodeID nodeID = node->GetID();
        ImVec2 nodePosition{0.0f, 0.0f};
        try { nodePosition = nodePositions.at(nodeID); } catch (const std::out_of_range&) {}
        ImNode::SetNodePosition(nodeID, nodePosition);
    };
    m_NodeGraph->ForEachNode(fn);

    ImNode::SetCurrentEditor(nullptr);
}

void RenderPipelineEditor::SaveNodePositions()
{
    if (!m_NodeGraph)
    {
        ASSERT(0);
        return;
    }

    ImNode::SetCurrentEditor(m_EditorContext);

    std::unordered_map<NodeID, ImVec2> nodePositions;

    const auto fn = [&nodePositions](EditorNode* node)
    {
        const NodeID nodeID = node->GetID();
        nodePositions[nodeID] = ImNode::GetNodePosition(nodeID);
    };
    m_NodeGraph->ForEachNode(fn);
    m_NodeGraph->SetNodePositions(nodePositions);

    ImNode::SetCurrentEditor(nullptr);
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

                if (!EditorNodePin::CanBeLinked(startPin, endPin))
                {
                    ImNode::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (ImNode::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                {
                    m_CommandExecutor->ExecuteCommand(new AddLinkNodeGraphCommand(startPin.ID, endPin.ID));
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
                m_CommandExecutor->ExecuteCommand(new RemoveLinkNodeGraphCommand{ link.Get() });
            }
        }
    }
    ImNode::EndDelete();

    if (ImNode::ShowNodeContextMenu(&node)) m_NodeMenu->Open(node.Get());
    else if (ImNode::ShowPinContextMenu(&pin)) m_PinMenu->Open(pin.Get());
    else if (ImNode::ShowLinkContextMenu(&link)) m_LinkMenu->Open(link.Get());
    else if (ImNode::ShowBackgroundContextMenu()) m_NewNodeMenu->Open(0);

    const ImNode::NodeId doubleClickedNode = ImNode::GetDoubleClickedNode();
    if (doubleClickedNode)
    {
        const NodeID nodeID = doubleClickedNode.Get();
        EditorNode* editorNode = m_NodeGraph->GetNodeByID(nodeID);
        if (editorNode->GetType() == EditorNodeType::Custom)
        {
            App::Get()->OpenCustomNode(static_cast<CustomEditorNode*>(editorNode)->GetName());
        }
    }
}

void RenderPipelineEditor::RenderEditor()
{
    EditorErrorHandler& errorHandler = App::Get()->GetErrorHandler();

    const auto renderNode = [&errorHandler](EditorNode* node) {
        bool isError = errorHandler.IsErrorNode(node->GetID());
        if (isError)
        {
            ImNode::PushStyleColor(ImNode::StyleColor::StyleColor_NodeBorder, ImVec4(0.8f, 0.1f, 0.0f, 1.0f));
        }
        node->Render();
        if (isError)
        {
            ImNode::PopStyleColor();
        }

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

CustomNodePipelineEditor::CustomNodePipelineEditor():
    RenderPipelineEditor(new NodeGraphCommandExecutor{})
{
    m_CustomNodeEditor = true;
    Load(new NodeGraph{});
}

CustomNodePipelineEditor::CustomNodePipelineEditor(CustomEditorNode* node):
    RenderPipelineEditor(new NodeGraphCommandExecutor{})
{
    m_CustomNodeEditor = true;
    m_ShouldLoadPositions = true;
    m_Node = node;
    Load(node->GetNodeGraph());
}

CustomNodePipelineEditor::~CustomNodePipelineEditor()
{
    if (m_Node == nullptr) delete m_NodeGraph;
    m_NodeGraph = nullptr;
}

bool CustomNodePipelineEditor::CanCreateNode() const
{
	// Check for empty name
	bool canBeCreated = !m_Name.empty();

	// Check if it has at least one input/output pin
	if (canBeCreated)
	{
		bool atLeastOnePin = false;
		const auto fn = [&canBeCreated, &atLeastOnePin](EditorNode* node)
		{
			if (node->GetType() == EditorNodeType::Pin)
				atLeastOnePin = true;
		};
		m_NodeGraph->ForEachNode(fn);

		canBeCreated = atLeastOnePin;
	}

	// Check if there is already node with that name
	if (canBeCreated)
	{
        const auto& customNodes = *App::Get()->GetCustomNodes();
		for (const auto& customNode : customNodes)
		{
			if (customNode->GetName() == m_Name)
			{
				canBeCreated = false;
				break;
			}
		}
	}

	return canBeCreated;
}

CustomEditorNode* CustomNodePipelineEditor::CreateNode()
{
    ASSERT(!m_Node);

    m_Node = new CustomEditorNode{ nullptr, m_Name, m_NodeGraph };
    SaveNodePositions();

    return m_Node;
}

void CustomNodePipelineEditor::RewriteNode()
{
    ASSERT(m_Node);
    SaveNodePositions();
}

void CustomNodePipelineEditor::LoadPositionsIfNeeded()
{
    if (m_ShouldLoadPositions)
    {
        m_ShouldLoadPositions = false;
        LoadNodePositions();
    }

}
