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

void NodeGraph::AddLink(const EditorNodeLink& link)
{
    m_Links[link.ID] = link;
}

void NodeGraph::RemoveNode(NodeID nodeID)
{
    m_Nodes.erase(nodeID);
}

void NodeGraph::RemoveLink(LinkID linkID)
{
    m_Links.erase(linkID);
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
    EditorNodePin outputPin;
    const auto fn = [&pinID, &outputPin](const EditorNodePin& pin)
    {
        if (pin.ID == pinID) outputPin = pin;
    };

    for (const auto& it : m_Nodes)
    {
        const auto& node = it.second;
        node->ForEachPin(fn);
        if (outputPin.Type != PinType::Invalid) break;
    }

    ASSERT_M(outputPin.Type != PinType::Invalid, "Pin not found!");
    return outputPin;
}

EditorNode* NodeGraph::GetPinOwner(PinID pinID) const
{
    EditorNode* outputNode = nullptr;
    for (const auto& it : m_Nodes)
    {
        const auto& node = it.second;
        const auto fn = [&pinID, &outputNode, &node](const EditorNodePin& pin)
        {
            if (pinID == pin.ID)
                outputNode = node.get();
        };
        node->ForEachPin(fn);

        if (outputNode != nullptr) break;
    }
    ASSERT(outputNode);
    return outputNode;
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