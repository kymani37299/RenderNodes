#pragma once

#include <inttypes.h>

#include "../Common.h"
#include "../NodeGraph/NodeGraph.h"
#include "../NodeGraph/VariablePool.h"
#include "Drawing/EditorContextMenu.h"
#include "Drawing/EditorDialog.h"

class NodeGraphCommandExecutor;

class RenderPipelineEditor
{
public:
    RenderPipelineEditor(NodeGraphCommandExecutor* commandExecutor);

    ~RenderPipelineEditor();
	void InitializeDefaultNodePositions();

    void Render();
    void HandleKeyPressed(int key, int mods);

    void Unload();
    void Load(NodeGraph* nodeGraph, VariablePool* variablePool);

    void LoadNodePositions();
    void SaveNodePositions();

    NodeGraphCommandExecutor* GetCommandExecutor() const { return m_CommandExecutor.get(); }

private:
    void UpdateEditor();
    void RenderEditor();

    void RenderContextMenus();
    void RenderNodePopups();
    
    void RenderVariableWindow();
private:
    ImNode::EditorContext* m_EditorContext = nullptr;

    Ptr<NewNodeContextMenu> m_NewNodeMenu;
    Ptr<NodeContextMenu> m_NodeMenu;
    Ptr<LinkContextMenu> m_LinkMenu;
    Ptr<PinContextMenu> m_PinMenu;

    Ptr<EditorDialog> m_CurrentDialog;

protected:
    bool m_CustomNodeEditor = false;
    NodeGraph* m_NodeGraph = nullptr;
    VariablePool* m_VariablePool = nullptr;
    Ptr<NodeGraphCommandExecutor> m_CommandExecutor;
};

class CustomNodePipelineEditor : public RenderPipelineEditor
{
public:
    CustomNodePipelineEditor(VariablePool* variablePool); // Init for new node
    CustomNodePipelineEditor(CustomEditorNode* node, VariablePool* variablePool); // Init for edit node
    
    ~CustomNodePipelineEditor();

    bool CanCreateNode() const;
    CustomEditorNode* CreateNode();
    void RewriteNode();

    std::string& GetNameRef() { return m_Name; }
    void LoadPositionsIfNeeded();

    bool EditMode() const { return m_Node != nullptr; }

private:
	std::string m_Name = "";

	CustomEditorNode* m_Node = nullptr;
	bool m_ShouldLoadPositions = false;
};
