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
	AsignVariableExecutorNode(const std::string& variableName, ValueNode<T>* value):
		m_VariableKey(Hash::Crc32(variableName)),
		m_InitialValueNode(value) {}

	void Execute(ExecuteContext& context) override
	{
		auto& variableMap = context.Variables.GetMapFromType<T>();
		variableMap[m_VariableKey] = m_InitialValueNode->GetValue(context);
	}

private:
	uint32_t m_VariableKey;
	Ptr<ValueNode<T>> m_InitialValueNode;
};

class CreateTextureExecutorNode : public ExecutorNode
{
public:
	CreateTextureExecutorNode(const std::string& textureName, int width, int height, bool isFramebuffer):
		m_TextureKey(Hash::Crc32(textureName)),
		m_Width(width),
		m_Height(height),
		m_IsFramebuffer(isFramebuffer) {}

	void Execute(ExecuteContext& context) override;

private:
	uint32_t m_TextureKey;
	int m_Width;
	int m_Height;
	bool m_IsFramebuffer;
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
	LoadTextureExecutorNode(const std::string& texutreName, const std::string& texturePath) :
		m_TextureKey(Hash::Crc32(texutreName)),
		m_TexturePath(texturePath) {}

	void Execute(ExecuteContext& context) override;

private:
	uint32_t m_TextureKey = 0;
	std::string m_TexturePath;
};

class LoadShaderExecutorNode : public ExecutorNode
{
public:
	LoadShaderExecutorNode(const std::string& shaderName, const std::string& shaderPath) :
		m_ShaderKey(Hash::Crc32(shaderName)),
		m_ShaderPath(shaderPath) {}

	void Execute(ExecuteContext& context) override;
private:
	uint32_t m_ShaderKey = 0;
	std::string m_ShaderPath;
};

class DrawMeshExecutorNode : public ExecutorNode
{
public:
	DrawMeshExecutorNode(TextureValueNode* framebufferNode, ShaderValueNode* shaderNode, MeshValueNode* meshNode):
		m_FramebufferNode(framebufferNode),
		m_ShaderNode(shaderNode),
		m_MeshNode(meshNode) {}

	~DrawMeshExecutorNode();

	void Execute(ExecuteContext& context) override;
private:
	unsigned m_VAO = 0;

	Ptr<TextureValueNode> m_FramebufferNode;
	Ptr<ShaderValueNode> m_ShaderNode;
	Ptr<MeshValueNode> m_MeshNode;
};