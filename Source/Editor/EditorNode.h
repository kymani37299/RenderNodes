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
    LoadScene,
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
    OnKeyPressed,
    OnKeyReleased,
    OnKeyDown,
    Int,
    VarInt,
    AsignInt,
    IntBinaryOperator,
    IntComparisonOperator,
    Float4x4,
    Transform_Rotate_Float4x4,
    VarFloat4x4,
    AsignFloat4x4,
	Transform_Translate_Float4x4,
	Transform_Scale_Float4x4,
	Transform_LookAt_Float4x4,
	Transform_PerspectiveProjection_Float4x4,
    Float4x4BinaryOperator,
    GetScene,
    ForEachSceneObject,
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
    Int,
    Float4x4,
    SceneObject,
    Scene,
    Any,
    
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
    case PinType::Int: return "Int";
    case PinType::Float4x4: return "Float4x4";
    case PinType::SceneObject: return "SceneObject";
    case PinType::Scene: return "Scene";
    case PinType::Any: return "Any";
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
    case PinType::Int: return ImColor(200, 100, 100);
    case PinType::Float4x4: return ImColor(184, 240, 153);
    case PinType::SceneObject: return ImColor(255, 153, 51);
    case PinType::Scene: return ImColor(240, 120, 51);
    case PinType::Any: return ImColor(100, 100, 100);
    default:
        NOT_IMPLEMENTED;
        break;
    }
    return ImColor(0, 0, 0, 255);
}

struct EditorNodePinConstant
{
	union
	{
		bool B;
        int I;
        float F;
		Float2 F2;
		Float3 F3;
		Float4 F4;
	};
	std::string STR;

    void SetDefaultValue(PinType pinType)
    {
        switch (pinType)
        {
        case PinType::Bool:
            B = false;
            break;
        case PinType::Int:
            I = 0;
            break;
        case PinType::Float:
            F = 0.0f;
            break;
        case PinType::Float2:
            F2 = Float2{ 0.0f };
            break;
        case PinType::Float3:
            F3 = Float3{ 0.0f };
            break;
        case PinType::Float4:
            F4 = Float4{ 0.0f };
            break;
        case PinType::String:
            STR = "";
            break;
        default:
            NOT_IMPLEMENTED;
        }
    }
};

struct EditorNodePin
{
    bool IsInput = false;
    PinType Type = PinType::Invalid;
    PinID ID = 0;
    std::string Label = "";
    NodeID LinkedNode = 0;

    bool HasConstantValue = false;
    EditorNodePinConstant ConstantValue;

    static EditorNodePin CreateConstantInputPin(const std::string& label, PinType type);
    static EditorNodePin CreateInputPin(const std::string& label, PinType type);
    static EditorNodePin CreateOutputPin(const std::string& label, PinType type);

    static bool CanBeLinked(const EditorNodePin& a, const EditorNodePin& b);
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

    void UpdatePin(const EditorNodePin& newPin);
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