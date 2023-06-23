#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <inttypes.h>
#include <typeindex>
#include <typeinfo>

#include "../Common.h"
#include "../IDGen.h"

// DO NOT CHANGE ORDER OF VALUES
// IT WILL AFFECT HOW WE LOAD OLD SAVE FILES
// ALWAYS APPEND ON END
enum class EditorNodeType
{
    Invalid,

	OnUpdate,
	OnStart,
    Bool,
    Float,
	Float2,
	Float3,
	Float4,
    AsignFloat,
    AsignFloat2,
    AsignFloat3,
    AsignFloat4,
    VarFloat,
    VarFloat2,
    VarFloat3,
    VarFloat4,
	FloatBinaryOperator,
	Float2BinaryOperator,
	Float3BinaryOperator,
	Float4BinaryOperator,
	If,
	Print,
    ClearRenderTarget,
    CreateTexture,
    GetTexture,
    PresentTexture,
    LoadTexture,
    LoadShader,
    GetCubeMesh,
    DrawMesh,
    GetShader,
    BindTable,
    GetMesh,
    LoadMesh,
    FloatComparisonOperator,
    CreateFloat2,
    CreateFloat3,
    CreateFloat4,
    SplitFloat2,
    SplitFloat3,
    SplitFloat4,
    BoolBinaryOperator,
    VarBool,
    AsignBool,
    RenderState,
    String,
    Pin,
    Custom,
    UnresolvedCustom,
};

// DO NOT CHANGE ORDER OF VALUES
// IT WILL AFFECT HOW WE LOAD OLD SAVE FILES
// ALWAYS APPEND ON END
enum class PinType
{
    Invalid,

    Execution,
    Bool,
    Float,
    Float2,
    Float3,
    Float4,
    Texture,
    Buffer,
    Mesh,
    Shader,
    BindTable,
    RenderState,
    String,

    Count
};

static const std::string ToString(PinType pinType)
{
    switch (pinType)
    {
    case PinType::Invalid: return "Invalid";
    case PinType::Execution: return "Execution";
    case PinType::Bool: return "Bool";
    case PinType::Float: return "Float";
    case PinType::Float2: return "Float2";
    case PinType::Float3: return "Float3";
    case PinType::Float4: return "Float4";
    case PinType::Texture: return "Texture";
    case PinType::Buffer: return "Buffer";
    case PinType::Mesh: return "Mesh";
    case PinType::Shader: return "Shader";
    case PinType::BindTable: return "BindTable";
    case PinType::RenderState: return "RenderState";
    case PinType::String: return "String";
    default: NOT_IMPLEMENTED;
    }
    return "<unknown>";
}

static ImColor GetPinColor(PinType type)
{
    switch (type)
    {
    case PinType::Invalid: return ImColor(1.0f, 0.0f, 0.0f, 1.0f);
    case PinType::Execution: return ImColor(1.0f, 1.0f, 1.0f, 1.0f);
    case PinType::Bool: return ImColor(179, 112, 176);
    case PinType::Float: 
    case PinType::Float2:
    case PinType::Float3:
    case PinType::Float4: return ImColor(184, 216, 186);
    case PinType::Texture: return ImColor(212, 19, 108);
    case PinType::Buffer: return ImColor(48, 107, 172);
    case PinType::Mesh: return ImColor(145, 142, 244);
    case PinType::Shader: return ImColor(184, 77, 0);
    case PinType::BindTable: return ImColor(206, 171, 177);
    case PinType::RenderState: return ImColor(165, 190, 0);
    case PinType::String: return ImColor(192, 110, 82);
    default:
        NOT_IMPLEMENTED;
        break;
    }
    return ImColor(0, 0, 0, 255);
}

struct EditorNodePin
{
    bool IsInput = false;
    PinType Type = PinType::Invalid;
    PinID ID = 0;
    std::string Label = "";

    static EditorNodePin CreateInputPin(const std::string& label, PinType type);
    static EditorNodePin CreateOutputPin(const std::string& label, PinType type);
};

struct EditorNodeLink
{
    LinkID ID;
    PinID Start;
    PinID End;
};

class NodeGraphSerializer;
#define SERIALIZEABLE_EDITOR_NODE() friend class NodeGraphSerializer

class EditorNode
{
    SERIALIZEABLE_EDITOR_NODE();
public:
    // Using it to get metadata from specific class
    static std::unordered_map<std::type_index, EditorNode*> s_ClassRepresents;

    template<typename T>
    static EditorNode* GetClassRepresent()
    {
        if (s_ClassRepresents.find(typeid(T)) == s_ClassRepresents.end())
            s_ClassRepresents[typeid(T)] = new T{};
        return s_ClassRepresents[typeid(T)];
    }

public:
    EditorNode(const std::string& label, EditorNodeType nodeType);
    
    virtual EditorNode* Clone() const = 0;

    NodeID GetID() const { return m_ID; }
    EditorNodeType GetType() const { return m_Type; }
    const std::vector<EditorNodePin>& GetPins() const { return m_Pins; }
    const std::vector<EditorNodePin>& GetCustomPins() const { return m_CustomPins; }

    void RemovePin(PinID pinID);

    void Render();
    virtual void RenderPopups() {}

    unsigned AddCustomPin(const EditorNodePin& pin);

protected:
    virtual void RenderContent();

    unsigned AddPin(const EditorNodePin& pin);

private:
    std::string m_Label;
    EditorNodeType m_Type;
    NodeID m_ID = 0;
    std::vector<EditorNodePin> m_Pins;
    std::vector<EditorNodePin> m_CustomPins;
};