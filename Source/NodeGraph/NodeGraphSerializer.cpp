#include "NodeGraphSerializer.h"

#include <sstream>

#include "../Common.h"
#include "../NodeGraph/NodeGraph.h"
#include "../Editor/RenderPipelineEditor.h"

// #define ENABLE_TOKEN_VERIFICATION
#define ASSERT_ON_LOAD_FAILED

#ifdef ASSERT_ON_LOAD_FAILED
#define LOAD_ASSERT() ASSERT(0)
#else
#define LOAD_ASSERT() {}
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

	m_Version = VERSION;

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

#define READ_EDITOR_NODE(Type) Type* readNode = static_cast<Type*>(node)

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
		READ_EDITOR_NODE(BoolEditorNode);
		WriteAttribute("Value", readNode->m_Value ? 1 : 0);
	} break;
	case EditorNodeType::Float:
	case EditorNodeType::Float2:
	case EditorNodeType::Float3:
	case EditorNodeType::Float4:
	{
		READ_EDITOR_NODE(FloatNEditorNode);
		WriteAttribute("NumFloats", readNode->m_NumValues);
		for (unsigned i = 0; i < readNode->m_NumValues; i++)
		{
			WriteAttribute("Value" + std::to_string(i), readNode->m_Values[i]);
		}
	} break;
	case EditorNodeType::FloatBinaryOperator:
	case EditorNodeType::Float2BinaryOperator:
	case EditorNodeType::Float3BinaryOperator:
	case EditorNodeType::Float4BinaryOperator:
	case EditorNodeType::FloatComparisonOperator:
	case EditorNodeType::BoolBinaryOperator:
	{
		READ_EDITOR_NODE(BinaryOperatorEditorNode);
		WriteAttribute("OP", readNode->m_Op);
	} break;
	case EditorNodeType::If:
	case EditorNodeType::Print:
	case EditorNodeType::PresentTexture:
	case EditorNodeType::ClearRenderTarget:
	case EditorNodeType::GetCubeMesh:
	case EditorNodeType::DrawMesh:
	case EditorNodeType::BindTable:
	case EditorNodeType::CreateFloat2:
	case EditorNodeType::CreateFloat3:
	case EditorNodeType::CreateFloat4:
	case EditorNodeType::SplitFloat2:
	case EditorNodeType::SplitFloat3:
	case EditorNodeType::SplitFloat4:
	case EditorNodeType::OnUpdate:
	case EditorNodeType::OnStart:
	{
		READ_EDITOR_NODE(EditorNode);
	} break;
	case EditorNodeType::AsignFloat:
	case EditorNodeType::AsignFloat2:
	case EditorNodeType::AsignFloat3:
	case EditorNodeType::AsignFloat4:
	case EditorNodeType::AsignBool:
	{
		READ_EDITOR_NODE(AsignVariableEditorNode);
		WriteAttribute("Name", readNode->m_Name);
	} break;
	case EditorNodeType::VarFloat:
	case EditorNodeType::VarFloat2:
	case EditorNodeType::VarFloat3:
	case EditorNodeType::VarFloat4:
	case EditorNodeType::VarBool:
	case EditorNodeType::GetTexture:
	case EditorNodeType::GetShader:
	{
		READ_EDITOR_NODE(VariableEditorNode);
		WriteAttribute("Name", readNode->m_VariableName);
	} break;
	case EditorNodeType::GetMesh:
	{
		READ_EDITOR_NODE(GetMeshEditorNode);
		WriteAttribute("Name", readNode->m_VariableName);
		WriteAttribute("PositionBit", readNode->m_PositionBit ? 1 : 0);
		WriteAttribute("TexcoordBit", readNode->m_TexcoordBit ? 1 : 0);
		WriteAttribute("NormalBit", readNode->m_NormalBit ? 1 : 0);
		WriteAttribute("TangentBit", readNode->m_TangentBit ? 1 : 0);
	} break;
	case EditorNodeType::CreateTexture:
	{
		READ_EDITOR_NODE(CreateTextureEditorNode);
		WriteAttribute("Name", readNode->m_Name);
		WriteAttribute("Width", readNode->m_Width);
		WriteAttribute("Height", readNode->m_Height);
		WriteAttribute("IsFramebuffer", readNode->m_Framebuffer);
	} break;
	case EditorNodeType::LoadShader:
	case EditorNodeType::LoadTexture:
	case EditorNodeType::LoadMesh:
	{
		READ_EDITOR_NODE(NameAndPathExecutionEditorNode);
		WriteAttribute("Name", readNode->m_Name);
		WriteAttribute("Path", readNode->m_Path);
	} break;
	default:
		NOT_IMPLEMENTED;
		break;
	}

	WriteAttribute("ID", node->m_ID);

	// Write pins
	WriteToken(BEGIN_PIN_LIST_TOKEN);
	WriteAttribute("Count", node->m_Pins.size());
	for (const EditorNodePin& pin : node->m_Pins) WriteAttribute("ID", pin.ID);
	WriteToken(END_PIN_LIST_TOKEN);
	
	// Write custom pins
	if (m_Version > 1)
	{
		WriteToken(BEGIN_PIN_LIST_TOKEN);
		WriteAttribute("Count", node->m_CustomPins.size());
		for (const EditorNodePin& pin : node->m_CustomPins)
		{
			WriteAttribute("ID", pin.ID);
			WriteAttribute("IsInput", pin.IsInput ? 1 : 0);
			WriteAttribute("Type", EnumToInt(pin.Type));
			WriteAttribute("Label", pin.Label);
		}
		WriteToken(END_PIN_LIST_TOKEN);
	}

	WriteToken(END_NODE_TOKEN);
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
	case EditorNodeType::CreateFloat2:
	{
		INIT_NODE(CreateFloat2EditorNode);
	} break;
	case EditorNodeType::CreateFloat3:
	{
		INIT_NODE(CreateFloat3EditorNode);
	} break;
	case EditorNodeType::CreateFloat4:
	{
		INIT_NODE(CreateFloat4EditorNode);
	} break;
	case EditorNodeType::SplitFloat2:
	{
		INIT_NODE(SplitFloat2EditorNode);
	} break;
	case EditorNodeType::SplitFloat3:
	{
		INIT_NODE(SplitFloat3EditorNode);
	} break;
	case EditorNodeType::SplitFloat4:
	{
		INIT_NODE(SplitFloat4EditorNode);
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
	case EditorNodeType::FloatComparisonOperator:
	{
		INIT_NODE(FloatComparisonOperatorEditorNode);
		ReadBinaryOperatorNode(newNode);
	} break;
	case EditorNodeType::BoolBinaryOperator:
	{
		INIT_NODE(BoolBinaryOperatorEditorNode);
		ReadBinaryOperatorNode(newNode);
	} break;
	case EditorNodeType::If:
	{
		INIT_NODE(IfEditorNode);
	} break;
	case EditorNodeType::Print:
	{
		INIT_NODE(PrintEditorNode);
	} break;
	case EditorNodeType::OnUpdate:
	{
		INIT_NODE(OnUpdateEditorNode);
	} break;
	case EditorNodeType::OnStart:
	{
		INIT_NODE(OnStartEditorNode);
	} break;
	case EditorNodeType::AsignFloat:
	{
		INIT_NODE(AsignFloatEditorNode);
		ReadAsignVariableNode(newNode);
	} break;
	case EditorNodeType::AsignFloat2:
	{
		INIT_NODE(AsignFloat2EditorNode);
		ReadAsignVariableNode(newNode);
	} break;
	case EditorNodeType::AsignFloat3:
	{
		INIT_NODE(AsignFloat3EditorNode);
		ReadAsignVariableNode(newNode);
	} break;
	case EditorNodeType::AsignFloat4:
	{
		INIT_NODE(AsignFloat4EditorNode);
		ReadAsignVariableNode(newNode);
	} break;
	case EditorNodeType::AsignBool:
	{
		INIT_NODE(AsignBoolEditorNode);
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
	case EditorNodeType::VarBool:
	{
		INIT_NODE(VarBoolEditorNode);
		ReadVariableNode(newNode);
	} break;
	case EditorNodeType::GetTexture:
	{
		INIT_NODE(GetTextureEditorNode);
		ReadVariableNode(newNode);
	} break;
	case EditorNodeType::GetShader:
	{
		INIT_NODE(GetShaderEditorNode);
		ReadVariableNode(newNode);
	} break;
	case EditorNodeType::ClearRenderTarget:
	{
		INIT_NODE(ClearRenderTargetEditorNode);
	} break;
	case EditorNodeType::CreateTexture:
	{
		INIT_NODE(CreateTextureEditorNode);
		newNode->m_Name = ReadStrAttr("Name");
		newNode->m_Width = ReadIntAttr("Width");
		newNode->m_Height = ReadIntAttr("Height");
		newNode->m_Framebuffer = ReadBoolAttr("IsFramebuffer");
	} break;
	case EditorNodeType::PresentTexture:
	{
		INIT_NODE(PresentTextureEditorNode);
	} break;
	case EditorNodeType::LoadTexture:
	{
		INIT_NODE(LoadTextureEditorNode);
		ReadNamePathPathNode(newNode);
	} break;
	case EditorNodeType::LoadShader:
	{
		INIT_NODE(LoadShaderEditorNode);
		ReadNamePathPathNode(newNode);
	} break;
	case EditorNodeType::GetCubeMesh:
	{
		INIT_NODE(GetCubeMeshEditorNode);
	} break;
	case EditorNodeType::DrawMesh:
	{
		INIT_NODE(DrawMeshEditorNode);
	} break;
	case EditorNodeType::BindTable:
	{
		INIT_NODE(BindTableEditorNode);
	} break;
	case EditorNodeType::GetMesh:
	{
		INIT_NODE(GetMeshEditorNode);
		ReadVariableNode(newNode);
		newNode->m_PositionBit = ReadBoolAttr("PositionBit");
		newNode->m_TexcoordBit = ReadBoolAttr("TexcoordBit");
		newNode->m_NormalBit = ReadBoolAttr("NormalBit");
		newNode->m_TangentBit = ReadBoolAttr("TangentBit");
	} break;
	case EditorNodeType::LoadMesh:
	{
		INIT_NODE(LoadMeshEditorNode);
		ReadNamePathPathNode(newNode);
	} break;
	default:
		NOT_IMPLEMENTED;
		m_OperationSuccess = false;
		node = new FloatEditorNode{}; // Just some random node to prevent crash
	}

	node->m_Type = nodeType;
	node->m_ID = ReadIntAttr("ID");
	
	// Read pins
	{
		EatToken(BEGIN_PIN_LIST_TOKEN);

		unsigned pinCount = ReadIntAttr("Count");
		if (pinCount > node->m_Pins.size())
		{
			LOAD_ASSERT();
			m_OperationSuccess = false;
			return node;
		}
		for (unsigned i = 0; i < pinCount; i++)
			node->m_Pins[i].ID = ReadIntAttr("ID");

		EatToken(END_PIN_LIST_TOKEN);
	}

	// Read custom pins
	if(m_Version > 1)
	{
		EatToken(BEGIN_PIN_LIST_TOKEN);

		unsigned pinCount = ReadIntAttr("Count");
		node->m_CustomPins.resize(pinCount);
		for (unsigned i = 0; i < pinCount; i++)
		{
			EditorNodePin pin;
			pin.ID = ReadIntAttr("ID");
			pin.IsInput = ReadBoolAttr("IsInput");
			pin.Type = IntToEnum<PinType>(ReadIntAttr("Type"));
			pin.Label = ReadStrAttr("Label");
			node->m_CustomPins[i] = pin;
		}

		EatToken(END_PIN_LIST_TOKEN);
	}

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

void NodeGraphSerializer::ReadAsignVariableNode(AsignVariableEditorNode* asignNode)
{
	asignNode->m_Name = ReadStrAttr("Name");
}

void NodeGraphSerializer::ReadVariableNode(VariableEditorNode* varNode)
{
	varNode->m_VariableName = ReadStrAttr("Name");
}

void NodeGraphSerializer::ReadFloatNode(FloatNEditorNode* floatNode)
{
	const unsigned numFloats = ReadIntAttr("NumFloats");
	for (unsigned i = 0; i < numFloats; i++)
	{
		floatNode->m_Values[i] = ReadFloatAttr("Value" + std::to_string(i));
	}
}

void NodeGraphSerializer::ReadBinaryOperatorNode(BinaryOperatorEditorNode* binOpNode)
{
	binOpNode->m_Op = ReadStrAttr("OP");
}

void NodeGraphSerializer::ReadNamePathPathNode(NameAndPathExecutionEditorNode* nameAndPathNode)
{
	nameAndPathNode->m_Name = ReadStrAttr("Name");
	nameAndPathNode->m_Path = ReadStrAttr("Path");
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

static bool ValidateIntStr(const std::string& intStr)
{
	if (intStr.empty()) return false;
	for (unsigned i = (intStr[0] == '-') ? 1 : 0; i < intStr.size(); i++)
	{
		if (!std::isdigit(intStr[i]))
		{
			return false;
		}
	}
	return true;
}

static bool ValidateFloatStr(const std::string& floatStr)
{
	if (floatStr.empty()) return false;

	unsigned numDots = 0;
	for (unsigned i = (floatStr[0] == '-') ? 1 : 0; i < floatStr.size(); i++)
	{
		if (floatStr[i] == '.')
		{
			numDots++;
			if (numDots > 1)
				return false;

			continue;
		}
		if (!std::isdigit(floatStr[i]))
		{
			return false;
		}
	}
	return true;
}

int NodeGraphSerializer::ReadIntAttr(const std::string& name)
{
	const std::string intStr = ReadAttribute(name);
	if (!ValidateIntStr(intStr))
	{
		LOAD_ASSERT();
		m_OperationSuccess = false;
	}
	return m_OperationSuccess ? std::stoi(intStr) : 0;
}

float NodeGraphSerializer::ReadFloatAttr(const std::string& name)
{
	const std::string floatStr = ReadAttribute(name);
	if (!ValidateFloatStr(floatStr))
	{
		LOAD_ASSERT();
		m_OperationSuccess = false;
	}
	return m_OperationSuccess ? std::stof(floatStr) : 0.0f;
}

char NodeGraphSerializer::ReadCharAttr(const std::string& name)
{
	const std::string charStr = ReadAttribute(name);
	m_OperationSuccess = m_OperationSuccess && charStr.size() == 1;
	if (!m_OperationSuccess) LOAD_ASSERT();
	return m_OperationSuccess ? charStr[0] : 0;
}

bool NodeGraphSerializer::ReadBoolAttr(const std::string& name)
{
	const int val = ReadIntAttr(name);
	m_OperationSuccess = m_OperationSuccess && (val == 0 || val == 1);
	if (!m_OperationSuccess) LOAD_ASSERT();
	return m_OperationSuccess ? val != 0 : false;
}

std::string NodeGraphSerializer::ReadStrAttr(const std::string& name)
{
	return ReadAttribute(name);
}