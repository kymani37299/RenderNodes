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

void NodeGraph::ReplaceNode(NodeID nodeID, EditorNode* node)
{
    ASSERT(node->GetID() == nodeID);
    ASSERT(m_Nodes.find(nodeID) != m_Nodes.end());
    m_Nodes[node->GetID()] = Ptr<EditorNode>(node);
}

void NodeGraph::ReplacePinLinks(PinID oldPin, PinID newPin)
{
    for (auto& it : m_Links)
    {
        auto& l = it.second;

        if (l.Start == oldPin)
            l.Start = newPin;

        if (l.End == oldPin)
            l.End = newPin;
    }
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
    RemoveAllLinks(nodeID);

    m_Nodes.erase(nodeID);
    m_NodePositions.erase(nodeID);
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

void NodeGraph::RemoveAllLinks(NodeID nodeID)
{
    EditorNode* node = m_Nodes[nodeID].get();

	std::vector<LinkID> linksToRemove;
	for (const auto& it : m_Links)
	{
		const auto& link = it.second;
		if (GetPinOwner(link.Start) == node || GetPinOwner(link.End) == node)
			linksToRemove.push_back(link.ID);
	}

	for (const LinkID& linkID : linksToRemove)
		RemoveLink(linkID);
}

void NodeGraph::RemoveAllPins(NodeID nodeID)
{
    EditorNode* node = m_Nodes[nodeID].get();

    std::vector<PinID> pinsToRemove{};
    pinsToRemove.reserve(node->GetPins().size());
    for (const auto& pin : node->GetPins())
    {
        pinsToRemove.push_back(pin.ID);
    }

    for (const auto& pin : pinsToRemove)
    {
        RemovePin(pin);
    }
}

bool NodeGraph::ContainsNode(NodeID nodeID) const
{
    return m_Nodes.find(nodeID) != m_Nodes.end();
}

EditorNode* NodeGraph::GetNodeByID(NodeID nodeID) const
{
    ASSERT(ContainsNode(nodeID));
    return m_Nodes.at(nodeID).get();
}

EditorNodeLink NodeGraph::GetLinkByID(LinkID linkID) const
{
    ASSERT(m_Links.find(linkID) != m_Links.end());
    return m_Links.at(linkID);
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

void NodeGraph::UpdatePin(const EditorNodePin& newPin) const
{
    EditorNode* node = GetPinOwner(newPin.ID);
    ASSERT_M(node, "Pin not found!");
    if(node) node->UpdatePin(newPin);
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

void NodeGraph::RefreshNodes(const VariablePool& variablePool)
{
	for (auto& it : m_Nodes)
	{
        RefreshNode(it.second.get(), variablePool);
	}
}

void NodeGraph::RefreshNode(EditorNode* editorNode, const VariablePool& variablePool)
{
	switch (editorNode->GetType())
	{
	case EditorNodeType::Custom:
	{
		CustomEditorNode* node = static_cast<CustomEditorNode*>(editorNode);
		node->RegeneratePins();
	} break;
	case EditorNodeType::Variable:
	{
		VariableEditorNode* node = static_cast<VariableEditorNode*>(editorNode);
		node->RefreshLabel(variablePool);
	} break;
	case EditorNodeType::AsignVariable:
	{
		AsignVariableEditorNode* node = static_cast<AsignVariableEditorNode*>(editorNode);
		node->RefreshLabel(variablePool);
	} break;
	}
}
