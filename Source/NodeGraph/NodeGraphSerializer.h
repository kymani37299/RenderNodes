#pragma once

#include "../Common.h"

#include <string>
#include <fstream>
#include <vector>

class NodeGraph;
class EditorNode;
class ExecutionEditorNode;
struct EditorNodeLink;
struct EditorNodePin;

class NodeGraphSerializer
{
	static constexpr unsigned VERSION = 1;

public:
	void Serialize(const std::string& path, const NodeGraph& nodeGraph);
	bool Deserialize(const std::string& path, NodeGraph& nodeGraph);

	// Hack since I need to use it in lambda
public:
	void WriteNode(EditorNode* node);
	void WriteLink(const EditorNodeLink& link);
	
private:
	// Writes
	void WriteNodeList();
	void WriteLinkList();
	void WritePinList(const std::vector<EditorNodePin>& pins);
	void WritePin(const EditorNodePin& pin);
	void WriteExecutionNodeDetails(ExecutionEditorNode* node);

	// Reads
	void ReadNodeList();
	void ReadLinkList();
	std::vector<EditorNodePin> ReadPinList();
	EditorNode* ReadNode();
	EditorNodeLink ReadLink();
	EditorNodePin ReadPin();
	void ReadExecutionNode(ExecutionEditorNode* exNode);

	// Tmp hack since nodes will self initialize some data
	void ClearNode(EditorNode* node);

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
	unsigned m_Version = 1;

	const NodeGraph* m_NodeReadGraph = nullptr;
	NodeGraph* m_NodeWriteGraph = nullptr;

	std::ifstream m_Input;
	std::ofstream m_Output;

	bool m_OperationSuccess;
};