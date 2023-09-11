#include "NodeGraphCommands.h"

#include <unordered_set>

#include "../Common.h"

static std::unordered_set<EditorNode*> GetSelectedNodes(NodeGraph& nodeGraph)
{
	std::unordered_set<EditorNode*> selectedNodes{};
	const auto fn = [&selectedNodes](EditorNode* node) {
		if (ImNode::IsNodeSelected(node->GetID()))
			selectedNodes.insert(node);
	};
	nodeGraph.ForEachNode(fn);
	return selectedNodes;
}

NodeGraphCommandExecutor::~NodeGraphCommandExecutor()
{
	for (const auto& pendingCommand : m_PendingCommands)
	{
		delete pendingCommand;
	}
}

void NodeGraphCommandExecutor::SetNodeGraph(NodeGraph* nodeGraph)
{
	m_NodeGraph = nodeGraph;
	while (!m_Commands.empty()) m_Commands.pop();
}

void NodeGraphCommandExecutor::ExecuteCommand(NodeGraphCommand* command)
{
	ASSERT(m_NodeGraph && command);
	m_PendingCommands.push_back(command);
}

void NodeGraphCommandExecutor::ExecutePendingCommands()
{
	for (const auto& pendingCommand : m_PendingCommands)
	{
		pendingCommand->Execute(m_Context, *m_NodeGraph);
		m_Commands.push(Ptr<NodeGraphCommand>(pendingCommand));
	}
	m_PendingCommands.clear();

	if (m_PerformUndo)
	{
		m_PerformUndo = false;

		while (!m_Commands.empty() && m_Commands.top()->ShouldSkipUndo())
			m_Commands.pop();

		if (m_Commands.empty())
			return;

		NodeGraphCommand* command = m_Commands.top().get();
		ASSERT(command);

		command->Undo(m_Context, *m_NodeGraph);

		m_Commands.pop();
	}
}

void AddNodeNodeGraphCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	EditorNodePin nodePin;
	if (m_AttachedPin) nodePin = nodeGraph.GetPinByID(m_AttachedPin);

	nodeGraph.AddNode(m_Node);
	ImNode::SetNodePosition(m_Node->GetID(), { m_NodePosition.x, m_NodePosition.y });

	if (nodePin.Type != PinType::Invalid)
	{
		EditorNodePin targetPin;
		for (const auto& pin : m_Node->GetPins())
		{
			if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
			{
				targetPin = pin;
				break;
			}
		}
		
		if (targetPin.Type == PinType::Invalid)
		{
			for (const auto& pin : m_Node->GetCustomPins())
			{
				if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
				{
					targetPin = pin;
					break;
				}
			}
		}

		if (targetPin.Type != PinType::Invalid)
		{
			const PinID startPin = nodePin.IsInput ? targetPin.ID : nodePin.ID;
			const PinID endPin = nodePin.IsInput ? nodePin.ID : targetPin.ID;
			nodeGraph.AddLink({ IDGen::Generate(), startPin, endPin });
		}
	}
}

void AddNodeNodeGraphCommand::Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	nodeGraph.RemoveNode(m_Node->GetID());
}

void AddLinkNodeGraphCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	ASSERT(m_StartPin && m_EndPin);

	m_LinkID = IDGen::Generate();
	nodeGraph.AddLink({ m_LinkID, m_StartPin, m_EndPin });
}

void AddLinkNodeGraphCommand::Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	ASSERT(m_LinkID);

	nodeGraph.RemoveLink(m_LinkID);
}

void RemoveLinkNodeGraphCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	ASSERT(m_LinkID);

	m_BackupLink = nodeGraph.GetLinkByID(m_LinkID);
	nodeGraph.RemoveLink(m_LinkID);
}

void RemoveLinkNodeGraphCommand::Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	ASSERT(m_BackupLink.ID);

	nodeGraph.AddLink(m_BackupLink);
}

void RemovePinNodeGraphCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	ASSERT(m_PinID);

	m_BackupPin = nodeGraph.GetPinByID(m_PinID);
	nodeGraph.RemovePin(m_PinID);
}

void RemovePinNodeGraphCommand::Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	ASSERT(m_BackupPin.ID && m_BackupPinOwner);
	m_BackupPinOwner->AddCustomPin(m_BackupPin);
}

void RemoveSelectedNodesNodeGraphCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	const auto selectedNodes = GetSelectedNodes(nodeGraph);

	for (const auto& selectedNode : selectedNodes)
	{
		if (selectedNode->GetType() != EditorNodeType::OnStart && selectedNode->GetType() != EditorNodeType::OnUpdate)
		{
			// TODO: Same as AddNode command - we need to keep the links also
			m_RemovedNodesBackup.push_back(Ptr<EditorNode>(selectedNode->Clone()));

			const auto selectedNodePos = ImNode::GetNodePosition(selectedNode->GetID());
			m_RemovedNodesLocations.push_back({ selectedNodePos.x , selectedNodePos.y });

			nodeGraph.RemoveNode(selectedNode->GetID());
		}
	}
}

void RemoveSelectedNodesNodeGraphCommand::Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	ASSERT(m_RemovedNodesLocations.size() == m_RemovedNodesBackup.size());

	for (unsigned i = 0; i < m_RemovedNodesBackup.size(); i++)
	{
		EditorNode* node = m_RemovedNodesBackup[i].get();
		const Float2 nodePos = m_RemovedNodesLocations[i];

		nodeGraph.AddNode(node);
		ImNode::SetNodePosition(node->GetID(), { nodePos.x, nodePos.y });
	}
}

void CopySelectedNodesNodeGraphCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	const auto selectedNodes = GetSelectedNodes(nodeGraph);

	auto& copiedNodes = context.CopiedNodes;
	copiedNodes.clear();

	Float2 locationSum{ 0.0f, 0.0f };
	for (const auto& selectedNode : selectedNodes)
	{
		if (selectedNode->GetType() != EditorNodeType::OnStart && selectedNode->GetType() != EditorNodeType::OnUpdate)
		{
			const NodeID nodeID = selectedNode->GetID();
			copiedNodes.insert(nodeID);

			const ImVec2 nodePos = ImNode::GetNodePosition(nodeID);
			locationSum.x += nodePos.x;
			locationSum.y += nodePos.y;
		}
	}
	context.RootCopyLocation = locationSum / (float)selectedNodes.size();
}

void PasteNodesNodeGraphCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	const Float2 rootCopyLocation = context.RootCopyLocation;
	const auto& copiedNodes = context.CopiedNodes;

	std::unordered_map<PinID, PinID> pinMapping{};

	for (const auto& copiedNodeID : copiedNodes)
	{
		EditorNode* copiedNode = nodeGraph.GetNodeByID(copiedNodeID);
		EditorNode* pastedNode = copiedNode->Clone();

		const ImVec2 copiedNodeLocation = ImNode::GetNodePosition(copiedNodeID);
		const ImVec2 localNodeLocation{ copiedNodeLocation.x - rootCopyLocation.x, copiedNodeLocation.y - rootCopyLocation.y };
		const ImVec2 pastedNodeOffset = ImNode::ScreenToCanvas(ImGui::GetMousePos());
		const ImVec2 pastedNodeLocation = localNodeLocation + pastedNodeOffset;

		ImNode::SetNodePosition(pastedNode->GetID(), pastedNodeLocation);
		nodeGraph.AddNode(pastedNode);
		m_PastedNodes.push_back(pastedNode->GetID());

		const auto& copiedPins = copiedNode->GetPins();
		const auto& copiedCustomPins = copiedNode->GetCustomPins();

		const auto& pastedPins = pastedNode->GetPins();
		const auto& pastedCustomPins = pastedNode->GetCustomPins();

		ASSERT(copiedPins.size() == pastedPins.size() && copiedCustomPins.size() == pastedCustomPins.size());

		for (unsigned i = 0; i < copiedPins.size(); i++) pinMapping[copiedPins[i].ID] = pastedPins[i].ID;
		for (unsigned i = 0; i < copiedCustomPins.size(); i++) pinMapping[copiedCustomPins[i].ID] = pastedCustomPins[i].ID;
	}

	std::vector<EditorNodeLink> linksToAdd{};
	const auto addLinks = [&nodeGraph, &copiedNodes, &pinMapping, &linksToAdd](const EditorNodeLink& link)
	{
		const NodeID startID = nodeGraph.GetPinOwner(link.Start)->GetID();
		const NodeID endID = nodeGraph.GetPinOwner(link.End)->GetID();

		if (copiedNodes.count(startID) > 0 && copiedNodes.count(endID) > 0)
		{
			EditorNodeLink newLink;
			newLink.ID = IDGen::Generate();
			newLink.Start = pinMapping[link.Start];
			newLink.End = pinMapping[link.End];
			linksToAdd.push_back(newLink);
		}
	};
	nodeGraph.ForEachLink(addLinks);

	for (const auto& link : linksToAdd)
		nodeGraph.AddLink(link);
}

void PasteNodesNodeGraphCommand::Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	for (const auto& pastedNode : m_PastedNodes)
	{
		nodeGraph.RemoveNode(pastedNode);
	}
}

void MakePinConstantCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	EditorNodePin pin = nodeGraph.GetPinByID(m_PinID);
	pin.HasConstantValue = true;
	pin.ConstantValue.SetDefaultValue(pin.Type);
	nodeGraph.UpdatePin(pin);

	const auto fn = [this](const EditorNodeLink& link)
	{
		if (link.Start == m_PinID || link.End == m_PinID)
		{
			m_DeletedLinks.push_back(link);
		}
	};
	nodeGraph.ForEachLink(fn);

	for (const auto& link : m_DeletedLinks)
		nodeGraph.RemoveLink(link.ID);
}

void MakePinConstantCommand::Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	EditorNodePin pin = nodeGraph.GetPinByID(m_PinID);
	pin.HasConstantValue = false;
	nodeGraph.UpdatePin(pin);

	for (const auto& link : m_DeletedLinks)
		nodeGraph.AddLink(link);
}

void MakeConstantToPinCommand::Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	EditorNodePin pin = nodeGraph.GetPinByID(m_PinID);
	pin.HasConstantValue = false;

	m_Value = pin.ConstantValue;
	nodeGraph.UpdatePin(pin);
}

void MakeConstantToPinCommand::Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph)
{
	EditorNodePin pin = nodeGraph.GetPinByID(m_PinID);
	pin.HasConstantValue = true;
	pin.ConstantValue = m_Value;
	nodeGraph.UpdatePin(pin);
}
