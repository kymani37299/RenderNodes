#pragma once

#include "../Common.h"

class ExecutorNode
{
public:
	virtual void Execute() = 0;

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
	void Execute() override {}
};

template<typename T>
class ValueNode
{
public:
	virtual T GetValue() const = 0;
};

template<typename T>
class ConstantValueNode : public ValueNode<T>
{
public:
	ConstantValueNode(const T& value) :
		m_Value(value) {}

	virtual T GetValue() const override { return m_Value; }

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

	virtual T GetValue() const override
	{
		const T a = m_A->GetValue();
		const T b = m_B->GetValue();

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

class IfExecutorNode : public ExecutorNode
{
public:
	IfExecutorNode(ValueNode<bool>* conditionNode, ExecutorNode* elseBranch) :
		m_Condition(Ptr<ValueNode<bool>>(conditionNode)),
		m_Else(Ptr<ExecutorNode>(elseBranch)) {}

	void Execute() override;

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

	void Execute() override;

private:
	Ptr<ValueNode<float>> m_FloatNode;
};