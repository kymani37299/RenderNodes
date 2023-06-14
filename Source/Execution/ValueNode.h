#pragma once

#include "../Util/Hash.h"
#include "ExecuteContext.h"

template<typename T>
class ValueNode
{
public:
	virtual T GetValue(ExecuteContext& context) const = 0;
};

using BoolValueNode = ValueNode<bool>;
using FloatValueNode = ValueNode<float>;
using Float2ValueNode = ValueNode<glm::vec2>;
using Float3ValueNode = ValueNode<glm::vec3>;
using Float4ValueNode = ValueNode<glm::vec4>;
using TextureValueNode = ValueNode<Texture*>;
using BufferValueNode = ValueNode<Buffer*>;
using MeshValueNode = ValueNode<Mesh*>;
using ShaderValueNode = ValueNode<Shader*>;

template<typename T>
class ConstantValueNode : public ValueNode<T>
{
public:
	ConstantValueNode(const T& value) :
		m_Value(value) {}

	virtual T GetValue(ExecuteContext& context) const override { return m_Value; }

private:
	T m_Value;
};

template<typename T, ExecutorStaticResource staticResource>
class StaticResourceNode : public ValueNode<T>
{
public:
	virtual T GetValue(ExecuteContext& context) const override { return context.RenderResources.GetStaticResource<T, staticResource>(); }
};

template<typename T>
class BinaryOperatorValueNode : public ValueNode<T>
{
public:
	BinaryOperatorValueNode(ValueNode<T>* a, ValueNode<T>* b, char op) :
		m_A(Ptr<ValueNode<T>>(a)),
		m_B(Ptr<ValueNode<T>>(b)),
		m_Op(op) {}

	virtual T GetValue(ExecuteContext& context) const override
	{
		if (!m_A || !m_B)
		{
			ExecutionPrivate::Failure("BinaryOperatorValueNode", "Binary operator have missing values!");
			context.Failure = true;
			return T{};
		}

		const T a = m_A->GetValue(context);
		const T b = m_B->GetValue(context);

		switch (m_Op)
		{
		case '+':
			return a + b;
		case '-':
			return a - b;
		case '/':
			return a / b;
		case '*':
			return a * b;
		}
		NOT_IMPLEMENTED;
		return a;
	}

private:
	Ptr<ValueNode<T>> m_A;
	Ptr<ValueNode<T>> m_B;
	char m_Op;
};

template<typename T>
class VariableValueNode : public ValueNode<T>
{
public:
	VariableValueNode(const std::string& varName) :
		m_VarKey(Hash::Crc32(varName)) {}

	virtual T GetValue(ExecuteContext& context) const override
	{
		auto& variableMap = context.Variables.GetMapFromType<T>();
		if (variableMap.find(m_VarKey) == variableMap.end())
		{
			ExecutionPrivate::Failure("VariableValueNode", "Variable not declared");
			context.Failure = true;
			return T{};
		}
		return variableMap[m_VarKey];
	}

private:
	uint32_t m_VarKey;
};