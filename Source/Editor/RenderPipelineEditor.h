#pragma once

#include <inttypes.h>

#include "../Common.h"
#include "../NodeGraph/NodeGraph.h"
#include "Drawing/EditorContextMenu.h"

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

    Ptr<NewNodeContextMenu> m_NewNodeMenu;
    Ptr<NodeContextMenu> m_NodeMenu;
    Ptr<LinkContextMenu> m_LinkMenu;
    Ptr<PinContextMenu> m_PinMenu;

    Ptr<NodeGraph> m_NodeGraph;
};