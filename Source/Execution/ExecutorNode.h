#pragma once

#include <variant>

#include "../Common.h"
#include "../Util/Hash.h"

#include "ExecutorNodeVisitor.h"
#include "ValueNode.h"

class ExecutorNode
{
public:
	virtual ~ExecutorNode() {}

	virtual void Accept(ExecutorNodeVisitor& visitor) = 0;

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

class EmptyExecutorNode : public ExecutorNode
{
public:
	void Accept(ExecutorNodeVisitor& visitor) override { visitor.Visit(*this); }
};

class IfExecutorNode : public ExecutorNode
{
public:
	IfExecutorNode(BoolValueNode* conditionNode, ExecutorNode* elseBranch) :
		m_Condition(conditionNode),
		m_Else(elseBranch) {}

	void Accept(ExecutorNodeVisitor& visitor) override { visitor.Visit(*this); }

	ExecutorNode* GetNextNode() const override
	{
		if (m_PassedCondition)
			return ExecutorNode::GetNextNode();
		else
			return m_Else.get();
	}

	void SetCondition(bool condition) { m_PassedCondition = condition; }

	BoolValueNode* GetConditionNode() { return m_Condition.get(); }
	ExecutorNode* GetElseExecutorNode() { return m_Else.get(); }

private:
	bool m_PassedCondition = false;

	Ptr<BoolValueNode> m_Condition;
	Ptr<ExecutorNode> m_Else;
};

class PrintExecutorNode : public ExecutorNode
{
public:
	PrintExecutorNode(FloatValueNode* floatNode):
		m_FloatNode(floatNode) {}

	PrintExecutorNode(Float2ValueNode* floatNode) :
		m_Float2Node(floatNode) {}

	PrintExecutorNode(Float3ValueNode* floatNode) :
		m_Float3Node(floatNode) {}

	PrintExecutorNode(Float4ValueNode* floatNode) :
		m_Float4Node(floatNode) {}

	PrintExecutorNode(IntValueNode* intNode) :
		m_IntNode(intNode) {}

	PrintExecutorNode(BoolValueNode* boolNode) :
		m_BoolNode(boolNode) {}

	PrintExecutorNode(StringValueNode* stringNode) :
		m_StringNode(stringNode) {}

	void Accept(ExecutorNodeVisitor& visitor) override { visitor.Visit(*this); }

	FloatValueNode* GetFloatValueNode() { return m_FloatNode.get(); }
	Float2ValueNode* GetFloat2ValueNode() { return m_Float2Node.get(); }
	Float3ValueNode* GetFloat3ValueNode() { return m_Float3Node.get(); }
	Float4ValueNode* GetFloat4ValueNode() { return m_Float4Node.get(); }
	IntValueNode* GetIntValueNode() { return m_IntNode.get(); }
	BoolValueNode* GetBoolValueNode() { return m_BoolNode.get(); }
	StringValueNode* GetStringValueNode() { return m_StringNode.get(); }

private:
	Ptr<FloatValueNode> m_FloatNode;
	Ptr<Float2ValueNode> m_Float2Node;
	Ptr<Float3ValueNode> m_Float3Node;
	Ptr<Float4ValueNode> m_Float4Node;
	Ptr<IntValueNode> m_IntNode;
	Ptr<BoolValueNode> m_BoolNode;
	Ptr<StringValueNode> m_StringNode;
};

class ClearRenderTargetExecutorNode : public ExecutorNode
{
public:
	ClearRenderTargetExecutorNode(TextureValueNode* textureNode, Float4ValueNode* clearColorNode) :
		m_TextureNode(textureNode),
		m_ClearColorNode(clearColorNode) {}

	void Accept(ExecutorNodeVisitor& visitor) override { visitor.Visit(*this); }

	Float4ValueNode* GetClearColorNode() { return m_ClearColorNode.get(); }
	TextureValueNode* GetTextureNode() { return m_TextureNode.get(); }

private:
	Ptr<Float4ValueNode> m_ClearColorNode;
	Ptr<TextureValueNode> m_TextureNode;
};

class AsignVariableExecutorNode : public ExecutorNode
{
public:
	using ValueNodeVariant = std::variant<
		Ptr<ValueNode<int>>,
		Ptr<ValueNode<float>>,
		Ptr<ValueNode<std::string>>,
		Ptr<ValueNode<bool>>,
		Ptr<ValueNode<Float2>>,
		Ptr<ValueNode<Float3>>,
		Ptr<ValueNode<Float4>>,
		Ptr<ValueNode<Float4x4>> 
	>;

	template<typename T>
	AsignVariableExecutorNode(VariableID variableID, ValueNode<T>* value):
		m_VariableID(variableID),
		m_InitialValueNode(Ptr<ValueNode<T>>(value)) 
	{
		static_assert(
			std::disjunction_v<
			std::is_same<Ptr<ValueNode<T>>, Ptr<ValueNode<int>>>,
			std::is_same<Ptr<ValueNode<T>>, Ptr<ValueNode<float>>>,
			std::is_same<Ptr<ValueNode<T>>, Ptr<ValueNode<std::string>>>,
			std::is_same<Ptr<ValueNode<T>>, Ptr<ValueNode<bool>>>,
			std::is_same<Ptr<ValueNode<T>>, Ptr<ValueNode<Float2>>>,
			std::is_same<Ptr<ValueNode<T>>, Ptr<ValueNode<Float3>>>,
			std::is_same<Ptr<ValueNode<T>>, Ptr<ValueNode<Float4>>>,
			std::is_same<Ptr<ValueNode<T>>, Ptr<ValueNode<Float4x4>>>
			> ,
			"Type T is not supported in AssignVariableExecutorNode variant! Add it to ValueNodeVariant."
			);
	}

	void Accept(ExecutorNodeVisitor& visitor) override { visitor.Visit(*this); }

	VariableID GetVariableID() { return m_VariableID; }

	template<typename T>
	ValueNode<T>* GetValueNode() { return std::get<Ptr<ValueNode<T>>>(m_InitialValueNode).get(); }

private:
	VariableID m_VariableID = 0;
	ValueNodeVariant m_InitialValueNode;
};

class PresentTextureExecutorNode : public ExecutorNode
{
public:
	PresentTextureExecutorNode(TextureValueNode* texture):
		m_Texture(texture)
	{ }

	void Accept(ExecutorNodeVisitor& visitor) override { visitor.Visit(*this); }

	TextureValueNode* GetTextureNode() { return m_Texture.get(); }

private:
	Ptr<TextureValueNode> m_Texture;
};

class DrawMeshExecutorNode : public ExecutorNode
{
public:
	DrawMeshExecutorNode(TextureValueNode* framebufferNode, ShaderValueNode* shaderNode, MeshValueNode* meshNode, BindTableValueNode* bindTable, RenderStateValueNode* renderState):
		m_FramebufferNode(framebufferNode),
		m_ShaderNode(shaderNode),
		m_MeshNode(meshNode),
		m_BindTable(bindTable),
		m_RenderState(renderState) {}

	~DrawMeshExecutorNode();

	void Accept(ExecutorNodeVisitor& visitor) override { visitor.Visit(*this); }

	TextureValueNode* GetFramebufferNode() { return m_FramebufferNode.get(); }
	ShaderValueNode* GetShaderNode() { return m_ShaderNode.get(); }
	MeshValueNode* GetMeshNode() { return m_MeshNode.get(); }
	BindTableValueNode* GetBindTableNode() { return m_BindTable.get(); }
	RenderStateValueNode* GetRenderStateNode() { return m_RenderState.get(); }

	unsigned GetVAO() { return m_VAO; }
	void SetVAO(unsigned vao) { m_VAO = vao; }

private:
	unsigned m_VAO = 0; // TODO: Visitor or executor should own resourrces

	Ptr<TextureValueNode> m_FramebufferNode;
	Ptr<ShaderValueNode> m_ShaderNode;
	Ptr<MeshValueNode> m_MeshNode;
	Ptr<BindTableValueNode> m_BindTable;
	Ptr<RenderStateValueNode> m_RenderState;
};

class ForEachSceneObjectExecutorNode : public ExecutorNode
{
public:
	ForEachSceneObjectExecutorNode(SceneValueNode* sceneNode, PinID iteratorPin, ExecutorNode* loopExecutorNode) :
		m_SceneNode(sceneNode),
		m_IteratorPin(iteratorPin),
		m_LoopExecutorNode(loopExecutorNode)
	{ }

	void Accept(ExecutorNodeVisitor& visitor) override { visitor.Visit(*this); }

	PinID GetIteratorPin() { return m_IteratorPin; }
	SceneValueNode* GetSceneNode() { return m_SceneNode.get(); }
	ExecutorNode* GetLoopExecutorNode() { return m_LoopExecutorNode.get(); }

private:
	PinID m_IteratorPin;
	Ptr<SceneValueNode> m_SceneNode;
	Ptr<ExecutorNode> m_LoopExecutorNode;
};