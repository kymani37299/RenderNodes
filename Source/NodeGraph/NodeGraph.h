#pragma once

#include <unordered_map>

#include "../Editor/EditorNode.h"

class NodeGraph
{
public:
    static NodeGraph* CreateDefaultNodeGraph();
public:

    void AddNode(EditorNode* node);
    void AddLink(const EditorNodeLink& link);

    void RemoveNode(NodeID nodeID);
    void RemoveLink(LinkID linkID);

    EditorNodePin GetPinByID(PinID pinID) const;
    PinID GetInputPinFromOutput(PinID outputPinID) const;
    PinID GetOutputPinForInput(PinID inputPinID) const;
    EditorNode* GetPinOwner(PinID pinID) const;

    OnStartEditorNode* GetOnStartNode() const;
    OnUpdateEditorNode* GetOnUpdateNode() const;

    template<typename Fn>
    void ForEachNode(Fn& fn)
    {
        for (const auto& it : m_Nodes)
        {
            fn(it.second.get());
        }
    }

    template<typename Fn>
    void ForEachNode(Fn& fn) const
    {
        for (const auto& it : m_Nodes)
        {
            fn(it.second.get());
        }
    }

    template<typename Fn>
    void ForEachLink(Fn& fn)
    {
        for (const auto& it : m_Links)
        {
            fn(it.second);
        }
    }

    template<typename Fn>
    void ForEachLink(Fn& fn) const
    {
        for (const auto& it : m_Links)
        {
            fn(it.second);
        }
    }

    unsigned GetNodeCount() const { return m_Nodes.size(); }
    unsigned GetLinkCount() const { return m_Links.size(); }

private:
    std::unordered_map<NodeID, Ptr<EditorNode>> m_Nodes;
    std::unordered_map<LinkID, EditorNodeLink> m_Links;
};