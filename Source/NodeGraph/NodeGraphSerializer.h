#pragma once

#include "../Common.h"
#include "../IDGen.h"

#include "../NodeGraph/VariablePool.h"

#include <string>
#include <fstream>
#include <vector>

class NodeGraph;
class EditorNode;
class ExecutionEditorNode;
class AsignVariableEditorNode;
class VariableEditorNode;
class FloatNEditorNode;
class BinaryOperatorEditorNode;
class NameAndPathExecutionEditorNode;
class CustomEditorNode;
struct EditorNodeLink;
struct EditorNodePin;

class NodeGraphSerializer
{
	static constexpr unsigned VERSION = 9;

public:
	void Serialize(const std::string& path, const NodeGraph& nodeGraph, const VariablePool& variablePool);
	UniqueID Deserialize(const std::string& path, NodeGraph& nodeGraph, VariablePool& variablePool, std::vector<CustomEditorNode*>& customNodes);

	// Hack since I need to use it in lambda
public:
	void WriteNode(EditorNode* node);
	void WriteLink(const EditorNodeLink& link);
	
private:
	// Writes
	void WriteVariablePool(const VariablePool& variablePool);
	void WriteVariable(VariableID id, const Variable& variable);
	void WriteNodeGraph(const NodeGraph& nodeGraph);
	void WriteNodeList(const NodeGraph& nodeGraph);
	void WriteLinkList(const NodeGraph& nodeGraph);
	void WriteNodePositions(const NodeGraph& nodeGraph);
	void WriteCustomNodeList();
	void WriteCustomNode(CustomEditorNode* node);
	void WritePinList(const std::vector<EditorNodePin>& pins, bool custom);
	void WritePin(const EditorNodePin& pin, bool custom);

	// Reads
	void ReadVariablePool(VariablePool& variablePool);
	void ReadNodeGraph(NodeGraph& nodeGraph, const std::vector<CustomEditorNode*>& customNodes);
	void ReadNodeList(NodeGraph& nodeGraph, const std::vector<CustomEditorNode*>& customNodes);
	void ReadNodePositions(NodeGraph& nodeGraph);
	void CleanupDeprecatedNodes(NodeGraph& nodeGraph);
	std::vector<CustomEditorNode*> ReadCustomNodeList();
	void ReadLinkList(NodeGraph& nodeGraph);
	EditorNode* ReadNode(NodeGraph& nodeGraph, const std::vector<CustomEditorNode*>& customNodes);
	EditorNodeLink ReadLink();
	CustomEditorNode* ReadCustomNode(const std::vector<CustomEditorNode*>& customNodes);
	
	void ReadFloatNode(FloatNEditorNode* floatNode);
	void ReadBinaryOperatorNode(BinaryOperatorEditorNode* binOpNode);

private:
	template<typename T>
	void WriteAttribute(const std::string& name, const T& value)
	{
		m_Output << name << " : " << value << "\n";
	}

	template<>
	void WriteAttribute(const std::string& name, const bool& value)
	{
		WriteAttribute(name, value ? 1 : 0);
	}

	template<>
	void WriteAttribute(const std::string& name, const Float2& value)
	{
		WriteAttribute(name + ".X", value.x);
		WriteAttribute(name + ".Y", value.y);
	}

	template<>
	void WriteAttribute(const std::string& name, const Float3& value)
	{
		WriteAttribute(name + ".X", value.x);
		WriteAttribute(name + ".Y", value.y);
		WriteAttribute(name + ".Z", value.z);
	}

	template<>
	void WriteAttribute(const std::string& name, const Float4& value)
	{
		WriteAttribute(name + ".X", value.x);
		WriteAttribute(name + ".Y", value.y);
		WriteAttribute(name + ".Z", value.z);
		WriteAttribute(name + ".W", value.w);
	}

	template<>
	void WriteAttribute(const std::string& name, const Float4x4& value)
	{
		WriteAttribute(name + ".00", value[0][0]);
		WriteAttribute(name + ".01", value[0][1]);
		WriteAttribute(name + ".02", value[0][2]);
		WriteAttribute(name + ".10", value[1][0]);
		WriteAttribute(name + ".11", value[1][1]);
		WriteAttribute(name + ".12", value[1][2]);
		WriteAttribute(name + ".20", value[2][0]);
		WriteAttribute(name + ".21", value[2][1]);
		WriteAttribute(name + ".22", value[2][2]);
	}

	template<>
	void WriteAttribute(const std::string& name, const TextureData& value)
	{
		WriteAttribute(name + ".Path", value.Path);
		WriteAttribute(name + ".Width", value.Width);
		WriteAttribute(name + ".Height", value.Height);
		WriteAttribute(name + ".Framebuffer", value.Framebuffer);
		WriteAttribute(name + ".DepthStencil", value.DepthStencil);
	}

	template<>
	void WriteAttribute(const std::string& name, const ShaderData& value)
	{
		WriteAttribute(name + ".Path", value.Path);
	}

	template<>
	void WriteAttribute(const std::string& name, const SceneData& value)
	{
		WriteAttribute(name + ".Path", value.Path);
	}

	void WriteToken(const std::string& token);
	void EatToken(const std::string& token);

	std::string ReadAttribute(const std::string& name);
	int ReadIntAttr(const std::string& name);
	float ReadFloatAttr(const std::string& name);
	char ReadCharAttr(const std::string& name);
	bool ReadBoolAttr(const std::string& name);
	std::string ReadStrAttr(const std::string& name);
	Float2 ReadFloat2Attr(const std::string& name);
	Float3 ReadFloat3Attr(const std::string& name);
	Float4 ReadFloat4Attr(const std::string& name);
	Float4x4 ReadFloat4x4Attr(const std::string& name);
	TextureData ReadTextureDataAttr(const std::string& name);
	SceneData ReadSceneDataAttr(const std::string& name);
	ShaderData ReadShaderDataAttr(const std::string& name);

private:
	bool m_UseTokens = false;
	unsigned m_Version = VERSION;

	std::ifstream m_Input;
	std::ofstream m_Output;

	bool m_OperationSuccess;
};