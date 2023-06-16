#pragma once

#include <string>
#include <vector>

#include "../../IDGen.h"

class NodeGraph;

class EditorContextMenu
{
public:
	EditorContextMenu(const std::string& id) :
		m_ID(id) {}

	void Open()
	{
		m_ShouldOpen = true;
	}

	void Draw();

protected:
	virtual void DrawContent() = 0;

private:
	bool m_ShouldOpen = false;
	std::string m_ID;
};

class EditorComboBox : public EditorContextMenu
{
public:
	EditorComboBox(const std::string& id, std::string& valueRef, const std::vector<std::string>& values) :
		EditorContextMenu(id),
		m_ValueRef(valueRef),
		m_Values(values) {}

protected:
	void DrawContent() override;

private:
	std::string& m_ValueRef;
	std::vector<std::string> m_Values;
};

class NewNodeContextMenu : public EditorContextMenu
{
public:
	NewNodeContextMenu(const std::string& id, NodeGraph& nodeGraph) :
		EditorContextMenu(id),
		m_NodeGraph(nodeGraph)
	{}

	void Open() = delete;
	void Open(PinID draggedPin)
	{
		EditorContextMenu::Open();
		m_DraggedPin = draggedPin;
	}

protected:
	void DrawContent() override;

private:
	NodeGraph& m_NodeGraph;
	PinID m_DraggedPin = 0;
};

class NodeContextMenu : public EditorContextMenu
{
public:
	NodeContextMenu(const std::string& id, NodeGraph& nodeGraph) :
		EditorContextMenu(id),
		m_NodeGraph(nodeGraph) {}

	void Open() = delete;
	void Open(NodeID nodeID)
	{
		EditorContextMenu::Open();
		m_NodeID = nodeID;
	}

protected:
	void DrawContent() override;

private:
	NodeGraph& m_NodeGraph;
	NodeID m_NodeID = 0;
};

class LinkContextMenu : public EditorContextMenu
{
public:
	LinkContextMenu(const std::string& id, NodeGraph& nodeGraph) :
		EditorContextMenu(id),
		m_NodeGraph(nodeGraph) {}

	void Open() = delete;
	void Open(LinkID linkID)
	{
		EditorContextMenu::Open();
		m_LinkID = linkID;
	}

protected:
	void DrawContent() override;

private:
	NodeGraph& m_NodeGraph;
	LinkID m_LinkID = 0;
};

class PinContextMenu : public EditorContextMenu
{
public:
	PinContextMenu(const std::string& id, NodeGraph& nodeGraph) :
		EditorContextMenu(id),
		m_NodeGraph(nodeGraph) {}

	void Open() = delete;
	void Open(PinID pinID)
	{
		EditorContextMenu::Open();
		m_PinID = pinID;
	}

protected:
	void DrawContent() override;

private:
	NodeGraph& m_NodeGraph;
	PinID m_PinID = 0;
};