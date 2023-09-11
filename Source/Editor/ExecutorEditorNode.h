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
	AsignVariableEditorNode(EditorNodeType nodeType, PinType inputType) :
		ExecutionEditorNode("Asign " + ToString(inputType), nodeType)
	{
		m_ValuePin = AddPin(EditorNodePin::CreateInputPin("Value", inputType));
		m_NamePin = AddPin(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
	}

	const EditorNodePin& GetValuePin() const { return GetPins()[m_ValuePin]; }
	const EditorNodePin& GetNamePin() const { return GetPins()[m_NamePin]; }

private:
	unsigned m_ValuePin;
	unsigned m_NamePin;
};

template<EditorNodeType nodeType, PinType pinType>
class AsignVariableEditorNodeT : public AsignVariableEditorNode
{
public:
	AsignVariableEditorNodeT() :
		AsignVariableEditorNode(nodeType, pinType) {}

	EditorNode* Clone() const override
	{
		return new AsignVariableEditorNodeT<nodeType, pinType>{};
	}
};

using AsignBoolEditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignBool, PinType::Bool>;
using AsignIntEditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignInt, PinType::Int>;
using AsignFloatEditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat, PinType::Float>;
using AsignFloat2EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat2, PinType::Float2>;
using AsignFloat3EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat3, PinType::Float3>;
using AsignFloat4EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat4, PinType::Float4>;
using AsignFloat4x4EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat4x4, PinType::Float4x4>;

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
		m_FloatInputPin = AddPin(EditorNodePin::CreateInputPin("Float", PinType::Float));
		m_Float2InputPin = AddPin(EditorNodePin::CreateInputPin("Float2", PinType::Float2));
		m_Float3InputPin = AddPin(EditorNodePin::CreateInputPin("Float3", PinType::Float3));
		m_Float4InputPin = AddPin(EditorNodePin::CreateInputPin("Float4", PinType::Float4));
		m_IntInputPin = AddPin(EditorNodePin::CreateInputPin("Int", PinType::Int));
		m_BoolInputPin = AddPin(EditorNodePin::CreateInputPin("Bool", PinType::Bool));
		m_StringInputPin = AddPin(EditorNodePin::CreateInputPin("String", PinType::String));
	}

	EditorNode* Clone() const override
	{
		return new PrintEditorNode{};
	}

	const EditorNodePin& GetFloatInputPin() const { return  GetPins()[m_FloatInputPin]; }
	const EditorNodePin& GetFloat2InputPin() const { return GetPins()[m_Float2InputPin]; }
	const EditorNodePin& GetFloat3InputPin() const { return GetPins()[m_Float3InputPin]; }
	const EditorNodePin& GetFloat4InputPin() const { return GetPins()[m_Float4InputPin]; }
	const EditorNodePin& GetIntInputPin() const { return GetPins()[m_IntInputPin]; }
	const EditorNodePin& GetBoolInputPin() const { return GetPins()[m_BoolInputPin]; }
	const EditorNodePin& GetStringInputPin() const { return GetPins()[m_StringInputPin]; }

private:
	unsigned m_FloatInputPin;
	unsigned m_Float2InputPin;
	unsigned m_Float3InputPin;
	unsigned m_Float4InputPin;
	unsigned m_IntInputPin;
	unsigned m_BoolInputPin;
	unsigned m_StringInputPin;
};

class CreateTextureEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	CreateTextureEditorNode() :
		ExecutionEditorNode("Create texture", EditorNodeType::CreateTexture) 
	{
		m_NamePin = AddPin(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
		m_WidthPin = AddPin(EditorNodePin::CreateConstantInputPin("Width", PinType::Int));
		m_HeightPin = AddPin(EditorNodePin::CreateConstantInputPin("Height", PinType::Int));
		m_FramebufferPin = AddPin(EditorNodePin::CreateConstantInputPin("Framebuffer", PinType::Bool));
		m_DepthStencilPin = AddPin(EditorNodePin::CreateConstantInputPin("DepthStencil", PinType::Bool));
	}

	EditorNode* Clone() const override
	{
		return new CreateTextureEditorNode{};
	}

	const EditorNodePin& GetNamePin() const { return GetPins()[m_NamePin]; }
	const EditorNodePin& GetWidthPin() const { return GetPins()[m_WidthPin]; }
	const EditorNodePin& GetHeightPin() const { return GetPins()[m_HeightPin]; }
	const EditorNodePin& GetFramebufferPin() const { return GetPins()[m_FramebufferPin]; }
	const EditorNodePin& GetDepthStencilPin() const { return GetPins()[m_DepthStencilPin]; }
	
private:
	unsigned m_NamePin;
	unsigned m_WidthPin;
	unsigned m_HeightPin;
	unsigned m_FramebufferPin;
	unsigned m_DepthStencilPin;
};

class NameAndPathExecutionEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	NameAndPathExecutionEditorNode(const std::string& nodeLabel, EditorNodeType nodeType) :
		ExecutionEditorNode(nodeLabel, nodeType) 
	{
		m_NamePin = AddPin(EditorNodePin::CreateConstantInputPin("Name", PinType::String));
	}

	const EditorNodePin& GetNamePin() const { return GetPins()[m_NamePin]; }
	const std::string& GetPath() const { return m_Path; }

protected:
	virtual void RenderContent() override;

private:
	unsigned m_NamePin;

protected: // Should be private
	std::string m_Path = "";
};

class LoadTextureEditorNode : public NameAndPathExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	LoadTextureEditorNode() :
		NameAndPathExecutionEditorNode("Load texture", EditorNodeType::LoadTexture) {}

	EditorNode* Clone() const override
	{
		LoadTextureEditorNode* node = new LoadTextureEditorNode{};
		node->m_Path = m_Path;
		return node;
	}
};

class LoadShaderEditorNode : public NameAndPathExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	LoadShaderEditorNode() :
		NameAndPathExecutionEditorNode("Load shader", EditorNodeType::LoadShader) {}

	EditorNode* Clone() const override
	{
		LoadShaderEditorNode* node = new LoadShaderEditorNode{};
		node->m_Path = m_Path;
		return node;
	}
};

class LoadMeshEditorNode : public NameAndPathExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	LoadMeshEditorNode() :
		NameAndPathExecutionEditorNode("Load mesh", EditorNodeType::LoadMesh) {}

	EditorNode* Clone() const override
	{
		LoadMeshEditorNode* node = new LoadMeshEditorNode{};
		node->m_Path = m_Path;
		return node;
	}
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