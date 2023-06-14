#include "RenderPipelineEditor.h"

#include <atomic>

#include "../Common.h"
#include "../IDGen.h"

RenderPipelineEditor::~RenderPipelineEditor()
{
    Unload();
   
}

void RenderPipelineEditor::InitializeDefaultNodePositions()
{
    ImNode::SetCurrentEditor(m_EditorContext);
    ImNode::SetNodePosition(m_NodeGraph->GetOnStartNode()->GetID(), ImVec2{ 100, 200 });
    ImNode::SetNodePosition(m_NodeGraph->GetOnUpdateNode()->GetID(), ImVec2{ 100, 600 });
    ImNode::SetCurrentEditor(nullptr);
}

void RenderPipelineEditor::Render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImNode::SetCurrentEditor(m_EditorContext);
    ImNode::Begin("Render pipeline editor", ImVec2(0.0, 0.0f));

    UpdateEditor();
    RenderEditor();

    RenderContextMenus();

    ImNode::End();
    ImNode::SetCurrentEditor(nullptr);
}

void RenderPipelineEditor::Unload()
{
    ImNode::DestroyEditor(m_EditorContext);
}

void RenderPipelineEditor::Load(NodeGraph* nodeGraph)
{
    ImNode::Config config;
    config.SettingsFile = "NodeEditor.json";
    m_EditorContext = ImNode::CreateEditor(&config);

    m_NodeGraph = Ptr<NodeGraph>(nodeGraph);
}

void RenderPipelineEditor::UpdateEditor()
{
    ImNode::PinId startPinID, endPinID, pin;
    ImNode::LinkId link;
    ImNode::NodeId node;

    if (ImNode::BeginCreate(ImColor(255, 255, 255), 2.0f))
    {
        if (ImNode::QueryNewLink(&startPinID, &endPinID))
        {
            if (startPinID && endPinID)
            {
                auto startPin = m_NodeGraph->GetPinByID(startPinID.Get());
                auto endPin = m_NodeGraph->GetPinByID(endPinID.Get());

                // Make sure start is always output pin
                if (startPin.IsInput)
                {
                    const auto tmpPin = startPin;
                    startPin = endPin;
                    endPin = tmpPin;
                }

                bool validLink = startPin.ID != endPin.ID;
                validLink = validLink && startPin.IsInput != endPin.IsInput;
                validLink = validLink && startPin.Type == endPin.Type;

                if (!validLink)
                {
                    ImNode::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (ImNode::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                {
                    m_NodeGraph->AddLink(EditorNodeLink{ IDGen::Generate(), startPin.ID, endPin.ID });
                }
            }
        }

        if (ImNode::QueryNewNode(&pin))
        {
            if (ImNode::AcceptNewItem())
            {
                m_NewNodePinID = pin.Get();

                ImNode::Suspend();
                ImGui::OpenPopup("New Node Context Menu");
                ImNode::Resume();
            }
        }
    }
    ImNode::EndCreate();

    if (ImNode::BeginDelete())
    {
        while (ImNode::QueryDeletedLink(&link))
        {
            if (ImNode::AcceptDeletedItem())
            {
                m_NodeGraph->RemoveLink(link.Get());
            }
        }

        while (ImNode::QueryDeletedNode(&node))
        {
            if (node.Get() == m_NodeGraph->GetOnStartNode()->GetID() || 
                node.Get() == m_NodeGraph->GetOnUpdateNode()->GetID())
            {
                ImNode::RejectDeletedItem();
            }
            else if (ImNode::AcceptDeletedItem())
            {
                m_NodeGraph->RemoveNode(node.Get());
            }
        }
    }
    ImNode::EndDelete();

    ImNode::Suspend();
    if (ImNode::ShowNodeContextMenu(&m_ContextNodeID)) ImGui::OpenPopup("Node Context Menu");
    else if (ImNode::ShowPinContextMenu(&m_ContextPinID)) ImGui::OpenPopup("Pin Context Menu");
    else if (ImNode::ShowLinkContextMenu(&m_ContextLinkID)) ImGui::OpenPopup("Link Context Menu");
    else if (ImNode::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("New Node Context Menu");
        m_ContextLinkID = 0;
    }
    ImNode::Resume();
}

void RenderPipelineEditor::RenderEditor()
{
    const auto renderNode = [](EditorNode* node) {
        node->Render();
    };
    m_NodeGraph->ForEachNode(renderNode);

    const auto renderLink = [](const EditorNodeLink& link) {
        ImNode::Link(link.ID, link.Start, link.End);
    };
    m_NodeGraph->ForEachLink(renderLink);
}

static bool IsCompatible(const EditorNodePin& nodePin, EditorNode node)
{
    if (nodePin.Type == PinType::Invalid)
        return true;

	EditorNodePin targetPin;
	for (const auto& pin : node.GetPins())
	{
		if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
		{
			targetPin = pin;
			break;
		}
	}
    return targetPin.Type != PinType::Invalid;
}

void RenderPipelineEditor::RenderContextMenus()
{
    ImNode::Suspend();

    if (ImGui::BeginPopup("New Node Context Menu"))
    {
        EditorNode* newNode = nullptr;

        EditorNodePin nodePin;
        if (m_NewNodePinID) 
            nodePin = m_NodeGraph->GetPinByID(m_NewNodePinID);

        if (ImGui::BeginMenu("Constants"))
		{
            if (IsCompatible(nodePin, BoolEditorNode{}) && ImGui::MenuItem("Bool")) newNode = new BoolEditorNode();
            if (IsCompatible(nodePin, FloatEditorNode{}) && ImGui::MenuItem("Float")) newNode = new FloatEditorNode();
            if (IsCompatible(nodePin, Float2EditorNode{}) && ImGui::MenuItem("Float2")) newNode = new Float2EditorNode();
            if (IsCompatible(nodePin, Float3EditorNode{}) && ImGui::MenuItem("Float3")) newNode = new Float3EditorNode();
            if (IsCompatible(nodePin, Float4EditorNode{}) && ImGui::MenuItem("Float4")) newNode = new Float4EditorNode();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Assign variable"))
        {
            if (IsCompatible(nodePin, AsignFloatEditorNode{}) && ImGui::MenuItem("Float")) newNode = new AsignFloatEditorNode();
            if (IsCompatible(nodePin, AsignFloat2EditorNode{}) && ImGui::MenuItem("Float2")) newNode = new AsignFloat2EditorNode();
            if (IsCompatible(nodePin, AsignFloat3EditorNode{}) && ImGui::MenuItem("Float3")) newNode = new AsignFloat3EditorNode();
            if (IsCompatible(nodePin, AsignFloat4EditorNode{}) && ImGui::MenuItem("Float4")) newNode = new AsignFloat4EditorNode();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Get variable"))
		{
			if (IsCompatible(nodePin, VarFloatEditorNode{}) && ImGui::MenuItem("Float")) newNode = new VarFloatEditorNode();
			if (IsCompatible(nodePin, VarFloat2EditorNode{}) && ImGui::MenuItem("Float2")) newNode = new VarFloat2EditorNode();
			if (IsCompatible(nodePin, VarFloat3EditorNode{}) && ImGui::MenuItem("Float3")) newNode = new VarFloat3EditorNode();
			if (IsCompatible(nodePin, VarFloat4EditorNode{}) && ImGui::MenuItem("Float4")) newNode = new VarFloat4EditorNode();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Operator"))
        {
			if (IsCompatible(nodePin, FloatBinaryOperatorEditorNode{}) && ImGui::MenuItem("Float")) newNode = new FloatBinaryOperatorEditorNode();
			if (IsCompatible(nodePin, Float2BinaryOperatorEditorNode{}) && ImGui::MenuItem("Float2")) newNode = new Float2BinaryOperatorEditorNode();
			if (IsCompatible(nodePin, Float3BinaryOperatorEditorNode{}) && ImGui::MenuItem("Float3")) newNode = new Float3BinaryOperatorEditorNode();
			if (IsCompatible(nodePin, Float4BinaryOperatorEditorNode{}) && ImGui::MenuItem("Float4")) newNode = new Float4BinaryOperatorEditorNode();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Texture"))
        {
			if (IsCompatible(nodePin, CreateTextureEditorNode{}) && ImGui::MenuItem("Create texture")) newNode = new CreateTextureEditorNode();
			if (IsCompatible(nodePin, LoadTextureEditorNode{}) && ImGui::MenuItem("Load texture")) newNode = new LoadTextureEditorNode();
			if (IsCompatible(nodePin, GetTextureEditorNode{}) && ImGui::MenuItem("Get texture")) newNode = new GetTextureEditorNode();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render"))
        {
            if (IsCompatible(nodePin, LoadShaderEditorNode{}) && ImGui::MenuItem("Load shader")) newNode = new LoadShaderEditorNode();
            if (IsCompatible(nodePin, ClearRenderTargetEditorNode{}) && ImGui::MenuItem("Clear render target")) newNode = new ClearRenderTargetEditorNode();
			if (IsCompatible(nodePin, PresentTextureEditorNode{}) && ImGui::MenuItem("Present texture")) newNode = new PresentTextureEditorNode();
			if (IsCompatible(nodePin, GetCubeMeshEditorNode{}) && ImGui::MenuItem("Get cube mesh")) newNode = new GetCubeMeshEditorNode();
			if (IsCompatible(nodePin, GetShaderEditorNode{}) && ImGui::MenuItem("Get shader")) newNode = new GetShaderEditorNode();
			if (IsCompatible(nodePin, DrawMeshEditorNode{}) && ImGui::MenuItem("Draw mesh")) newNode = new DrawMeshEditorNode();
            ImGui::EndMenu();
        }

        if (IsCompatible(nodePin, IfEditorNode{}) && ImGui::MenuItem("If condition")) newNode = new IfEditorNode();
        if (IsCompatible(nodePin, PrintEditorNode{}) && ImGui::MenuItem("Print")) newNode = new PrintEditorNode();

        if (newNode)
        {
            if (m_NewNodePinID)
            {
                const auto nodePin = m_NodeGraph->GetPinByID(m_NewNodePinID);
                EditorNodePin targetPin;
                for (const auto& pin : newNode->GetPins())
                {
                    if (pin.Type == nodePin.Type && pin.IsInput != nodePin.IsInput)
                    {
                        targetPin = pin;
                        break;
                    }
                }

                if (targetPin.Type != PinType::Invalid)
                {
                    const PinID startPin = nodePin.IsInput ? targetPin.ID : nodePin.ID;
                    const PinID endPin = nodePin.IsInput ? nodePin.ID : targetPin.ID;
                    m_NodeGraph->AddLink({ IDGen::Generate(), startPin, endPin });
                }
            }

            m_NodeGraph->AddNode(newNode);
            ImNode::SetNodePosition(newNode->GetID(), ImNode::ScreenToCanvas(ImGui::GetMousePos()));
            m_NewNodePinID = 0;
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Node Context Menu"))
    {
        ImGui::Text("Node context - TODO");
        ImGui::EndPopup();
    }

	if (ImGui::BeginPopup("Pin Context Menu"))
	{
		ImGui::Text("Pin context - TODO");
        ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Link Context Menu"))
	{
		ImGui::Text("Link context - TODO");
        ImGui::EndPopup();
	}

    ImNode::Resume();
}