#pragma once

#include "../Common.h"
#include "../Util/Hash.h"
#include "../Render/Texture.h"

#include <unordered_map>

struct VariableBlock
{
	std::unordered_map<uint32_t, float> Floats;
	std::unordered_map<uint32_t, glm::vec2> Float2s;
	std::unordered_map<uint32_t, glm::vec3> Float3s;
	std::unordered_map<uint32_t, glm::vec4> Float4s;

	template<typename T> std::unordered_map<uint32_t, T>& GetMapFromType();
	template<> std::unordered_map<uint32_t, float>& GetMapFromType<float>() { return Floats; }
	template<> std::unordered_map<uint32_t, glm::vec2>& GetMapFromType<glm::vec2>() { return Float2s; }
	template<> std::unordered_map<uint32_t, glm::vec3>& GetMapFromType<glm::vec3>() { return Float3s; }
	template<> std::unordered_map<uint32_t, glm::vec4>& GetMapFromType<glm::vec4>() { return Float4s; }
};

struct ExecuteContext
{
	Ptr<Texture> RenderTarget;
	VariableBlock Variables;
};

class ExecutorNode
{
public:
	virtual void Execute(ExecuteContext& context) = 0;

	void SetNextNode(ExecutorNode* node)
	{
		m_NextNode = Ptr<ExecutorNode>(node);
	}

	virtual ExecutorNode* GetNextNode() const
	{
		return m_NextNode.get();
	}

private:
	Ptr<ExecutorNode> m_NextNode;
};

struct CompiledPipeline
{
	ExecutorNode* OnStartNode = nullptr;
	ExecutorNode* OnUpdateNode = nullptr;
};

class EmptyExecutorNode : public ExecutorNode
{
public:
	void Execute(ExecuteContext& context) override {}
};

template<typename T>
class ValueNode
{
public:
	virtual T GetValue(ExecuteContext& context) const = 0;
};

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
class BinaryOperatorValueNode : public ValueNode<T>
{
public:
	BinaryOperatorValueNode(ValueNode<T>* a, ValueNode<T>* b, char op) :
		m_A(Ptr<ValueNode<T>>(a)),
		m_B(Ptr<ValueNode<T>>(b)),
		m_Op(op) {}

	virtual T GetValue(ExecuteContext& context) const override
	{
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
		return context.Variables.GetMapFromType<T>()[m_VarKey];
	}

private:
	uint32_t m_VarKey;
};

class IfExecutorNode : public ExecutorNode
{
public:
	IfExecutorNode(ValueNode<bool>* conditionNode, ExecutorNode* elseBranch) :
		m_Condition(Ptr<ValueNode<bool>>(conditionNode)),
		m_Else(Ptr<ExecutorNode>(elseBranch)) {}

	void Execute(ExecuteContext& context) override;

	ExecutorNode* GetNextNode() const override
	{
		if (m_PassedCondition)
			return ExecutorNode::GetNextNode();
		else
			return m_Else.get();
	}

private:
	bool m_PassedCondition = false;

	Ptr<ValueNode<bool>> m_Condition;
	Ptr<ExecutorNode> m_Else;
};

class PrintExecutorNode : public ExecutorNode
{
public:
	PrintExecutorNode(ValueNode<float>* floatNode):
		m_FloatNode(Ptr<ValueNode<float>>(floatNode)) {}

	PrintExecutorNode(ValueNode<glm::vec2>* floatNode) :
		m_Float2Node(Ptr<ValueNode<glm::vec2>>(floatNode)) {}

	PrintExecutorNode(ValueNode<glm::vec3>* floatNode) :
		m_Float3Node(Ptr<ValueNode<glm::vec3>>(floatNode)) {}

	PrintExecutorNode(ValueNode<glm::vec4>* floatNode) :
		m_Float4Node(Ptr<ValueNode<glm::vec4>>(floatNode)) {}

	void Execute(ExecuteContext& context) override;

private:
	Ptr<ValueNode<float>> m_FloatNode;
	Ptr<ValueNode<glm::vec2>> m_Float2Node;
	Ptr<ValueNode<glm::vec3>> m_Float3Node;
	Ptr<ValueNode<glm::vec4>> m_Float4Node;
};

class ClearRenderTargetExecutorNode : public ExecutorNode
{
public:
	ClearRenderTargetExecutorNode(ValueNode<glm::vec4>* clearColorNode) :
		m_ClearColorNode(clearColorNode) {}

	void Execute(ExecuteContext& context) override;

private:
	Ptr<ValueNode<glm::vec4>> m_ClearColorNode;
};

template<typename T>
class AsignVariableExecutorNode : public ExecutorNode
{
public:
	AsignVariableExecutorNode(const std::string& variableName, ValueNode<T>* value):
		m_VariableKey(Hash::Crc32(variableName)),
		m_InitialValueNode(Ptr<ValueNode<T>>(value)) {}

	void Execute(ExecuteContext& context) override
	{
		context.Variables.GetMapFromType<T>()[m_VariableKey] = m_InitialValueNode->GetValue(context);
	}

private:
	uint32_t m_VariableKey;
	Ptr<ValueNode<T>> m_InitialValueNode;
};