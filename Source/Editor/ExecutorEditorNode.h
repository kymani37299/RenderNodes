#pragma once

#include "EditorNode.h"

class ExecutionEditorNode : public EditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	ExecutionEditorNode(const std::string& label, EditorNodeType nodeType, bool skipInput = false) :
		EditorNode(label, nodeType)
	{
		if (!skipInput) m_ExectuionPinInput = AddPin(EditorNodePin::CreateInputPin("", PinType::Execution));
		m_ExectuionPinOutput = AddPin(EditorNodePin::CreateOutputPin("", PinType::Execution));
	}

	const EditorNodePin& GetExecutionInput() const { return GetPins()[m_ExectuionPinInput]; }
	const EditorNodePin& GetExecutionOutput() const { return GetPins()[m_ExectuionPinOutput]; }

private:
	unsigned m_ExectuionPinInput;
	unsigned m_ExectuionPinOutput;
};

class OnStartEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	OnStartEditorNode() :
		ExecutionEditorNode("On start", EditorNodeType::OnStart, true) {}
};

class OnUpdateEditorNode : public ExecutionEditorNode
{
public:
	OnUpdateEditorNode() :
		ExecutionEditorNode("On update", EditorNodeType::OnUpdate, true)
	{
		AddPin(EditorNodePin::CreateOutputPin("DT", PinType::Float));
	}
};

class AsignVariableEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	AsignVariableEditorNode(EditorNodeType nodeType, PinType inputType) :
		ExecutionEditorNode("Asign " + ToString(inputType), nodeType)
	{
		m_ValuePin = AddPin(EditorNodePin::CreateInputPin("", inputType));
	}

	const EditorNodePin& GetValuePin() const { return GetPins()[m_ValuePin]; }
	const std::string& GetName() const { return m_Name; }

protected:
	virtual void RenderContent() override;

private:
	unsigned m_ValuePin;
	std::string m_Name;
};

template<EditorNodeType nodeType, PinType pinType>
class AsignVariableEditorNodeT : public AsignVariableEditorNode
{
public:
	AsignVariableEditorNodeT() :
		AsignVariableEditorNode(nodeType, pinType) {}
};

using AsignFloatEditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat, PinType::Float>;
using AsignFloat2EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat2, PinType::Float2>;
using AsignFloat3EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat3, PinType::Float3>;
using AsignFloat4EditorNode = AsignVariableEditorNodeT<EditorNodeType::AsignFloat4, PinType::Float4>;

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
	}

	const EditorNodePin& GetFloatInputPin() const { return  GetPins()[m_FloatInputPin]; }
	const EditorNodePin& GetFloat2InputPin() const { return GetPins()[m_Float2InputPin]; }
	const EditorNodePin& GetFloat3InputPin() const { return GetPins()[m_Float3InputPin]; }
	const EditorNodePin& GetFloat4InputPin() const { return GetPins()[m_Float4InputPin]; }

private:
	unsigned m_FloatInputPin;
	unsigned m_Float2InputPin;
	unsigned m_Float3InputPin;
	unsigned m_Float4InputPin;
};

class CreateTextureEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	CreateTextureEditorNode() :
		ExecutionEditorNode("Create texture", EditorNodeType::CreateTexture) {}

	const std::string& GetName() const { return m_Name; }
	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	bool IsFramebuffer() const { return m_Framebuffer; }

protected:
	virtual void RenderContent() override;

private:
	std::string m_Name = "";
	int m_Width = 1;
	int m_Height = 1;
	bool m_Framebuffer = false;
};

class NameAndPathExecutionEditorNode : public ExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	NameAndPathExecutionEditorNode(const std::string& nodeLabel, EditorNodeType nodeType) :
		ExecutionEditorNode(nodeLabel, nodeType) {}

	const std::string& GetName() const { return m_Name; }
	const std::string& GetPath() const { return m_Path; }

protected:
	virtual void RenderContent() override;

private:
	std::string m_Name = "";
	std::string m_Path = "";
};

class LoadTextureEditorNode : public NameAndPathExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	LoadTextureEditorNode() :
		NameAndPathExecutionEditorNode("Load texture", EditorNodeType::LoadTexture) {}
};

class LoadShaderEditorNode : public NameAndPathExecutionEditorNode
{
	SERIALIZEABLE_EDITOR_NODE();
public:
	LoadShaderEditorNode() :
		NameAndPathExecutionEditorNode("Load shader", EditorNodeType::LoadShader) {}
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
	}

	const EditorNodePin& GetFrameBufferPin() const { return GetPins()[m_FramebufferPin]; }
	const EditorNodePin& GetMeshPin() const { return GetPins()[m_MeshPin]; }
	const EditorNodePin& GetShaderPin() const { return GetPins()[m_ShaderPin]; }

private:
	unsigned m_FramebufferPin;
	unsigned m_ShaderPin;
	unsigned m_MeshPin;
};