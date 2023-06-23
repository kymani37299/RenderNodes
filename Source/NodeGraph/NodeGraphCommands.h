#pragma once

#include <stack>
#include <vector>

#include "NodeGraph.h"

struct NodeGraphCommandExecutorContext
{
	Float2 RootCopyLocation{};
	std::unordered_set<NodeID> CopiedNodes;
};

class NodeGraphCommand
{
public:
	virtual void Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) = 0;
	virtual void Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) = 0;

	virtual bool ShouldSkipUndo() const { return false; }
};

class NodeGraphCommandExecutor
{
public:
	~NodeGraphCommandExecutor();

	const NodeGraphCommandExecutorContext& GetContext() const { return m_Context; }

	NodeGraph* GetNodeGraph() { return m_NodeGraph; }
	void SetNodeGraph(NodeGraph* nodeGraph);

	void ExecuteCommand(NodeGraphCommand* command);
	void UndoCommand() { m_PerformUndo = true; }

	void ExecutePendingCommands();

	unsigned GetExecutedCommandCount() const { return m_Commands.size(); }

private:
	bool m_PerformUndo = false;

	NodeGraph* m_NodeGraph;
	NodeGraphCommandExecutorContext m_Context;

	std::vector<NodeGraphCommand*> m_PendingCommands;
	std::stack<Ptr<NodeGraphCommand>> m_Commands;
};

class AddNodeNodeGraphCommand : public NodeGraphCommand
{
public:
	AddNodeNodeGraphCommand(EditorNode* node, Float2 nodePosition, PinID attachedPin = 0) :
		m_Node(node),
		m_NodePosition(nodePosition),
		m_AttachedPin(attachedPin) {}

	void Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;
	void Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;

private:
	EditorNode* m_Node;
	Float2 m_NodePosition;
	PinID m_AttachedPin;
};

class AddLinkNodeGraphCommand : public NodeGraphCommand
{
public:
	AddLinkNodeGraphCommand(PinID startPin, PinID endPin) :
		m_StartPin(startPin),
		m_EndPin(endPin) { }

	void Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;
	void Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;

private:
	PinID m_StartPin;
	PinID m_EndPin;

	PinID m_LinkID = 0;
};

class RemoveLinkNodeGraphCommand : public NodeGraphCommand
{
public:
	RemoveLinkNodeGraphCommand(LinkID linkID) :
		m_LinkID(linkID) {}

	void Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;
	void Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;

private:
	LinkID m_LinkID;

	EditorNodeLink m_BackupLink;
};

class RemovePinNodeGraphCommand : public NodeGraphCommand
{
public:
	RemovePinNodeGraphCommand(PinID pinID) :
		m_PinID(pinID) {}

	void Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;
	void Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;

private:
	PinID m_PinID;

	EditorNode* m_BackupPinOwner;
	EditorNodePin m_BackupPin;
};

class RemoveSelectedNodesNodeGraphCommand : public NodeGraphCommand
{
public:
	void Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;
	void Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;

private:
	std::vector<Ptr<EditorNode>> m_RemovedNodesBackup;
	std::vector<Float2> m_RemovedNodesLocations;
};

class CopySelectedNodesNodeGraphCommand : public NodeGraphCommand
{
public:
	void Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;
	void Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override {}

	bool ShouldSkipUndo() const override { return true; }
};

class PasteNodesNodeGraphCommand : public NodeGraphCommand
{
public:
	void Execute(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;
	void Undo(NodeGraphCommandExecutorContext& context, NodeGraph& nodeGraph) override;

private:
	std::vector<NodeID> m_PastedNodes;
};