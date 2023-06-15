#pragma once

#include <inttypes.h>

#include "../Common.h"
#include "../NodeGraph/NodeGraph.h"

class RenderPipelineEditor
{
public:
    ~RenderPipelineEditor();
	void InitializeDefaultNodePositions();

    void Render();

    
    const NodeGraph& GetNodeGraph() const
    {
        return *m_NodeGraph;
    }

    void Unload();
    void Load(NodeGraph* nodeGraph);

private:
    void UpdateEditor();
    void RenderEditor();

    void RenderContextMenus();
    void RenderNodePopups();

private:
    ImNode::EditorContext* m_EditorContext = nullptr;

    // Probably can be deleted
    ImNode::NodeId m_ContextNodeID;
    ImNode::PinId m_ContextPinID;
    ImNode::LinkId m_ContextLinkID;

    PinID m_NewNodePinID = 0;

    Ptr<NodeGraph> m_NodeGraph;
};