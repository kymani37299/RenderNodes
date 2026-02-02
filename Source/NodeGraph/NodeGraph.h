#pragma once

#include <unordered_map>

#include "../Editor/EditorNode.h"
#include "../Editor/ExecutorEditorNode.h"
#include "../Editor/EvaluationEditorNode.h"

class NodeGraph
{
public:
    static NodeGraph* CreateDefaultNodeGraph();
public:

    void AddNode(EditorNode* node);
    void ReplaceNode(NodeID nodeID, EditorNode* node);
    void ReplacePinLinks(PinID oldPin, PinID newPin);
    void AddLink(const EditorNodeLink& link);

    void RemoveNode(NodeID nodeID);
    void RemoveLink(LinkID linkID);
    void RemovePin(PinID pinID);

    void RemoveAllLinks(NodeID nodeID);
    void RemoveAllPins(NodeID nodeID);
    
    bool ContainsNode(NodeID nodeID) const;

    EditorNode* GetNodeByID(NodeID nodeID) const;
    EditorNodeLink GetLinkByID(LinkID linkID) const;
	EditorNodePin GetPinByID(PinID pinID) const;

    void UpdatePin(const EditorNodePin& newPin) const;

    EditorNode* GetPinOwner(PinID pinID) const;
    bool IsCustomPin(PinID pinID) const;

    PinID GetInputPinFromOutput(PinID outputPinID) const;
    PinID GetOutputPinForInput(PinID inputPinID) const;

    OnStartEditorNode* GetOnStartNode() const;
    OnUpdateEditorNode* GetOnUpdateNode() const;

    void RefreshNodes(const VariablePool& variablePool);
    void RefreshNode(EditorNode* node, const VariablePool& variablePool);

    template<typename Fn>
    void ForEachNode(Fn& fn) const
    {
        for (const auto& it : m_Nodes)
        {
            fn(it.second.get());
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

    const std::unordered_map<NodeID, ImVec2>& GetNodePositions() const { return m_NodePositions; }
    void SetNodePositions(const std::unordered_map<NodeID, ImVec2>& nodePositions) { m_NodePositions = nodePositions; }

private:
    std::unordered_map<NodeID, Ptr<EditorNode>> m_Nodes;
    std::unordered_map<LinkID, EditorNodeLink> m_Links;

    std::unordered_map<NodeID, ImVec2> m_NodePositions;
};