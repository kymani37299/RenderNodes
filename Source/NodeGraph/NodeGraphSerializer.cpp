#include "NodeGraphSerializer.h"

#include <sstream>

#include "../Common.h"
#include "NodeGraph/NodeGraph.h"
#include "Editor/RenderPipelineEditor.h"

// #define ENABLE_TOKEN_VERIFICATION
// #define ASSERT_ON_LOAD_FAILED

#ifdef ASSERT_ON_LOAD_FAILED
#define LOAD_ASSERT() ASSERT(0)
#else
#define LOAD_ASSERT()
#endif // ASSERT_ON_LOAD_FAILED

static const std::string BEGIN_NODE_LIST_TOKEN = "BEGIN_NODE_LIST";
static const std::string END_NODE_LIST_TOKEN = "END_NODE_LIST";
static const std::string BEGIN_LINK_LIST_TOKEN = "BEGIN_LINK_LIST";
static const std::string END_LINK_LIST_TOKEN = "END_LINK_LIST";
static const std::string BEGIN_NODE_TOKEN = "BEGIN_NODE";
static const std::string END_NODE_TOKEN = "END_NODE";
static const std::string BEGIN_LINK_TOKEN = "BEGIN_LINK";
static const std::string END_LINK_TOKEN = "END_LINK";
static const std::string BEGIN_PIN_LIST_TOKEN = "BEGIN_PIN_LIST";
static const std::string END_PIN_LIST_TOKEN = "END_PIN_LIST";

void NodeGraphSerializer::Serialize(const std::string& path, const NodeGraph& nodeGraph)
{
	m_NodeReadGraph = &nodeGraph;
	m_Output.open(path);

#ifdef ENABLE_TOKEN_VERIFICATION
	m_UseTokens = true;
#else
	m_UseTokens = false;
#endif

	WriteAttribute("Version", VERSION);
	WriteAttribute("UseTokens", m_UseTokens);
	WriteAttribute("FirstID", IDGen::Generate());
	WriteNodeList();
	WriteLinkList();

	m_Output.close();

	m_NodeReadGraph = nullptr;
}

UniqueID NodeGraphSerializer::Deserialize(const std::string& path, NodeGraph& nodeGraph)
{
	m_OperationSuccess = true;
	m_NodeWriteGraph = &nodeGraph;

	m_Input = std::ifstream{ path };
	if (!m_Input.is_open()) return 0;

	m_Version = ReadIntAttr("Version");
	m_UseTokens = ReadIntAttr("UseTokens");
	unsigned firstID = ReadIntAttr("FirstID");

	ReadNodeList();
	ReadLinkList();

	m_Input.close();
	m_NodeWriteGraph = nullptr;

	return m_OperationSuccess ? firstID : 0;
}

//////////////////////////////
///			WRITING			//
//////////////////////////////

#pragma region(WRITING)

void NodeGraphSerializer::WriteNodeList()
{
	WriteToken(BEGIN_NODE_LIST_TOKEN);
	WriteAttribute("Count", m_NodeReadGraph->GetNodeCount());

	const auto fn = [this](EditorNode* node) { this->WriteNode(node); };
	m_NodeReadGraph->ForEachNode(fn);

	WriteToken(END_NODE_LIST_TOKEN);
}

void NodeGraphSerializer::WriteLinkList()
{
	WriteToken(BEGIN_LINK_LIST_TOKEN);
	WriteAttribute("Count", m_NodeReadGraph->GetLinkCount());

	const auto fn = [this](const EditorNodeLink& link) { this->WriteLink(link); };
	m_NodeReadGraph->ForEachLink(fn);

	WriteToken(END_LINK_LIST_TOKEN);
}

void NodeGraphSerializer::WritePinList(const std::vector<EditorNodePin>& pins)
{
	WriteToken(BEGIN_PIN_LIST_TOKEN);
	WriteAttribute("Count", pins.size());
	for (const EditorNodePin& pin : pins) WriteAttribute("ID", pin.ID);
	WriteToken(END_PIN_LIST_TOKEN);
}

void NodeGraphSerializer::WriteNode(EditorNode* node)
{
	WriteToken(BEGIN_NODE_TOKEN);

	WriteAttribute("Type", EnumToInt(node->m_Type));

	switch (node->GetType())
	{
	case EditorNodeType::Invalid:
		ASSERT(0);
		break;
	case EditorNodeType::Bool:
	{
		BoolEditorNode* boolNode = static_cast<BoolEditorNode*>(node);
		WriteAttribute("Value", boolNode->m_Value ? 1 : 0);
	} break;
	case EditorNodeType::Float:
	case EditorNodeType::Float2:
	case EditorNodeType::Float3:
	case EditorNodeType::Float4:
	{
		FloatNEditorNode* floatNode = static_cast<FloatEditorNode*>(node);
		WriteAttribute("NumFloats", floatNode->m_NumValues);
		WriteAttribute("Pin", floatNode->m_Pin);
		for (unsigned i = 0; i < floatNode->m_NumValues; i++)
		{
			WriteAttribute("Value" + std::to_string(i), floatNode->m_Values[i]);

		}
	} break;
	case EditorNodeType::FloatBinaryOperator:
	case EditorNodeType::Float2BinaryOperator:
	case EditorNodeType::Float3BinaryOperator:
	case EditorNodeType::Float4BinaryOperator:
	{
		BinaryOperatorEditorNode* binaryOperator = static_cast<BinaryOperatorEditorNode*>(node);
		WriteAttribute("A", binaryOperator->m_Apin);
		WriteAttribute("B", binaryOperator->m_Bpin);
		WriteAttribute("OP", binaryOperator->m_Op);
	} break;
	case EditorNodeType::If:
	{
		IfEditorNode* ifNode = static_cast<IfEditorNode*>(node);
		WriteExecutionNodeDetails(ifNode);
		WriteAttribute("Condition", ifNode->m_ConditionPin);
		WriteAttribute("ExecutionElse", ifNode->m_ExectuionPinElse);
	} break;
	case EditorNodeType::Print:
	{
		PrintEditorNode* printNode = static_cast<PrintEditorNode*>(node);
		WriteExecutionNodeDetails(printNode);
		WriteAttribute("FloatInput", printNode->m_FloatInputPin);
		WriteAttribute("Float2Input", printNode->m_Float2InputPin);
		WriteAttribute("Float3Input", printNode->m_Float3InputPin);
		WriteAttribute("Float4Input", printNode->m_Float4InputPin);
	} break;
	case EditorNodeType::OnUpdate:
	case EditorNodeType::OnStart:
	{
		ExecutionEditorNode* exNode = static_cast<ExecutionEditorNode*>(node);
		WriteExecutionNodeDetails(exNode);
	} break;
	case EditorNodeType::AsignFloat:
	case EditorNodeType::AsignFloat2:
	case EditorNodeType::AsignFloat3:
	case EditorNodeType::AsignFloat4:
	{
		AsignVariableEditorNode* asignNode = static_cast<AsignVariableEditorNode*>(node);
		WriteExecutionNodeDetails(asignNode);
		WriteAttribute("ValuePin", asignNode->m_ValuePin);
		WriteAttribute("Name", asignNode->m_Name);
	} break;
	case EditorNodeType::VarFloat:
	case EditorNodeType::VarFloat2:
	case EditorNodeType::VarFloat3:
	case EditorNodeType::VarFloat4:
	{
		VariableEditorNode* varNode = static_cast<VariableEditorNode*>(node);
		WriteAttribute("Name", varNode->m_VariableName);
	} break;
	case EditorNodeType::ClearRenderTarget:
	{
		ClearRenderTargetEditorNode* clearRTNode = static_cast<ClearRenderTargetEditorNode*>(node);
		WriteExecutionNodeDetails(clearRTNode);
		WriteAttribute("ClearColorPin", clearRTNode->GetClearColorPin());
	} break;
	default:
		NOT_IMPLEMENTED;
		break;
	}

	WriteAttribute("ID", node->m_ID);
	
	WritePinList(node->m_Inputs);
	WritePinList(node->m_Outputs);
	WritePinList(node->m_Executions);

	WriteToken(END_NODE_TOKEN);
}

void NodeGraphSerializer::WriteExecutionNodeDetails(ExecutionEditorNode* node)
{
	WriteAttribute("ExecutionInput", node->m_ExectuionPinInput);
	WriteAttribute("ExecutionOutput", node->m_ExectuionPinOutput);
}

void NodeGraphSerializer::WriteLink(const EditorNodeLink& link)
{
	WriteToken(BEGIN_LINK_TOKEN);

	WriteAttribute("ID", link.ID);
	WriteAttribute("Start", link.Start);
	WriteAttribute("End", link.End);

	WriteToken(END_LINK_TOKEN);
}

#pragma endregion // WRITING

//////////////////////////////
///			READING			//
//////////////////////////////

#pragma region READING

void NodeGraphSerializer::ReadNodeList()
{
	EatToken(BEGIN_NODE_LIST_TOKEN);

	unsigned nodeCount = ReadIntAttr("Count");
	for (unsigned i = 0; i < nodeCount; i++)
	{
		EditorNode* node = ReadNode();
		m_NodeWriteGraph->AddNode(node);
	}

	EatToken(END_NODE_LIST_TOKEN);
}

void NodeGraphSerializer::ReadLinkList()
{
	EatToken(BEGIN_LINK_LIST_TOKEN);

	unsigned linkCount = ReadIntAttr("Count");
	for (unsigned i = 0; i < linkCount; i++)
	{
		EditorNodeLink link = ReadLink();
		m_NodeWriteGraph->AddLink(link);
	}

	EatToken(END_LINK_LIST_TOKEN);
}

void NodeGraphSerializer::ReadPinList(std::vector<EditorNodePin>& pins)
{
	EatToken(BEGIN_PIN_LIST_TOKEN);

	unsigned pinCount = ReadIntAttr("Count");
	if (pinCount != pins.size())
	{
		LOAD_ASSERT();
		m_OperationSuccess = false;
		return;
	}

	pins.resize(pinCount);
	for (unsigned i = 0; i < pinCount; i++)
		pins[i].ID = ReadIntAttr("ID");

	EatToken(END_PIN_LIST_TOKEN);
}

#define INIT_NODE(NodeType) NodeType* newNode = new NodeType{}; node = newNode

EditorNode* NodeGraphSerializer::ReadNode()
{
	EatToken(BEGIN_NODE_TOKEN);

	EditorNode* node = nullptr;
	
	EditorNodeType nodeType = IntToEnum<EditorNodeType>(ReadIntAttr("Type"));
	switch (nodeType)
	{
	case EditorNodeType::Bool:
	{
		INIT_NODE(BoolEditorNode);
		newNode->m_Value = ReadBoolAttr("Value");
	} break;
	case EditorNodeType::Float:
	{
		INIT_NODE(FloatEditorNode);
		ReadFloatNode(newNode);
	} break;
	case EditorNodeType::Float2:
	{
		INIT_NODE(Float2EditorNode);
		ReadFloatNode(newNode);
	} break;
	case EditorNodeType::Float3:
	{
		INIT_NODE(Float3EditorNode);
		ReadFloatNode(newNode);
	} break;
	case EditorNodeType::Float4:
	{
		INIT_NODE(Float4EditorNode);
		ReadFloatNode(newNode);
	} break;
	case EditorNodeType::FloatBinaryOperator:
	{
		INIT_NODE(FloatBinaryOperatorEditorNode);
		ReadBinaryOperatorNode(newNode);
	} break;
	case EditorNodeType::Float2BinaryOperator:
	{
		INIT_NODE(Float2BinaryOperatorEditorNode);
		ReadBinaryOperatorNode(newNode);
	} break;
	case EditorNodeType::Float3BinaryOperator:
	{
		INIT_NODE(Float3BinaryOperatorEditorNode);
		ReadBinaryOperatorNode(newNode);
	} break;
	case EditorNodeType::Float4BinaryOperator:
	{
		INIT_NODE(Float4BinaryOperatorEditorNode);
		ReadBinaryOperatorNode(newNode);
	} break;
	case EditorNodeType::If:
	{
		INIT_NODE(IfEditorNode);
		ReadExecutionNode(newNode);
		newNode->m_ConditionPin = ReadIntAttr("Condition");
		newNode->m_ExectuionPinElse = ReadIntAttr("ExecutionElse");
	} break;
	case EditorNodeType::Print:
	{
		INIT_NODE(PrintEditorNode);
		ReadExecutionNode(newNode);
		newNode->m_FloatInputPin = ReadIntAttr("FloatInput");
		newNode->m_Float2InputPin = ReadIntAttr("Float2Input");
		newNode->m_Float3InputPin = ReadIntAttr("Float3Input");
		newNode->m_Float4InputPin = ReadIntAttr("Float4Input");
	} break;
	case EditorNodeType::OnUpdate:
	{
		INIT_NODE(OnUpdateEditorNode);
		ReadExecutionNode(newNode);
	} break;
	case EditorNodeType::OnStart:
	{
		INIT_NODE(OnStartEditorNode);
		ReadExecutionNode(newNode);
	} break;
	case EditorNodeType::AsignFloat:
	{
		INIT_NODE(AsignFloatEditorNode);
		ReadExecutionNode(newNode);
		ReadAsignVariableNode(newNode);
	} break;
	case EditorNodeType::AsignFloat2:
	{
		INIT_NODE(AsignFloat2EditorNode);
		ReadExecutionNode(newNode);
		ReadAsignVariableNode(newNode);
	} break;
	case EditorNodeType::AsignFloat3:
	{
		INIT_NODE(AsignFloat3EditorNode);
		ReadExecutionNode(newNode);
		ReadAsignVariableNode(newNode);
	} break;
	case EditorNodeType::AsignFloat4:
	{
		INIT_NODE(AsignFloat3EditorNode);
		ReadExecutionNode(newNode);
		ReadAsignVariableNode(newNode);
	} break;
	case EditorNodeType::VarFloat:
	{
		INIT_NODE(VarFloatEditorNode);
		ReadVariableNode(newNode);
	} break;
	case EditorNodeType::VarFloat2:
	{
		INIT_NODE(VarFloat2EditorNode);
		ReadVariableNode(newNode);
	} break;
	case EditorNodeType::VarFloat3:
	{
		INIT_NODE(VarFloat3EditorNode);
		ReadVariableNode(newNode);
	} break;
	case EditorNodeType::VarFloat4:
	{
		INIT_NODE(VarFloat4EditorNode);
		ReadVariableNode(newNode);
	} break;
	case EditorNodeType::ClearRenderTarget:
	{
		INIT_NODE(ClearRenderTargetEditorNode);
		ReadExecutionNode(newNode);
		ReadAttribute("ClearColorPin");
	} break;
	default:
		NOT_IMPLEMENTED;
		m_OperationSuccess = false;
		node = new FloatEditorNode{}; // Just some random node to prevent crash
	}

	node->m_Type = nodeType;
	node->m_ID = ReadIntAttr("ID");
	
	ReadPinList(node->m_Inputs);
	ReadPinList(node->m_Outputs);
	ReadPinList(node->m_Executions);

	EatToken(END_NODE_TOKEN);

	return node;
}

EditorNodeLink NodeGraphSerializer::ReadLink()
{
	EatToken(BEGIN_LINK_TOKEN);

	EditorNodeLink link;
	link.ID = ReadIntAttr("ID");
	link.Start = ReadIntAttr("Start");
	link.End = ReadIntAttr("End");

	EatToken(END_LINK_TOKEN);

	return link;
}

void NodeGraphSerializer::ReadExecutionNode(ExecutionEditorNode* exNode)
{
	exNode->m_ExectuionPinInput = ReadIntAttr("ExecutionInput");
	exNode->m_ExectuionPinOutput = ReadIntAttr("ExecutionOutput");
}

void NodeGraphSerializer::ReadAsignVariableNode(AsignVariableEditorNode* asignNode)
{
	asignNode->m_ValuePin = ReadIntAttr("ValuePin");
	asignNode->m_Name = ReadStrAttr("Name");
}

void NodeGraphSerializer::ReadVariableNode(VariableEditorNode* varNode)
{
	varNode->m_VariableName = ReadStrAttr("Name");
}

void NodeGraphSerializer::ReadFloatNode(FloatNEditorNode* floatNode)
{
	const unsigned numFloats = ReadIntAttr("NumFloats");
	floatNode->m_Pin = ReadIntAttr("Pin");
	for (unsigned i = 0; i < numFloats; i++)
	{
		floatNode->m_Values[i] = ReadFloatAttr("Value" + std::to_string(i));
	}
}

void NodeGraphSerializer::ReadBinaryOperatorNode(BinaryOperatorEditorNode* binOpNode)
{
	binOpNode->m_Apin = ReadIntAttr("A");
	binOpNode->m_Bpin = ReadIntAttr("B");
	binOpNode->m_Op = ReadCharAttr("OP");
}

// Tmp hack since nodes will self initialize some data
void NodeGraphSerializer::ClearNode(EditorNode* node)
{
	node->m_Inputs.clear();
	node->m_Outputs.clear();
	node->m_Executions.clear();
}

#pragma endregion // READING

//////////////////////////////
///			UTILITY			//
//////////////////////////////

void NodeGraphSerializer::WriteToken(const std::string& token)
{
#ifdef ENABLE_TOKEN_VERIFICATION
	m_Output << token << "\n";
#endif
}

void NodeGraphSerializer::EatToken(const std::string& token)
{
	if (!m_UseTokens)
		return;

	if (!m_OperationSuccess) return;

	std::string line;
	if (!std::getline(m_Input, line) || token != line)
	{
		LOAD_ASSERT();
		m_OperationSuccess = false;
	}
}

std::string NodeGraphSerializer::ReadAttribute(const std::string& name)
{
	// Optimize this
	if (!m_OperationSuccess) return "";

	// Read line from stream
	std::string line;
	if (!std::getline(m_Input, line))
	{
		LOAD_ASSERT();
		m_OperationSuccess = false;
		return "";
	}
	
	// Read attribute name
	std::string attrName;
	unsigned it = 0;
	while (line[it] != ' ' && it < line.size())
		attrName += line[it++];

	if (attrName != name)
	{
		LOAD_ASSERT();
		m_OperationSuccess = false;
		return "";
	}

	// Skip before separator space
	it++;

	// Validate separator
	m_OperationSuccess = line[it++] == ':';
	if (!m_OperationSuccess)
	{
		LOAD_ASSERT();
		return "";
	}

	// Skip after separator space
	it++;

	// Read value
	std::string value;
	while (it < line.size())
		value += line[it++];

	return value;
}

int NodeGraphSerializer::ReadIntAttr(const std::string& name)
{
	const std::string intStr = ReadAttribute(name);
	return m_OperationSuccess ? std::stoi(intStr) : 0;
}

float NodeGraphSerializer::ReadFloatAttr(const std::string& name)
{
	const std::string floatStr = ReadAttribute(name);
	return m_OperationSuccess ? std::stof(floatStr) : 0.0f;
}

char NodeGraphSerializer::ReadCharAttr(const std::string& name)
{
	const std::string charStr = ReadAttribute(name);
	m_OperationSuccess = m_OperationSuccess && charStr.size() == 1;
	return m_OperationSuccess ? charStr[0] : ' ';
}

bool NodeGraphSerializer::ReadBoolAttr(const std::string& name)
{
	const int val = ReadIntAttr(name);
	ASSERT(val == 0 || val == 1);
	return val != 0;
}

std::string NodeGraphSerializer::ReadStrAttr(const std::string& name)
{
	return ReadAttribute(name);
}