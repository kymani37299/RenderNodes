#pragma once

#include <unordered_set>

#include "../IDGen.h"

class EditorErrorHandler
{
public:
	void MarkErrorNode(NodeID nodeID)
	{
		m_ErrorNodes.insert(nodeID);
	}

	bool IsErrorNode(NodeID nodeID)
	{
		return m_ErrorNodes.count(nodeID) > 0;
	}

	void Clear()
	{
		m_ErrorNodes.clear();
	}

private:
	std::unordered_set<NodeID> m_ErrorNodes;
};