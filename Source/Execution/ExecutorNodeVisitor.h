#pragma once

#include "ExecuteContext.h"

class EmptyExecutorNode;
class IfExecutorNode;
class PrintExecutorNode;
class ClearRenderTargetExecutorNode;
class AsignVariableExecutorNode;
class PresentTextureExecutorNode;
class DrawMeshExecutorNode;
class ForEachSceneObjectExecutorNode;
class CodeGenerator;

class ExecutorNodeVisitor
{
public:
	ExecutorNodeVisitor(ExecuteContext& context) :
		m_Context(context) {
	}

	virtual void Visit(EmptyExecutorNode& node) = 0;
	virtual void Visit(IfExecutorNode& node) = 0;
	virtual void Visit(PrintExecutorNode& node) = 0;
	virtual void Visit(ClearRenderTargetExecutorNode& node) = 0;
	virtual void Visit(AsignVariableExecutorNode& node) = 0;
	virtual void Visit(PresentTextureExecutorNode& node) = 0;
	virtual void Visit(DrawMeshExecutorNode& node) = 0;
	virtual void Visit(ForEachSceneObjectExecutorNode& node) = 0;

	void ExecuteNodes(ExecutorNode* startNode);

protected:
	ExecuteContext& m_Context;
};

class InEditorExecutorNodeVisitor : public ExecutorNodeVisitor
{
public:
	InEditorExecutorNodeVisitor(ExecuteContext& context) :
		ExecutorNodeVisitor(context) {
	}

	void Visit(EmptyExecutorNode& node) override;
	void Visit(IfExecutorNode& node) override;
	void Visit(PrintExecutorNode& node) override;
	void Visit(ClearRenderTargetExecutorNode& node) override;
	void Visit(AsignVariableExecutorNode& node) override;
	void Visit(PresentTextureExecutorNode& node) override;
	void Visit(DrawMeshExecutorNode& node) override;
	void Visit(ForEachSceneObjectExecutorNode& node) override;
};

class WebGLExecutorNodeVisitor : public ExecutorNodeVisitor
{
public:
	WebGLExecutorNodeVisitor(ExecuteContext& context, CodeGenerator& generator) :
		ExecutorNodeVisitor(context),
		m_Generator(generator){ }

	void Visit(EmptyExecutorNode& node) override;
	void Visit(IfExecutorNode& node) override;
	void Visit(PrintExecutorNode& node) override;
	void Visit(ClearRenderTargetExecutorNode& node) override;
	void Visit(AsignVariableExecutorNode& node) override;
	void Visit(PresentTextureExecutorNode& node) override;
	void Visit(DrawMeshExecutorNode& node) override;
	void Visit(ForEachSceneObjectExecutorNode& node) override;

private:
	CodeGenerator& m_Generator;
};