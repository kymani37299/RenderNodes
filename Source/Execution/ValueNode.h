#pragma once

#include <vector>
#include <string>

class CodeGenerator;
struct ExecuteContext;

struct ValueNodeExtraInfo
{
	struct VertexBits
	{
		bool Position : 1;
		bool Texcoord : 1;
		bool Normal : 1;
		bool Tangent : 1;
	} MeshVertexBits;
};

template<typename T>
class ValueNode
{
public:
	ValueNode() = default;
	ValueNode(const ValueNodeExtraInfo& extraInfo):
		m_ExtraInfo(extraInfo) {}

	virtual T GetValue(ExecuteContext& context) const = 0;
	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const = 0;

	const ValueNodeExtraInfo& GetExtraInfo() const { return m_ExtraInfo; }

protected:
	ValueNodeExtraInfo m_ExtraInfo;
};

struct Mesh;
struct Texture;
struct Buffer;
struct Shader;
struct SceneObject;
struct Scene;
struct RenderState;
struct BindTable;

using BoolValueNode = ValueNode<bool>;
using IntValueNode = ValueNode<int>;
using StringValueNode = ValueNode<std::string>;
using FloatValueNode = ValueNode<float>;
using Float2ValueNode = ValueNode<Float2>;
using Float3ValueNode = ValueNode<Float3>;
using Float4ValueNode = ValueNode<Float4>;
using Float4x4ValueNode = ValueNode<Float4x4>;
using TextureValueNode = ValueNode<Texture*>;
using MeshValueNode = ValueNode<Mesh*>;
using BufferValueNode = ValueNode<Buffer*>;
using ShaderValueNode = ValueNode<Shader*>;
using SceneObjectValueNode = ValueNode<SceneObject*>;
using SceneValueNode = ValueNode<Scene*>;
using RenderStateValueNode = ValueNode<RenderState>;
using BindTableValueNode = ValueNode<BindTable*>;