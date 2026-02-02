#include "NodeGraphSerializer.h"

#include <sstream>

#include "../App/App.h"
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

static const std::string BEGIN_NODE_GRAPH_TOKEN = "BEGIN_NODE_GRAPH";
static const std::string END_NODE_GRAPH_TOKEN = "END_NODE_GRAPH";
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
static const std::string BEGIN_NODE_POSITIONS_TOKEN = "BEGIN_NODE_POSITIONS";
static const std::string END_NODE_POSITIONS_TOKEN = "END_NODE_POSITIONS";
static const std::string BEGIN_VARIABLE_POOL_TOKEN = "BEGIN_VARIABLE_POOL";
static const std::string END_VARIABLE_POOL_TOKEN = "END_VARIABLE_POOL";
static const std::string BEGIN_VARIABLE_TOKEN = "BEGIN_VARIABLE";
static const std::string END_VARIABLE_TOKEN = "END_VARIABLE";

void NodeGraphSerializer::Serialize(const std::string& path, const NodeGraph& nodeGraph, const VariablePool& variablePool)
{
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

	WriteVariablePool(variablePool);
	WriteCustomNodeList();
	WriteNodeGraph(nodeGraph);

	m_Output.close();
}

UniqueID NodeGraphSerializer::Deserialize(const std::string& path, NodeGraph& nodeGraph, VariablePool& variablePool, std::vector<CustomEditorNode*>& customNodes)
{
	m_OperationSuccess = true;
	
	m_Input = std::ifstream{ path };
	if (!m_Input.is_open()) return 0;

	m_Version = ReadIntAttr("Version");

	if (m_Version < 7)
	{
		m_OperationSuccess = false;
		App::Get()->GetConsole().Log("Not compatabile with this version of file");
		return 0;
	}

	m_UseTokens = ReadIntAttr("UseTokens");
	unsigned firstID = ReadIntAttr("FirstID");

	variablePool = {};
	if (m_Version >= 9)
	{
		ReadVariablePool(variablePool);
	}
	
	customNodes = ReadCustomNodeList();
	ReadNodeGraph(nodeGraph, customNodes);

	m_Input.close();

	return m_OperationSuccess ? firstID : 0;
}

//////////////////////////////
///			WRITING			//
//////////////////////////////

#pragma region(WRITING)

void NodeGraphSerializer::WriteVariablePool(const VariablePool& variablePool)
{
	WriteToken(BEGIN_VARIABLE_POOL_TOKEN);

	uint32_t variableCount = 0;
	const auto countVariables = [&variableCount](VariableID id, const Variable& variable) {
		variableCount++;
	};
	variablePool.ForEachVariable(countVariables);

	WriteAttribute("Count", variableCount);

	uint32_t writtenCount = 0;
	const auto writeVariable = [this, &writtenCount, variableCount](VariableID id, const Variable& variable) {
		if (writtenCount < variableCount)
		{
			WriteVariable(id, variable);
		}
		else
		{
			ASSERT_M(0, "Internal error while serializing variable pool");
		}
		writtenCount++;
	};
	variablePool.ForEachVariable(writeVariable);

	WriteToken(END_VARIABLE_POOL_TOKEN);
}

void NodeGraphSerializer::WriteVariable(VariableID id, const Variable& variable)
{
	WriteToken(BEGIN_VARIABLE_TOKEN);

	WriteAttribute("ID", id);
	WriteAttribute("Name", variable.Name);
	WriteAttribute("Type", EnumToInt(variable.Type));
	switch (variable.Type)
	{
	case VariableType::Bool:
		WriteAttribute("ValueBool", variable.Get<bool>());
		break;
	case VariableType::Int:
		WriteAttribute("ValueInt", variable.Get<int>());
		break;
	case VariableType::Float:
		WriteAttribute("ValueFloat", variable.Get<float>());
		break;
	case VariableType::Float2:
		WriteAttribute("ValueFloat2", variable.Get<Float2>());
		break;
	case VariableType::Float3:
		WriteAttribute("ValueFloat3", variable.Get<Float3>());
		break;
	case VariableType::Float4:
		WriteAttribute("ValueFloat4", variable.Get<Float4>());
		break;
	case VariableType::Float4x4:
		WriteAttribute("ValueFloat4x4", variable.Get<Float4x4>());
		break;
	case VariableType::Shader:
		WriteAttribute("ValueShaderData", variable.Get<ShaderData>());
		break;
	case VariableType::Texture:
		WriteAttribute("ValueTextureData", variable.Get<TextureData>());
		break;
	case VariableType::Scene:
		WriteAttribute("ValueSceneData", variable.Get<SceneData>());
		break;
	case VariableType::Invalid:
	case VariableType::Count:
		break;
	default:
		NOT_IMPLEMENTED;
		break;
	}

	WriteToken(END_VARIABLE_TOKEN);
}

void NodeGraphSerializer::WriteNodeGraph(const NodeGraph& nodeGraph)
{
	WriteToken(BEGIN_NODE_GRAPH_TOKEN);

	WriteNodeList(nodeGraph);
	WriteLinkList(nodeGraph);
	WriteNodePositions(nodeGraph);

	WriteToken(END_NODE_GRAPH_TOKEN);
}

void NodeGraphSerializer::WriteNodeList(const NodeGraph& nodeGraph)
{
	WriteToken(BEGIN_NODE_LIST_TOKEN);
	WriteAttribute("Count", nodeGraph.GetNodeCount());

	const auto fn = [this](EditorNode* node) { this->WriteNode(node); };
	nodeGraph.ForEachNode(fn);

	WriteToken(END_NODE_LIST_TOKEN);
}

void NodeGraphSerializer::WriteLinkList(const NodeGraph& nodeGraph)
{
	WriteToken(BEGIN_LINK_LIST_TOKEN);
	WriteAttribute("Count", nodeGraph.GetLinkCount());

	const auto fn = [this](const EditorNodeLink& link) { this->WriteLink(link); };
	nodeGraph.ForEachLink(fn);

	WriteToken(END_LINK_LIST_TOKEN);
}

void NodeGraphSerializer::WriteCustomNodeList()
{
	const auto& customNodes = *App::Get()->GetCustomNodes();

	WriteToken(BEGIN_NODE_LIST_TOKEN);

	std::unordered_map<std::string, std::unordered_set<std::string>> nodeDependencies;
	std::unordered_set<CustomEditorNode*> nodesToWrite;
	for (const auto& customNode : customNodes)
	{
		const auto fn = [&nodeDependencies, cnName = customNode->GetName()](EditorNode* node) {
			if (node->GetType() == EditorNodeType::Custom)
			{
				CustomEditorNode* cn = static_cast<CustomEditorNode*>(node);
				nodeDependencies[cnName].insert(cn->GetName());
			}
		};
		customNode->GetNodeGraph()->ForEachNode(fn);
		nodesToWrite.insert(customNode.get());
	}

	std::unordered_set<std::string> writtenNodes;
	WriteAttribute("Count", customNodes.size());
	while (!nodesToWrite.empty())
	{
		CustomEditorNode* nodeToWrite = nullptr;
		for (const auto& customNode : nodesToWrite)
		{
			bool foundMissingDependency = false;
			for (const auto& nodeDep : nodeDependencies[customNode->GetName()])
			{
				if (writtenNodes.count(nodeDep) == 0)
				{
					foundMissingDependency = true;
					break;
				}
			}

			if (!foundMissingDependency)
			{
				nodeToWrite = customNode;
				break;
			}
		}

		if (nodeToWrite)
		{
			WriteCustomNode(nodeToWrite);

			nodesToWrite.erase(nodeToWrite);
			writtenNodes.insert(nodeToWrite->GetName());
		}
		else
		{
			LOAD_ASSERT();
			m_OperationSuccess = false;
			return;
		}
	}

	WriteToken(END_NODE_LIST_TOKEN);
}

void NodeGraphSerializer::WriteCustomNode(CustomEditorNode* node)
{
	WriteToken(BEGIN_NODE_TOKEN);

	WriteAttribute("Name", node->m_Name);
	WriteNodeGraph(*node->GetNodeGraph());

	WriteToken(END_NODE_TOKEN);
}

void NodeGraphSerializer::WritePinList(const std::vector<EditorNodePin>& pins, bool custom)
{
	WriteToken(BEGIN_PIN_LIST_TOKEN);
	WriteAttribute("Count", pins.size());
	for (const EditorNodePin& pin : pins)
		WritePin(pin, custom);
	WriteToken(END_PIN_LIST_TOKEN);

}

void NodeGraphSerializer::WritePin(const EditorNodePin& pin, bool custom)
{
	WriteAttribute("ID", pin.ID);
	
	if (custom)
	{
		WriteAttribute("IsInput", pin.IsInput);
		WriteAttribute("Type", EnumToInt(pin.Type));
		WriteAttribute("Label", pin.Label);
		WriteAttribute("LinkedNode", pin.LinkedNode);
	}
	
	WriteAttribute("HasConstantValue", pin.HasConstantValue);

	if (pin.HasConstantValue)
	{
		switch (pin.Type)
		{
		case PinType::Bool:
			WriteAttribute("ConstantValueB", pin.ConstantValue.B);
			break;
		case PinType::Int:
			WriteAttribute("ConstantValueI", pin.ConstantValue.I);
			break;
		case PinType::Float:
			WriteAttribute("ConstantValueF", pin.ConstantValue.F);
			break;
		case PinType::Float2:
			WriteAttribute("ConstantValueF2", pin.ConstantValue.F2);
			break;
		case PinType::Float3:
			WriteAttribute("ConstantValueF3", pin.ConstantValue.F3);
			break;
		case PinType::Float4:
			WriteAttribute("ConstantValueF4", pin.ConstantValue.F4);
			break;
		case PinType::String:
			WriteAttribute("ConstantValueSTR", pin.ConstantValue.STR);
			break;
		default:
			NOT_IMPLEMENTED;
		}
	}
}

void NodeGraphSerializer::WriteNodePositions(const NodeGraph& nodeGraph)
{
	WriteToken(BEGIN_NODE_POSITIONS_TOKEN);

	WriteAttribute("Count", nodeGraph.GetNodePositions().size());
	for (const auto& it : nodeGraph.GetNodePositions())
	{
		const NodeID& nodeID = it.first;
		const ImVec2& nodePos = it.second;

		WriteAttribute("ID", nodeID);
		WriteAttribute("Pos.X", nodePos.x);
		WriteAttribute("Pos.Y", nodePos.y);
	}
	WriteToken(END_NODE_POSITIONS_TOKEN);
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
		WriteAttribute("Value", readNode->m_Value);
	} break;
	case EditorNodeType::Int:
	{
		READ_EDITOR_NODE(IntEditorNode);
		WriteAttribute("Value", readNode->m_Value);
	} break;
	case EditorNodeType::String:
	{
		READ_EDITOR_NODE(StringEditorNode);
		WriteAttribute("Value", readNode->m_Value);
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
	case EditorNodeType::Float4x4:
	{
		READ_EDITOR_NODE(FloatNxNEditorNode);
		WriteAttribute("NumFloatsX", readNode->m_NumValuesX);
		WriteAttribute("NumFloatsY", readNode->m_NumValuesY);
		for (unsigned i = 0; i < readNode->m_NumValuesX; i++)
		{
			for (unsigned j = 0; j < readNode->m_NumValuesY; j++)
			{
				WriteAttribute("Value" + std::to_string(i) + std::to_string(j), readNode->m_Values[i][j]);
			}
		}
	} break;
	case EditorNodeType::FloatBinaryOperator:
	case EditorNodeType::Float2BinaryOperator:
	case EditorNodeType::Float3BinaryOperator:
	case EditorNodeType::Float4BinaryOperator:
	case EditorNodeType::Float4x4BinaryOperator:
	case EditorNodeType::FloatComparisonOperator:
	case EditorNodeType::BoolBinaryOperator:
	case EditorNodeType::IntBinaryOperator:
	case EditorNodeType::IntComparisonOperator:
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
	case EditorNodeType::Transform_Rotate_Float4x4:
	case EditorNodeType::Transform_Translate_Float4x4:
	case EditorNodeType::Transform_Scale_Float4x4:
	case EditorNodeType::Transform_LookAt_Float4x4:
	case EditorNodeType::Transform_PerspectiveProjection_Float4x4:
	case EditorNodeType::NormalizeFloat2:
	case EditorNodeType::NormalizeFloat3:
	case EditorNodeType::NormalizeFloat4:
	case EditorNodeType::CrossProductOperation:
	case EditorNodeType::ForEachSceneObject:
	{
		READ_EDITOR_NODE(EditorNode);
	} break;
	case EditorNodeType::GetMesh:
	{
		READ_EDITOR_NODE(GetMeshEditorNode);
		WriteAttribute("PositionBit", readNode->m_PositionBit);
		WriteAttribute("TexcoordBit", readNode->m_TexcoordBit);
		WriteAttribute("NormalBit", readNode->m_NormalBit);
		WriteAttribute("TangentBit", readNode->m_TangentBit);
	} break;
	case EditorNodeType::RenderState:
	{
		READ_EDITOR_NODE(RenderStateEditorNode);
		WriteAttribute("DepthWrite", readNode->m_DepthWrite);
		WriteAttribute("DepthTest", readNode->m_DepthTestMode);
	} break;
	case EditorNodeType::Pin:
	{
		READ_EDITOR_NODE(PinEditorNode);
		WriteAttribute("IsInput", readNode->GetPin().IsInput);
		WriteAttribute("PinType", EnumToInt(readNode->GetPin().Type));
		WriteAttribute("Name", readNode->m_Name);
	} break;
	case EditorNodeType::OnKeyPressed:
	case EditorNodeType::OnKeyReleased:
	case EditorNodeType::OnKeyDown:
	{
		READ_EDITOR_NODE(InputExecutionEditorNode);
		WriteAttribute("Key", readNode->GetKey());
		WriteAttribute("Mods", readNode->GetMods());
	} break;
	case EditorNodeType::Variable:
	{
		READ_EDITOR_NODE(VariableEditorNode);
		WriteAttribute("ID", readNode->m_VarID);
		WriteAttribute("Type", EnumToInt(readNode->m_VarType));
	} break;
	case EditorNodeType::AsignVariable:
	{
		READ_EDITOR_NODE(AsignVariableEditorNode);
		WriteAttribute("ID", readNode->m_VarID);
		WriteAttribute("Type", EnumToInt(readNode->m_VarType));
	} break;
	case EditorNodeType::Custom:
	{
		READ_EDITOR_NODE(CustomEditorNode);
		WriteAttribute("CustomNodeName", readNode->GetName());
	} break;
	case EditorNodeType::DEPRECATED_VarFloat:
	case EditorNodeType::DEPRECATED_VarFloat2:
	case EditorNodeType::DEPRECATED_VarFloat3:
	case EditorNodeType::DEPRECATED_VarFloat4:
	case EditorNodeType::DEPRECATED_VarFloat4x4:
	case EditorNodeType::DEPRECATED_VarBool:
	case EditorNodeType::DEPRECATED_VarInt:
	case EditorNodeType::DEPRECATED_GetScene:
	case EditorNodeType::DEPRECATED_GetTexture:
	case EditorNodeType::DEPRECATED_GetShader:
	case EditorNodeType::DEPRECATED_LoadShader:
	case EditorNodeType::DEPRECATED_LoadTexture:
	case EditorNodeType::DEPRECATED_LoadScene:
	case EditorNodeType::DEPRECATED_AsignFloat:
	case EditorNodeType::DEPRECATED_AsignFloat2:
	case EditorNodeType::DEPRECATED_AsignFloat3:
	case EditorNodeType::DEPRECATED_AsignFloat4:
	case EditorNodeType::DEPRECATED_AsignFloat4x4:
	case EditorNodeType::DEPRECATED_AsignBool:
	case EditorNodeType::DEPRECATED_AsignInt:
	case EditorNodeType::DEPRECATED_CreateTexture:
	{
		ASSERT_M(0, "Trying to save deprecated node");
	} break;
	case EditorNodeType::Deprecated:
	{
		// Do nothing we dont want to save this nodes
	} break;
	default:
		NOT_IMPLEMENTED;
		break;
	}

	WriteAttribute("ID", node->m_ID);

	WritePinList(node->m_Pins, false);
	WritePinList(node->m_CustomPins, true);

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

void NodeGraphSerializer::ReadVariablePool(VariablePool& variablePool)
{
	EatToken(BEGIN_VARIABLE_POOL_TOKEN);

	const int variableCount = ReadIntAttr("Count");

	for (int i = 0; i < variableCount; i++)
	{
		EatToken(BEGIN_VARIABLE_TOKEN);

		const VariableID id = ReadIntAttr("ID");
		const std::string name = ReadStrAttr("Name");
		const VariableType type = IntToEnum<VariableType>(ReadIntAttr("Type"));
		Variable& var = variablePool.GetRefOrCreate(id, type, name);

		switch (var.Type)
		{
		case VariableType::Bool:
			var.Get<bool>() = ReadBoolAttr("ValueBool");
			break;
		case VariableType::Int:
			var.Get<int>() = ReadIntAttr("ValueInt");
			break;
		case VariableType::Float:
			var.Get<float>() = ReadFloatAttr("ValueFloat");
			break;
		case VariableType::Float2:
			var.Get<Float2>() = ReadFloat2Attr("ValueFloat2");
			break;
		case VariableType::Float3:
			var.Get<Float3>() = ReadFloat3Attr("ValueFloat3");
			break;
		case VariableType::Float4:
			var.Get<Float4>() = ReadFloat4Attr("ValueFloat4");
			break;
		case VariableType::Float4x4:
			var.Get<Float4x4>() = ReadFloat4x4Attr("ValueFloat4x4");
			break;
		case VariableType::Shader:
			var.Get<ShaderData>() = ReadShaderDataAttr("ValueShaderData");
			break;
		case VariableType::Texture:
			var.Get<TextureData>() = ReadTextureDataAttr("ValueTextureData");
			break;
		case VariableType::Scene:
			var.Get<SceneData>() = ReadSceneDataAttr("ValueSceneData");
			break;
		case VariableType::Count:
		case VariableType::Invalid:
			break;
		default:
			NOT_IMPLEMENTED;
			break;
		}

		EatToken(END_VARIABLE_TOKEN);
	}
	EatToken(END_VARIABLE_POOL_TOKEN);
}

void NodeGraphSerializer::ReadNodeGraph(NodeGraph& nodeGraph, const std::vector<CustomEditorNode*>& customNodes)
{
	EatToken(BEGIN_NODE_GRAPH_TOKEN);

	ReadNodeList(nodeGraph, customNodes);
	ReadLinkList(nodeGraph);
	ReadNodePositions(nodeGraph);
	CleanupDeprecatedNodes(nodeGraph);

	EatToken(END_NODE_GRAPH_TOKEN);
}

void NodeGraphSerializer::ReadNodeList(NodeGraph& nodeGraph, const std::vector<CustomEditorNode*>& customNodes)
{
	EatToken(BEGIN_NODE_LIST_TOKEN);

	unsigned nodeCount = ReadIntAttr("Count");
	for (unsigned i = 0; i < nodeCount; i++)
	{
		EditorNode* node = ReadNode(nodeGraph, customNodes);
		nodeGraph.AddNode(node);
	}

	EatToken(END_NODE_LIST_TOKEN);
}

void NodeGraphSerializer::ReadNodePositions(NodeGraph& nodeGraph)
{
	EatToken(BEGIN_NODE_POSITIONS_TOKEN);

	std::unordered_map<NodeID, ImVec2> nodePositions;

	const unsigned n = ReadIntAttr("Count");
	for (unsigned i = 0; i < n; i++)
	{
		const NodeID nodeID = ReadIntAttr("ID");
		ImVec2 nodePos{};
		nodePos.x = ReadFloatAttr("Pos.X");
		nodePos.y = ReadFloatAttr("Pos.Y");

		nodePositions[nodeID] = nodePos;
	}

	nodeGraph.SetNodePositions(nodePositions);

	EatToken(END_NODE_POSITIONS_TOKEN);
}

void NodeGraphSerializer::CleanupDeprecatedNodes(NodeGraph& nodeGraph)
{
	std::vector<NodeID> deprecatedNodes{};
	const auto fn = [&deprecatedNodes](EditorNode* node) {
		if (node->GetType() == EditorNodeType::Deprecated)
		{
			deprecatedNodes.push_back(node->GetID());
		}
	};
	nodeGraph.ForEachNode(fn);

	for (const auto node : deprecatedNodes)
	{
		nodeGraph.RemoveAllPins(node);
	}
}

std::vector<CustomEditorNode*> NodeGraphSerializer::ReadCustomNodeList()
{
	std::vector<CustomEditorNode*> customNodes;

	EatToken(BEGIN_NODE_LIST_TOKEN);

	const unsigned customNodeCount = ReadIntAttr("Count");
	for (unsigned i = 0; i < customNodeCount; i++)
		customNodes.push_back(ReadCustomNode(customNodes));

	EatToken(END_NODE_LIST_TOKEN);

	return customNodes;
}

void NodeGraphSerializer::ReadLinkList(NodeGraph& nodeGraph)
{
	EatToken(BEGIN_LINK_LIST_TOKEN);

	unsigned linkCount = ReadIntAttr("Count");
	for (unsigned i = 0; i < linkCount; i++)
	{
		EditorNodeLink link = ReadLink();
		nodeGraph.AddLink(link);
	}

	EatToken(END_LINK_LIST_TOKEN);
}

#define INIT_NODE(NodeType) NodeType* newNode = new NodeType{}; node = newNode
#define INIT_SIMPLE_NODE(NodeEnum, NodeClass) case EditorNodeType::NodeEnum: { INIT_NODE(NodeClass);  } break;
#define INIT_FLOAT_NODE(NodeEnum, NodeClass) case EditorNodeType::NodeEnum: { INIT_NODE(NodeClass); const unsigned numFloats = ReadIntAttr("NumFloats"); for (unsigned i = 0; i < numFloats; i++) { newNode->m_Values[i] = ReadFloatAttr("Value" + std::to_string(i)); }  } break;
#define INIT_FLOAT_MATRIX_NODE(NodeEnum, NodeClass) case EditorNodeType::NodeEnum: { INIT_NODE(NodeClass); const unsigned numFloatsX = ReadIntAttr("NumFloatsX"); const unsigned numFloatsY = ReadIntAttr("NumFloatsY"); for (unsigned i = 0; i < numFloatsX; i++) { for(unsigned j = 0; j < numFloatsY; j++) { newNode->m_Values[i][j] = ReadFloatAttr("Value" + std::to_string(i) + std::to_string(j)); } }  } break;
#define INIT_BIN_OP_NODE(NodeEnum, NodeClass) case EditorNodeType::NodeEnum: { INIT_NODE(NodeClass); newNode->m_Op = ReadStrAttr("OP"); } break;
#define INIT_INPUT_NODE(NodeEnum, NodeClass) case EditorNodeType::NodeEnum: { INIT_NODE(NodeClass); newNode->m_Key = ReadIntAttr("Key"); newNode->m_Mods = ReadIntAttr("Mods"); } break

EditorNode* NodeGraphSerializer::ReadNode(NodeGraph& nodeGraph, const std::vector<CustomEditorNode*>& customNodes)
{
	EatToken(BEGIN_NODE_TOKEN);

	EditorNode* node = nullptr;
	
	EditorNodeType nodeType = IntToEnum<EditorNodeType>(ReadIntAttr("Type"));
	switch (nodeType)
	{
		INIT_SIMPLE_NODE(CreateFloat2, CreateFloat2EditorNode);
		INIT_SIMPLE_NODE(CreateFloat3, CreateFloat3EditorNode);
		INIT_SIMPLE_NODE(CreateFloat4, CreateFloat4EditorNode);
		INIT_SIMPLE_NODE(SplitFloat2, SplitFloat2EditorNode);
		INIT_SIMPLE_NODE(SplitFloat3, SplitFloat3EditorNode);
		INIT_SIMPLE_NODE(SplitFloat4, SplitFloat4EditorNode);
		INIT_SIMPLE_NODE(If, IfEditorNode);
		INIT_SIMPLE_NODE(Print, PrintEditorNode);
		INIT_SIMPLE_NODE(OnUpdate, OnUpdateEditorNode);
		INIT_SIMPLE_NODE(OnStart, OnStartEditorNode);
		INIT_SIMPLE_NODE(GetCubeMesh, GetCubeMeshEditorNode);
		INIT_SIMPLE_NODE(ClearRenderTarget, ClearRenderTargetEditorNode);
		INIT_SIMPLE_NODE(DrawMesh, DrawMeshEditorNode);
		INIT_SIMPLE_NODE(BindTable, BindTableEditorNode);
		INIT_SIMPLE_NODE(PresentTexture, PresentTextureEditorNode);
		INIT_SIMPLE_NODE(Transform_Rotate_Float4x4, Float4x4RotationTransformEditorNode);
		INIT_SIMPLE_NODE(Transform_Translate_Float4x4, Float4x4TranslationTransformEditorNode);
		INIT_SIMPLE_NODE(Transform_Scale_Float4x4, Float4x4ScaleTransformEditorNode);
		INIT_SIMPLE_NODE(Transform_LookAt_Float4x4, Float4x4LookAtTransformEditorNode);
		INIT_SIMPLE_NODE(Transform_PerspectiveProjection_Float4x4, Float4x4PerspectiveTransformEditorNode);
		INIT_SIMPLE_NODE(NormalizeFloat2, NormalizeFloat2EditorNode);
		INIT_SIMPLE_NODE(NormalizeFloat3, NormalizeFloat3EditorNode);
		INIT_SIMPLE_NODE(NormalizeFloat4, NormalizeFloat4EditorNode);
		INIT_SIMPLE_NODE(CrossProductOperation, CrossProductOperationEditorNode);
		INIT_SIMPLE_NODE(ForEachSceneObject, ForEachSceneObjectEditorNode);

		INIT_FLOAT_NODE(Float, FloatEditorNode);
		INIT_FLOAT_NODE(Float2, Float2EditorNode);
		INIT_FLOAT_NODE(Float3, Float3EditorNode);
		INIT_FLOAT_NODE(Float4, Float4EditorNode);

		INIT_FLOAT_MATRIX_NODE(Float4x4, Float4x4EditorNode);

		INIT_BIN_OP_NODE(FloatBinaryOperator, FloatBinaryOperatorEditorNode);
		INIT_BIN_OP_NODE(Float2BinaryOperator, Float2BinaryOperatorEditorNode);
		INIT_BIN_OP_NODE(Float3BinaryOperator, Float3BinaryOperatorEditorNode);
		INIT_BIN_OP_NODE(Float4BinaryOperator, Float4BinaryOperatorEditorNode);
		INIT_BIN_OP_NODE(Float4x4BinaryOperator, Float4x4BinaryOperatorEditorNode);
		INIT_BIN_OP_NODE(FloatComparisonOperator, FloatComparisonOperatorEditorNode);
		INIT_BIN_OP_NODE(BoolBinaryOperator, BoolBinaryOperatorEditorNode);
		INIT_BIN_OP_NODE(IntBinaryOperator, IntBinaryOperatorEditorNode);
		INIT_BIN_OP_NODE(IntComparisonOperator, IntComparisonOperatorEditorNode);

		INIT_INPUT_NODE(OnKeyPressed, OnKeyPressedEditorNode);
		INIT_INPUT_NODE(OnKeyReleased, OnKeyReleasedEditorNode);
		INIT_INPUT_NODE(OnKeyDown, OnKeyDownEditorNode);

	case EditorNodeType::Bool:
	{
		INIT_NODE(BoolEditorNode);
		newNode->m_Value = ReadBoolAttr("Value");
	} break;
	case EditorNodeType::Int:
	{
		INIT_NODE(IntEditorNode);
		newNode->m_Value = ReadIntAttr("Value");
	} break;
	case EditorNodeType::String:
	{
		INIT_NODE(StringEditorNode);
		newNode->m_Value = ReadStrAttr("Value");
	} break;
	case EditorNodeType::GetMesh:
	{
		INIT_NODE(GetMeshEditorNode);
		newNode->m_PositionBit = ReadBoolAttr("PositionBit");
		newNode->m_TexcoordBit = ReadBoolAttr("TexcoordBit");
		newNode->m_NormalBit = ReadBoolAttr("NormalBit");
		newNode->m_TangentBit = ReadBoolAttr("TangentBit");
	} break;
	case EditorNodeType::RenderState:
	{
		INIT_NODE(RenderStateEditorNode);
		newNode->m_DepthWrite = ReadBoolAttr("DepthWrite");
		newNode->m_DepthTestMode = ReadStrAttr("DepthTest");
	} break;
	case EditorNodeType::Pin:
	{
		const bool isInput = ReadBoolAttr("IsInput");
		const PinType pinType = IntToEnum<PinType>(ReadIntAttr("PinType"));
		PinEditorNode* newNode = new PinEditorNode{ isInput, pinType };
		newNode->m_Name = ReadStrAttr("Name");

		node = newNode;
	} break;
	case EditorNodeType::Variable:
	{
		const VariableID id = ReadIntAttr("ID");
		const VariableType type = IntToEnum<VariableType>(ReadIntAttr("Type"));
		node = new VariableEditorNode(id, type);
	} break;
	case EditorNodeType::AsignVariable:
	{
		const VariableID id = ReadIntAttr("ID");
		const VariableType type = IntToEnum<VariableType>(ReadIntAttr("Type"));
		node = new AsignVariableEditorNode(id, type);
	} break;
	case EditorNodeType::Custom:
	{
		CustomEditorNode* customNodeClass = nullptr;
		const std::string customNodeName = ReadStrAttr("CustomNodeName");
		for (const auto cm : customNodes)
		{
			if (customNodeName == cm->GetName())
			{
				customNodeClass = cm;
				break;
			}
		}
		if (customNodeClass == nullptr)
		{
			LOAD_ASSERT();
			m_OperationSuccess = false;
			return nullptr;
		}
		node = new CustomEditorNode(&nodeGraph, customNodeName, customNodeClass->GetNodeGraph(), false);
	} break;
	case EditorNodeType::DEPRECATED_VarBool:
	case EditorNodeType::DEPRECATED_VarInt:
	case EditorNodeType::DEPRECATED_VarFloat:
	case EditorNodeType::DEPRECATED_VarFloat2:
	case EditorNodeType::DEPRECATED_VarFloat3:
	case EditorNodeType::DEPRECATED_VarFloat4:
	case EditorNodeType::DEPRECATED_VarFloat4x4:
	case EditorNodeType::DEPRECATED_GetScene:
	case EditorNodeType::DEPRECATED_GetTexture:
	case EditorNodeType::DEPRECATED_GetShader:
	case EditorNodeType::DEPRECATED_AsignBool:
	case EditorNodeType::DEPRECATED_AsignInt:
	case EditorNodeType::DEPRECATED_AsignFloat:
	case EditorNodeType::DEPRECATED_AsignFloat2:
	case EditorNodeType::DEPRECATED_AsignFloat3:
	case EditorNodeType::DEPRECATED_AsignFloat4:
	case EditorNodeType::DEPRECATED_AsignFloat4x4:
	case EditorNodeType::DEPRECATED_CreateTexture:
	case EditorNodeType::Deprecated:
	{
		node = new DeprecatedEditorNode(nodeType);
	} break;
	case EditorNodeType::DEPRECATED_LoadScene:
	case EditorNodeType::DEPRECATED_LoadTexture:
	case EditorNodeType::DEPRECATED_LoadShader:
	{
		ReadStrAttr("Path");
		node = new DeprecatedEditorNode(nodeType);
	} break;
	default:
		NOT_IMPLEMENTED;
		m_OperationSuccess = false;
		node = new FloatEditorNode{}; // Just some random node to prevent crash
	}

	node->m_ID = ReadIntAttr("ID");
	
	const auto readConstantValue = [this](EditorNodePin& pin)
	{
		switch (pin.Type)
		{
		case PinType::Bool:
			pin.ConstantValue.B = ReadBoolAttr("ConstantValueB");
			break;
		case PinType::Int:
			pin.ConstantValue.I = ReadIntAttr("ConstantValueI");
			break;
		case PinType::Float:
			pin.ConstantValue.F = ReadFloatAttr("ConstantValueF");
			break;
		case PinType::Float2:
			pin.ConstantValue.F2 = ReadFloat2Attr("ConstantValueF2");
			break;
		case PinType::Float3:
			pin.ConstantValue.F3 = ReadFloat3Attr("ConstantValueF3");
			break;
		case PinType::Float4:
			pin.ConstantValue.F4 = ReadFloat4Attr("ConstantValueF4");
			break;
		case PinType::String:
			pin.ConstantValue.STR = ReadStrAttr("ConstantValueSTR");
			break;
		default:
			NOT_IMPLEMENTED;
		}
	};

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
		{
			node->m_Pins[i].ID = ReadIntAttr("ID");

			if (m_Version > 7)
			{
				node->m_Pins[i].HasConstantValue = ReadBoolAttr("HasConstantValue");
				if (node->m_Pins[i].HasConstantValue) readConstantValue(node->m_Pins[i]);
			}
			else
			{
				node->m_Pins[i].HasConstantValue = false;
			}

		}

		EatToken(END_PIN_LIST_TOKEN);
	}

	// Read custom pins
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
			pin.LinkedNode = ReadIntAttr("LinkedNode");

			if (m_Version > 7)
			{
				pin.HasConstantValue = ReadBoolAttr("HasConstantValue");
				if (pin.HasConstantValue) readConstantValue(pin);
			}
			else
			{
				pin.HasConstantValue = false;
			}

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

CustomEditorNode* NodeGraphSerializer::ReadCustomNode(const std::vector<CustomEditorNode*>& customNodes)
{
	EatToken(BEGIN_NODE_TOKEN);

	const std::string name = ReadStrAttr("Name");
	
	NodeGraph* nodeGraph = new NodeGraph{};
	ReadNodeGraph(*nodeGraph, customNodes);

	EatToken(END_NODE_TOKEN);

	return new CustomEditorNode{ nullptr, name, nodeGraph };
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
		// m_OperationSuccess = false;
		// return "";
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

Float2 NodeGraphSerializer::ReadFloat2Attr(const std::string& name)
{
	Float2 value;
	value.x = ReadFloatAttr(name + ".X");
	value.y = ReadFloatAttr(name + ".Y");
	return value;
}

Float3 NodeGraphSerializer::ReadFloat3Attr(const std::string& name)
{
	Float3 value;
	value.x = ReadFloatAttr(name + ".X");
	value.y = ReadFloatAttr(name + ".Y");
	value.z = ReadFloatAttr(name + ".Z");
	return value;
}

Float4 NodeGraphSerializer::ReadFloat4Attr(const std::string& name)
{
	Float4 value;
	value.x = ReadFloatAttr(name + ".X");
	value.y = ReadFloatAttr(name + ".Y");
	value.z = ReadFloatAttr(name + ".Z");
	value.w = ReadFloatAttr(name + ".W");
	return value;
}

Float4x4 NodeGraphSerializer::ReadFloat4x4Attr(const std::string& name)
{
	Float4x4 value;
	value[0][0] = ReadFloatAttr(name + ".00");
	value[0][1] = ReadFloatAttr(name + ".01");
	value[0][2] = ReadFloatAttr(name + ".02");
	value[1][0] = ReadFloatAttr(name + ".10");
	value[1][1] = ReadFloatAttr(name + ".11");
	value[1][2] = ReadFloatAttr(name + ".12");
	value[2][0] = ReadFloatAttr(name + ".20");
	value[2][1] = ReadFloatAttr(name + ".21");
	value[2][2] = ReadFloatAttr(name + ".22");
	return value;
}

TextureData NodeGraphSerializer::ReadTextureDataAttr(const std::string& name)
{
	TextureData value;
	value.Path = ReadStrAttr(name + ".Path");
	value.Width = ReadIntAttr(name + ".Width");
	value.Height = ReadIntAttr(name + ".Height");
	value.Framebuffer = ReadBoolAttr(name + ".Framebuffer");
	value.DepthStencil = ReadBoolAttr(name + ".DepthStencil");
	return value;
}

SceneData NodeGraphSerializer::ReadSceneDataAttr(const std::string& name)
{
	SceneData value;
	value.Path = ReadStrAttr(name + ".Path");
	return value;
}

ShaderData NodeGraphSerializer::ReadShaderDataAttr(const std::string& name)
{
	ShaderData value;
	value.Path = ReadStrAttr(name + ".Path");
	return value;
}
