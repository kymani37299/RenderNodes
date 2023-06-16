#pragma once

#include "../Util/Hash.h"
#include "ExecuteContext.h"

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
	const ValueNodeExtraInfo& GetExtraInfo() const { return m_ExtraInfo; }

protected:
	ValueNodeExtraInfo m_ExtraInfo;
};

using BoolValueNode = ValueNode<bool>;
using FloatValueNode = ValueNode<float>;
using Float2ValueNode = ValueNode<Float2>;
using Float3ValueNode = ValueNode<Float3>;
using Float4ValueNode = ValueNode<Float4>;
using TextureValueNode = ValueNode<Texture*>;
using MeshValueNode = ValueNode<Mesh*>;
using BufferValueNode = ValueNode<Buffer*>;
using ShaderValueNode = ValueNode<Shader*>;

struct BindTable
{
	template<typename T>
	struct Binding
	{
		std::string Name;
		Ptr<ValueNode<T>> Value;
	};
	std::vector<Binding<Texture*>> Textures;
	std::vector<Binding<float>> Floats;
	std::vector<Binding<Float2>> Float2s;
	std::vector<Binding<Float3>> Float3s;
	std::vector<Binding<Float4>> Float4s;
};
using BindTableValueNode = ValueNode<BindTable*>;

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

template<typename T>
class ConstantPtrValueNode : public ValueNode<T*>
{
public:
	ConstantPtrValueNode(T* value) :
		m_Value(value) {}

	virtual T* GetValue(ExecuteContext& context) const override { return m_Value.get(); }

private:
	Ptr<T> m_Value;
};

template<typename T, ExecutorStaticResource staticResource>
class StaticResourceNode : public ValueNode<T>
{
public:
	StaticResourceNode() = default;
	StaticResourceNode(const ValueNodeExtraInfo& extraInfo):
		ValueNode<T>(extraInfo) {}

public:
	virtual T GetValue(ExecuteContext& context) const override { return context.RenderResources.GetStaticResource<T, staticResource>(); }
};

template<typename T>
class BinaryArithmeticOperatorValueNode : public ValueNode<T>
{
public:
	BinaryArithmeticOperatorValueNode(ValueNode<T>* a, ValueNode<T>* b, char op) :
		m_A(Ptr<ValueNode<T>>(a)),
		m_B(Ptr<ValueNode<T>>(b)),
		m_Op(op) {}

	virtual T GetValue(ExecuteContext& context) const override
	{
		if (!m_A || !m_B)
		{
			ExecutionPrivate::Failure("BinaryArithmeticOperatorValueNode", "Binary operator have missing values!");
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
class ComparisonValueNode : public ValueNode<bool>
{
public:
	ComparisonValueNode(ValueNode<T>* a, ValueNode<T>* b, const std::string& op) :
		m_A(a),
		m_B(b),
		m_Op(op) {}

	virtual bool GetValue(ExecuteContext& context) const
	{
		if (!m_A || !m_B)
		{
			ExecutionPrivate::Failure("ComparisonValueNode", "Comparison operator have missing values!");
			context.Failure = true;
			return false;
		}

		const T a = m_A->GetValue(context);
		const T b = m_B->GetValue(context);

		if (m_Op == "==") return a == b;
		if (m_Op == "!=") return a != b;
		if (m_Op == ">") return a > b;
		if (m_Op == "<") return a < b;
		if (m_Op == ">=") return a >= b;
		if (m_Op == "<=") return a <= b;

		NOT_IMPLEMENTED;
		return false;
	}

private:
	Ptr<ValueNode<T>> m_A;
	Ptr<ValueNode<T>> m_B;
	std::string m_Op;
};

class BoolBinaryOperatorValueNode : public BoolValueNode
{
public:
	BoolBinaryOperatorValueNode(BoolValueNode* a, BoolValueNode* b, const std::string& op) :
		m_A(a),
		m_B(b),
		m_Op(op) {}

	virtual bool GetValue(ExecuteContext& context) const
	{
		if (!m_A || !m_B)
		{
			ExecutionPrivate::Failure("BoolBinaryOperatorValueNode", "Bool operator have missing values!");
			context.Failure = true;
			return false;
		}

		const bool a = m_A->GetValue(context);
		const bool b = m_B->GetValue(context);

		if (m_Op == "AND") return a && b;
		if (m_Op == "OR") return a || b;
		if (m_Op == "XOR") return (a && !b) || (!a && b);

		NOT_IMPLEMENTED;
		return false;
	}

private:
	Ptr<BoolValueNode> m_A;
	Ptr<BoolValueNode> m_B;
	std::string m_Op;
};

template<typename T>
class VariableValueNode : public ValueNode<T>
{
public:
	VariableValueNode(const std::string& varName, const ValueNodeExtraInfo& extraInfo):
		ValueNode<T>(extraInfo),
		m_VarKey(Hash::Crc32(varName)) {}

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

template<typename T, typename U, unsigned vecSize>
class CreateVectorValueNode : public ValueNode<T>
{
	static_assert(vecSize > 1 && vecSize <= 4);
public:

	CreateVectorValueNode(std::vector<ValueNode<U>*> inputs)
	{
		for (unsigned i = 0; i < inputs.size(); i++)
			m_Values[i] = Ptr<ValueNode<U>>(inputs[i]);
	}

	template<unsigned numParameters> T CreateT(ExecuteContext& context) const;
	template<> T CreateT<2>(ExecuteContext& context) const { return T{ m_Values[0]->GetValue(context), m_Values[1]->GetValue(context) }; }
	template<> T CreateT<3>(ExecuteContext& context) const { return T{ m_Values[0]->GetValue(context), m_Values[1]->GetValue(context), m_Values[2]->GetValue(context) }; }
	template<> T CreateT<4>(ExecuteContext& context) const { return T{ m_Values[0]->GetValue(context), m_Values[1]->GetValue(context), m_Values[2]->GetValue(context), m_Values[3]->GetValue(context) }; }

	virtual T GetValue(ExecuteContext& context) const override
	{
		return CreateT<vecSize>(context);
	}

private:
	Ptr<ValueNode<U>> m_Values[4];
};

template<typename U, typename T>
class SplitVectorValueNode : public ValueNode<T>
{
public:
	SplitVectorValueNode(ValueNode<U>* value, unsigned vecIndex) :
		m_Value(value),
		m_VecIndex(vecIndex) 
	{
		ASSERT(vecIndex <= 4);
	}

	virtual T GetValue(ExecuteContext& context) const override
	{
		if (!m_Value)
		{
			ExecutionPrivate::Failure("SplitVectorValueNode", "Missing input!");
			context.Failure = true;
			return T{};
		}
		return m_Value->GetValue(context)[m_VecIndex];
	}

private:
	unsigned m_VecIndex;
	Ptr<ValueNode<U>> m_Value;
};