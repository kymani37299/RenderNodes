#pragma once

#include <inttypes.h>

#include "../Common.h"
#include "../NodeGraph/NodeGraph.h"

namespace EditorPrivate
{
    UniqueID GenerateID();
}

class RenderPipelineEditor
{
public:
    ~RenderPipelineEditor();

    void Render();

    const NodeGraph& GetNodeGraph() const
    {
        return *m_NodeGraph;
    }

    void Load(const std::string& settingsPath, NodeGraph* nodeGraph);

private:
    void UpdateEditor();
    void RenderEditor();

    void RenderContextMenus();

private:
    ImNode::EditorContext* m_EditorContext = nullptr;

    ImNode::NodeId m_ContextNodeID;
    ImNode::PinId m_ContextPinID;
    ImNode::LinkId m_ContextLinkID;

    Ptr<NodeGraph> m_NodeGraph;
};