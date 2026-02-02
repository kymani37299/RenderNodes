#pragma once

#include "EditorNode.h"

#include "../App/App.h"

class ExecutionEditorNode : public EditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	ExecutionEditorNode(const std::string& label, EditorNodeType nodeType, bool skipInput = false, bool skipOutput = false) :
		EditorNode(label, nodeType)
	{
		if (!skipInput) m_ExectuionPinInput = AddPin(EditorNodePin::CreateInputPin("", PinType::Execution));
		if(!skipOutput) m_ExectuionPinOutput = AddPin(EditorNodePin::CreateOutputPin("", PinType::Execution));
	}

	const EditorNodePin& GetExecutionInput() const { return GetPins()[m_ExectuionPinInput]; }
	const EditorNodePin& GetExecutionOutput() const { return GetPins()[m_ExectuionPinOutput]; }

private:
	unsigned m_ExectuionPinInput;
	unsigned m_ExectuionPinOutput;
};

class PinEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();

public:
	PinEditorNode(bool isInput, PinType pinType) :
		ExecutionEditorNode(ToString(pinType), EditorNodeType::Pin, true, true)
	{
		if (isInput) m_Pin = AddPin(EditorNodePin::CreateInputPin("", pinType));
		else m_Pin = AddPin(EditorNodePin::CreateOutputPin("", pinType));
	}

	EditorNode* Clone() const override
	{
		PinEditorNode* node = new PinEditorNode{ GetPin().IsInput, GetPin().Type };
		node->m_Name = m_Name;
		return node;
	}

	const EditorNodePin& GetPin() const { return GetPins()[m_Pin]; }
	const std::string& GetName() const { return m_Name; }

protected:
	virtual void RenderContent();

private:
	std::string m_Name;
	unsigned m_Pin;
};

class NodeGraph;

class CustomEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:

	CustomEditorNode(NodeGraph* parentGraph, const std::string& name, NodeGraph* nodeGraph, bool regneratePins = true);

	EditorNode* Clone() const override
	{
		return new CustomEditorNode(m_ParentGraph, m_Name, m_NodeGraph.get());
	}

	EditorNode* Instance(NodeGraph* parentGraph) const
	{
		return new CustomEditorNode{ parentGraph, m_Name, m_NodeGraph.get() };
	}

	const std::string& GetName() const { return m_Name; }

	NodeID GetPinNode(PinID pinID) const 
	{ 
		for (const auto customPin : GetCustomPins())
		{
			if (customPin.ID == pinID)
			{
				ASSERT(customPin.LinkedNode != 0);
				return customPin.LinkedNode;
			}
		}
		ASSERT(0);
		return 0;
	}

	PinID GetPin(NodeID nodeID) const 
	{
		for (const auto customPin : GetCustomPins())
		{
			if (customPin.LinkedNode == nodeID)
			{
				return customPin.ID;
			}
		}
		return 0;
	}

	NodeGraph* GetNodeGraph() const { return m_NodeGraph.get(); }

	void RegeneratePins();

private:
	std::string m_Name;
	SharedPtr<NodeGraph> m_NodeGraph;
	NodeGraph* m_ParentGraph;
};

class OnStartEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	OnStartEditorNode() :
		ExecutionEditorNode("On start", EditorNodeType::OnStart, true) {}

	EditorNode* Clone() const override
	{
		return new OnStartEditorNode{};
	}
};

class OnUpdateEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	OnUpdateEditorNode() :
		ExecutionEditorNode("On update", EditorNodeType::OnUpdate, true)
	{
		AddPin(EditorNodePin::CreateOutputPin("DT", PinType::Float));
	}

	EditorNode* Clone() const override
	{
		return new OnUpdateEditorNode{};
	}
};

class InputExecutionEditorNode : public ExecutionEditorNode, IInputListener
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	InputExecutionEditorNode(const std::string& label, EditorNodeType type) :
		ExecutionEditorNode(label, type, true)
	{
	}

	InputExecutionEditorNode(const std::string& label, EditorNodeType type, int key, int mods) :
		ExecutionEditorNode(label, type, true),
		m_Key(key),
		m_Mods(mods)
	{
	}

	void RenderContent() override;

	int GetKey() const { return m_Key; }
	int GetMods() const { return m_Mods; }

	void OnKeyReleased(int key, int mods) override;

private:
	bool m_ListeningToInput = false;

	int m_Key = 0;
	int m_Mods = 0;

	std::string m_InputText;
};

class OnKeyPressedEditorNode : public InputExecutionEditorNode
{
public:
	OnKeyPressedEditorNode() :
		InputExecutionEditorNode("On key pressed", EditorNodeType::OnKeyPressed) {}

	OnKeyPressedEditorNode(int key, int mods) :
		InputExecutionEditorNode("On key pressed", EditorNodeType::OnKeyPressed, key, mods) {}

	EditorNode* Clone() const override { return new OnKeyPressedEditorNode{GetKey(), GetMods()}; }
};

class OnKeyReleasedEditorNode : public InputExecutionEditorNode
{
public:
	OnKeyReleasedEditorNode() :
		InputExecutionEditorNode("On key released", EditorNodeType::OnKeyReleased) {}

	OnKeyReleasedEditorNode(int key, int mods) :
		InputExecutionEditorNode("On key released", EditorNodeType::OnKeyReleased, key, mods) {}

	EditorNode* Clone() const override { return new OnKeyReleasedEditorNode{ GetKey(), GetMods() }; }
};

class OnKeyDownEditorNode : public InputExecutionEditorNode
{
public:
	OnKeyDownEditorNode() :
		InputExecutionEditorNode("On key down", EditorNodeType::OnKeyDown) {}

	OnKeyDownEditorNode(int key, int mods) :
		InputExecutionEditorNode("On key down", EditorNodeType::OnKeyDown, key, mods) {}

	EditorNode* Clone() const override { return new OnKeyDownEditorNode{ GetKey(), GetMods() }; }
};

class AsignVariableEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	AsignVariableEditorNode(VariableID id, VariableType varType) :
		ExecutionEditorNode("Asign ", EditorNodeType::AsignVariable),
		m_VarID(id),
		m_VarType(varType)
	{
		PinType pinType = ToPinType(varType);
		m_ValuePin = AddPin(EditorNodePin::CreateInputPin("Value", pinType));
	}

	void RefreshLabel(const VariablePool& variablePool)
	{
		ChangeLabel("Asign " + variablePool.GetRef(m_VarID).Name);
	}

	VariableID GetVariableID() const { return m_VarID; }
	const EditorNodePin& GetValuePin() const { return GetPins()[m_ValuePin]; }

	EditorNode* Clone() const
	{
		return new AsignVariableEditorNode{ m_VarID, m_VarType };
	}
private:
	VariableID m_VarID;
	VariableType m_VarType;
	
	unsigned m_ValuePin;
};

class IfEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	IfEditorNode() :
		ExecutionEditorNode("If", EditorNodeType::If)
	{
		m_ExectuionPinElse = AddPin(EditorNodePin::CreateOutputPin("ELSE", PinType::Execution));
		m_ConditionPin = AddPin(EditorNodePin::CreateInputPin("Condition", PinType::Bool));
	}

	EditorNode* Clone() const override
	{
		return new IfEditorNode{};
	}

	const EditorNodePin& GetExecutionElse() const { return GetPins()[m_ExectuionPinElse]; }
	const EditorNodePin& GetConditionPin() const { return GetPins()[m_ConditionPin]; }

private:
	unsigned m_ExectuionPinElse;
	unsigned m_ConditionPin;
};

class PrintEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	PrintEditorNode() :
		ExecutionEditorNode("Print node", EditorNodeType::Print)
	{
		m_InputPin = AddPin(EditorNodePin::CreateInputPin("Value", PinType::Any));
	}

	EditorNode* Clone() const override
	{
		return new PrintEditorNode{};
	}

	const EditorNodePin& GetInputPin() const { return GetPins()[m_InputPin]; }

private:
	unsigned m_InputPin;
};

class ClearRenderTargetEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	ClearRenderTargetEditorNode() :
		ExecutionEditorNode("Clear render target", EditorNodeType::ClearRenderTarget)
	{
		m_TargetTexturePin = AddPin(EditorNodePin::CreateInputPin("Texture", PinType::Texture));
		m_ClearColorPin = AddPin(EditorNodePin::CreateInputPin("Clear color", PinType::Float4));
	}

	EditorNode* Clone() const override
	{
		return new ClearRenderTargetEditorNode{};
	}

	const EditorNodePin& GetTargeteTexturePin() const { return GetPins()[m_TargetTexturePin]; }
	const EditorNodePin& GetClearColorPin() const { return GetPins()[m_ClearColorPin]; }

private:
	unsigned m_TargetTexturePin;
	unsigned m_ClearColorPin;
};

class PresentTextureEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	PresentTextureEditorNode() :
		ExecutionEditorNode("Present texture", EditorNodeType::PresentTexture)
	{
		m_InputTexturePin = AddPin(EditorNodePin::CreateInputPin("Texture", PinType::Texture));
	}

	EditorNode* Clone() const override
	{
		return new PresentTextureEditorNode{};
	}

	const EditorNodePin& GetTexturePin() const { return GetPins()[m_InputTexturePin]; }

private:
	unsigned m_InputTexturePin;
};

class DrawMeshEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	DrawMeshEditorNode():
		ExecutionEditorNode("Draw mesh", EditorNodeType::DrawMesh)
	{
		m_FramebufferPin = AddPin(EditorNodePin::CreateInputPin("Framebuffer", PinType::Texture));
		m_ShaderPin = AddPin(EditorNodePin::CreateInputPin("Shader", PinType::Shader));
		m_MeshPin = AddPin(EditorNodePin::CreateInputPin("Mesh", PinType::Mesh));
		m_BindTablePin = AddPin(EditorNodePin::CreateInputPin("BindTable", PinType::BindTable));
		m_RenderStatePin = AddPin(EditorNodePin::CreateInputPin("RenderState", PinType::RenderState));
	}

	EditorNode* Clone() const override
	{
		return new DrawMeshEditorNode{};
	}

	const EditorNodePin& GetFrameBufferPin() const { return GetPins()[m_FramebufferPin]; }
	const EditorNodePin& GetMeshPin() const { return GetPins()[m_MeshPin]; }
	const EditorNodePin& GetShaderPin() const { return GetPins()[m_ShaderPin]; }
	const EditorNodePin& GetBindTablePin() const { return GetPins()[m_BindTablePin]; }
	const EditorNodePin& GetRenderStatePin() const { return GetPins()[m_RenderStatePin]; }

private:
	unsigned m_FramebufferPin;
	unsigned m_ShaderPin;
	unsigned m_MeshPin;

	unsigned m_BindTablePin;
	unsigned m_RenderStatePin;
};

class ForEachSceneObjectEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	ForEachSceneObjectEditorNode():
		ExecutionEditorNode("For each scene object", EditorNodeType::ForEachSceneObject)
	{
		m_LoopPin = AddPin(EditorNodePin::CreateOutputPin("Loop", PinType::Execution));
		m_ScenePin = AddPin(EditorNodePin::CreateInputPin("Scene", PinType::Scene));
		m_SceneObjectPin = AddPin(EditorNodePin::CreateOutputPin("Scene object[*]", PinType::SceneObject));
	}

	EditorNode* Clone() const override
	{
		return new ForEachSceneObjectEditorNode{};
	}

	const EditorNodePin& GetLoopPin() const { return GetPins()[m_LoopPin]; }
	const EditorNodePin& GetScenePin() const { return GetPins()[m_ScenePin]; }
	const EditorNodePin& GetSceneObjectPin() const { return GetPins()[m_SceneObjectPin]; }

private:
	unsigned m_SceneObjectPin;
	unsigned m_LoopPin;
	unsigned m_ScenePin;
};