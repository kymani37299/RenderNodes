#pragma once

#include "../Common.h"
#include "../IDGen.h"

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
	static constexpr unsigned VERSION = 6;

public:
	void Serialize(const std::string& path, const NodeGraph& nodeGraph);
	UniqueID Deserialize(const std::string& path, NodeGraph& nodeGraph, std::vector<CustomEditorNode*>& customNodes);

	// Hack since I need to use it in lambda
public:
	void WriteNode(EditorNode* node);
	void WriteLink(const EditorNodeLink& link);
	
private:
	// Writes
	void WriteNodeGraph(const NodeGraph& nodeGraph);
	void WriteNodeList(const NodeGraph& nodeGraph);
	void WriteLinkList(const NodeGraph& nodeGraph);
	void WriteNodePositions(const NodeGraph& nodeGraph);
	void WriteCustomNodeList();
	void WriteCustomNode(CustomEditorNode* node);

	// Reads
	void ReadNodeGraph(NodeGraph& nodeGraph, const std::vector<CustomEditorNode*>& customNodes);
	void ReadNodeList(NodeGraph& nodeGraph);
	void ReadNodePositions(NodeGraph& nodeGraph);
	std::vector<CustomEditorNode*> ReadCustomNodeList();
	void ResolveCustomNodes(NodeGraph& nodeGraph, const std::vector<CustomEditorNode*>& customNodes);
	void ReadLinkList(NodeGraph& nodeGraph);
	EditorNode* ReadNode();
	EditorNodeLink ReadLink();
	CustomEditorNode* ReadCustomNode(const std::vector<CustomEditorNode*>& customNodes);
	
	void ReadFloatNode(FloatNEditorNode* floatNode);
	void ReadBinaryOperatorNode(BinaryOperatorEditorNode* binOpNode);
	void ReadNamePathPathNode(NameAndPathExecutionEditorNode* nameAndPathNode);

private:
	template<typename T>
	void WriteAttribute(const std::string& name, const T& value)
	{
		m_Output << name << " : " << value << "\n";
	}

	void WriteToken(const std::string& token);
	void EatToken(const std::string& token);

	std::string ReadAttribute(const std::string& name);
	int ReadIntAttr(const std::string& name);
	float ReadFloatAttr(const std::string& name);
	char ReadCharAttr(const std::string& name);
	bool ReadBoolAttr(const std::string& name);
	std::string ReadStrAttr(const std::string& name);

private:
	bool m_UseTokens = false;
	unsigned m_Version = VERSION;

	std::ifstream m_Input;
	std::ofstream m_Output;

	bool m_OperationSuccess;
};