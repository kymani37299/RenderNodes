#include "NodeGraph.h"

#include "../Editor/RenderPipelineEditor.h"

NodeGraph* NodeGraph::CreateDefaultNodeGraph()
{
    NodeGraph* nodeGraph = new NodeGraph{};
    nodeGraph->AddNode(new OnStartEditorNode());
    nodeGraph->AddNode(new OnUpdateEditorNode());
    return nodeGraph;
}

void NodeGraph::AddNode(EditorNode* node)
{
    m_Nodes[node->GetID()] = Ptr<EditorNode>(node);
}

void NodeGraph::AddLink(const EditorNodeLink& link)
{
    for (const auto& it : m_Links)
    {
        const auto& l = it.second;
        const auto& endPin = GetPinByID(link.End);
        if (endPin.Type != PinType::Execution && l.End == link.End)
        {
            m_Links.erase(it.first);
            break;
        }

        const auto& startPin = GetPinByID(link.Start);
        if (startPin.Type == PinType::Execution && l.Start == link.Start)
        {
            m_Links.erase(it.first);
            break;
        }
    }
    m_Links[link.ID] = link;
}

void NodeGraph::RemoveNode(NodeID nodeID)
{
    EditorNode* node = m_Nodes[nodeID].get();

    // Also remove all links that were attached to this node
    std::vector<LinkID> linksToRemove;
    for (const auto& it : m_Links)
    {
		const auto& link = it.second;
        if (GetPinOwner(link.Start) == node || GetPinOwner(link.End) == node)
            linksToRemove.push_back(link.ID);
    }

    for (const LinkID& linkID : linksToRemove)
        RemoveLink(linkID);

    m_Nodes.erase(nodeID);
}

void NodeGraph::RemoveLink(LinkID linkID)
{
    m_Links.erase(linkID);
}

void NodeGraph::RemovePin(PinID pinID)
{
    // Also remove all links that were attached to this pin
	std::vector<LinkID> linksToRemove;
	for (const auto& it : m_Links)
	{
		const auto& link = it.second;
		if (link.Start == pinID || link.End == pinID)
			linksToRemove.push_back(link.ID);
	}

	for (const LinkID& linkID : linksToRemove)
		RemoveLink(linkID);

    EditorNode* node = GetPinOwner(pinID);
    node->RemovePin(pinID);
}

PinID NodeGraph::GetInputPinFromOutput(PinID outputPinID) const
{
    for (const auto& it : m_Links)
    {
        const auto& link = it.second;
        if (link.Start == outputPinID)
            return link.End;
    }
    return 0;
}

PinID NodeGraph::GetOutputPinForInput(PinID inputPinID) const
{
    for (const auto& it : m_Links)
    {
        const auto& link = it.second;
        if (link.End == inputPinID)
            return link.Start;
    }
    return 0;
}

EditorNodePin NodeGraph::GetPinByID(PinID pinID) const
{
	for (const auto& it : m_Nodes)
	{
        const auto& node = it.second;
        for (const auto& pin : node->GetPins())
        {
            if (pin.ID == pinID) return pin;
        }
		for (const auto& pin : node->GetCustomPins())
		{
			if (pin.ID == pinID) return pin;
		}
	}
	ASSERT_M(0, "Pin not found!");
    return EditorNodePin{};
}

EditorNode* NodeGraph::GetPinOwner(PinID pinID) const
{
    for (const auto& it : m_Nodes)
    {
        const auto& node = it.second;
        for (const auto& pin : node->GetPins())
        {
            if (pinID == pin.ID) return node.get();
        }
		for (const auto& pin : node->GetCustomPins())
		{
			if (pinID == pin.ID) return node.get();
		}
    }
    return nullptr;
}

bool NodeGraph::IsCustomPin(PinID pinID) const
{
    for (const auto& it : m_Nodes)
    {
        const auto& node = it.second;
        for (const auto& pin : node->GetCustomPins())
        {
            if (pinID == pin.ID) return true;
        }
    }
    return false;
}

OnStartEditorNode* NodeGraph::GetOnStartNode() const
{
    for (const auto& it : m_Nodes)
    {
        const auto& node = it.second;
        if (node->GetType() == EditorNodeType::OnStart)
        {
            return static_cast<OnStartEditorNode*>(node.get());
        }
    }
    ASSERT_M(0, "Unable to find on start node");
    return nullptr;
}

OnUpdateEditorNode* NodeGraph::GetOnUpdateNode() const
{
    for (const auto& it : m_Nodes)
    {
        const auto& node = it.second;
        if (node->GetType() == EditorNodeType::OnUpdate)
        {
            return static_cast<OnUpdateEditorNode*>(node.get());
        }
    }
    ASSERT_M(0, "Unable to find on update node");
    return nullptr;
}