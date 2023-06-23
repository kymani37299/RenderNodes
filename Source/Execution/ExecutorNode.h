#pragma once

#include "../Common.h"
#include "../Util/Hash.h"

#include "ValueNode.h"
#include "ExecuteContext.h"

class ExecutorNode
{
public:
	virtual ~ExecutorNode() {}

	// If node reutrns false it means failure and execution stops
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

class EmptyExecutorNode : public ExecutorNode
{
public:
	void Execute(ExecuteContext& context) override {  }
};

class IfExecutorNode : public ExecutorNode
{
public:
	IfExecutorNode(BoolValueNode* conditionNode, ExecutorNode* elseBranch) :
		m_Condition(conditionNode),
		m_Else(elseBranch) {}

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

	void Execute(ExecuteContext& context) override;

private:
	Ptr<FloatValueNode> m_FloatNode;
	Ptr<Float2ValueNode> m_Float2Node;
	Ptr<Float3ValueNode> m_Float3Node;
	Ptr<Float4ValueNode> m_Float4Node;
};

class ClearRenderTargetExecutorNode : public ExecutorNode
{
public:
	ClearRenderTargetExecutorNode(TextureValueNode* textureNode, Float4ValueNode* clearColorNode) :
		m_TextureNode(textureNode),
		m_ClearColorNode(clearColorNode) {}

	void Execute(ExecuteContext& context) override;

private:
	Ptr<Float4ValueNode> m_ClearColorNode;
	Ptr<TextureValueNode> m_TextureNode;
};

template<typename T>
class AsignVariableExecutorNode : public ExecutorNode
{
public:
	AsignVariableExecutorNode(StringValueNode* nameNode, ValueNode<T>* value):
		m_NameNode(nameNode),
		m_InitialValueNode(value) {}

	void Execute(ExecuteContext& context) override
	{
		if (!m_NameNode)
		{
			ExecutionPrivate::Failure("AsignVariableExecutorNode", "Variable name not defined");
			context.Failure = true;
			return;
		}

		const std::string name = m_NameNode->GetValue(context);
		const unsigned varKey = Hash::Crc32(name);

		auto& variableMap = context.Variables.GetMapFromType<T>();
		variableMap[varKey] = m_InitialValueNode->GetValue(context);
	}

private:
	Ptr<StringValueNode> m_NameNode;
	Ptr<ValueNode<T>> m_InitialValueNode;
};

class CreateTextureExecutorNode : public ExecutorNode
{
public:
	CreateTextureExecutorNode(StringValueNode* nameNode, int width, int height, bool isFramebuffer, bool isDepthStencil):
		m_NameNode(nameNode),
		m_Width(width),
		m_Height(height),
		m_IsFramebuffer(isFramebuffer),
		m_IsDepthStencil(isDepthStencil) {}

	void Execute(ExecuteContext& context) override;

private:
	Ptr<StringValueNode> m_NameNode;

	int m_Width;
	int m_Height;
	bool m_IsFramebuffer;
	bool m_IsDepthStencil;
};

class PresentTextureExecutorNode : public ExecutorNode
{
public:
	PresentTextureExecutorNode(TextureValueNode* texture):
		m_Texture(texture)
	{ }

	void Execute(ExecuteContext& context) override;

private:
	Ptr<TextureValueNode> m_Texture;
};

class LoadTextureExecutorNode : public ExecutorNode
{
public:
	LoadTextureExecutorNode(StringValueNode* nameNode, const std::string& texturePath) :
		m_NameNode(nameNode),
		m_TexturePath(texturePath) {}

	void Execute(ExecuteContext& context) override;

private:
	Ptr<StringValueNode> m_NameNode;
	std::string m_TexturePath;
};

class LoadShaderExecutorNode : public ExecutorNode
{
public:
	LoadShaderExecutorNode(StringValueNode* nameNode, const std::string& shaderPath) :
		m_NameNode(nameNode),
		m_ShaderPath(shaderPath) {}

	void Execute(ExecuteContext& context) override;
private:
	Ptr<StringValueNode> m_NameNode;
	std::string m_ShaderPath;
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

	void Execute(ExecuteContext& context) override;
private:
	unsigned m_VAO = 0;

	Ptr<TextureValueNode> m_FramebufferNode;
	Ptr<ShaderValueNode> m_ShaderNode;
	Ptr<MeshValueNode> m_MeshNode;
	Ptr<BindTableValueNode> m_BindTable;
	Ptr<RenderStateValueNode> m_RenderState;
};

class LoadMeshExecutorNode : public ExecutorNode
{
public:
	LoadMeshExecutorNode(StringValueNode* nameNode, const std::string& meshPath) :
		m_NameNode(nameNode),
		m_MeshPath(meshPath) {}

	void Execute(ExecuteContext& context) override;
private:
	Ptr<StringValueNode> m_NameNode;
	std::string m_MeshPath;
};