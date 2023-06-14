#pragma once

#include "../Common.h"
#include "../IDGen.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <inttypes.h>

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
    default: NOT_IMPLEMENTED;
    }
    return "<unknown>";
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
    EditorNode(const std::string& label, EditorNodeType nodeType);

    NodeID GetID() const { return m_ID; }
    EditorNodeType GetType() const { return m_Type; }
    const std::vector<EditorNodePin>& GetPins() const { return m_Pins; }

    void Render();

protected:
    virtual void RenderContent();

    unsigned AddPin(const EditorNodePin& pin);

private:
    std::string m_Label;
    EditorNodeType m_Type;
    NodeID m_ID = 0;
    std::vector<EditorNodePin> m_Pins;
};