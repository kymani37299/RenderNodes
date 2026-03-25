#pragma once

#include <vector>

#include "ValueNode.h"

#include "ExecuteContext.h"
#include "ExecutorScene.h"
#include "RenderStructs.h"
#include "../CodeGenerator/CodeGenerator.h"

namespace ValueNodeHelpers
{
	static std::vector<std::string> VectorClassName = { "float", "Float2", "Float3", "Float4" };
	static std::vector<std::string> VectorMembers = { "x", "y", "z", "w" };
}

template<typename T>
class NullPtrValueNode : public ValueNode<T*>
{
	virtual T* GetValue(ExecuteContext& context) const override { return nullptr; }
	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override { generator.WriteNull(); }
};

template<typename T>
class ConstantValueNode : public ValueNode<T>
{
public:
	ConstantValueNode(const T& value) :
		m_Value(value) {}

	virtual T GetValue(ExecuteContext& context) const override { return m_Value; }
	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override { generator.WriteConstant(m_Value); }

private:
	T m_Value;
};

class BindTableConstantValueNode : public BindTableValueNode
{
public:
	BindTableConstantValueNode(BindTable* table) :
		m_Table(table) { }

	virtual BindTable* GetValue(ExecuteContext& context) const override { return m_Table.get(); }

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		BindTable* table = m_Table.get();
		if (!table)
		{
			generator.WriteNull();
		}
		else
		{
			generator.BeginBlock();

			WriteBindingArray(generator, context, "Textures", table->Textures);
			WriteBindingArray(generator, context, "Floats", table->Floats);
			WriteBindingArray(generator, context, "Float2s", table->Float2s);
			WriteBindingArray(generator, context, "Float3s", table->Float3s);
			WriteBindingArray(generator, context, "Float4s", table->Float4s);
			WriteBindingArray(generator, context, "Float4x4s", table->Float4x4s);
			
			generator.EndBlock();
		}
	}

private:
	template<typename T>
	static void WriteBindingArray(CodeGenerator& generator, ExecuteContext& context, const std::string& arrayName, const std::vector<BindTable::Binding<T>>& bindingArray)
	{
		if (bindingArray.empty())
			return;

		generator.ClassMemberDeclaration(arrayName);
		generator.ArrayBegin();
		for (const auto& binding : bindingArray)
		{
			generator.BeginInlineObject();
			generator.ClassMemberDeclaration("Name");
			generator.WriteConstant(binding.Name);
			generator.ArgumentsSeparator();
			generator.ClassMemberDeclaration("Value");
			if (binding.Value)
			{
				binding.Value->GenerateExpression(generator, context);
			}
			else
			{
				generator.WriteNull();
			}
			generator.EndInlineObject();
			generator.ArgumentsSeparator();
		}
		generator.ArrayEnd();
		generator.ArgumentsSeparator();
	}

private:
	Ptr<BindTable> m_Table;
};

class RenderStateConstantValueNode : public RenderStateValueNode
{
public:
	RenderStateConstantValueNode(const RenderState& renderState) :
		m_RenderState(renderState) {
	}

	virtual RenderState GetValue(ExecuteContext& context) const override { return m_RenderState; }

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override 
	{
		generator.BeginInlineObject();
		generator.ClassMemberDeclaration("DepthWrite");
		generator.WriteConstant(m_RenderState.DepthWrite);
		generator.ArgumentsSeparator();
		generator.ClassMemberDeclaration("DepthTest");
		generator.WriteConstant(DepthTestModeToString(m_RenderState.DepthTest));
		generator.EndInlineObject();
	}

private:
	static std::string DepthTestModeToString(GLenum depthTestMode)
	{
		switch (depthTestMode)
		{
		case GL_ALWAYS: return "Always";
		case GL_NEVER: return "Never";
		case GL_LESS: return "Less";
		case GL_LEQUAL: return "LEqual";
		case GL_GREATER: return "Greater";
		case GL_GEQUAL: return "GEqual";
		case GL_NOTEQUAL: return "NotEqual";
		default:
			NOT_IMPLEMENTED;
		}
		return "";
	}

private:
	RenderState m_RenderState;
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
	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override 
	{ 
		generator.FunctionCall("RenderNodeAPI.GetStaticResources");
		generator.FunctionArgumentsBegin();
		generator.FunctionArgumentsEnd();
		generator.ClassMemeberAccess(context.RenderResources.GetStaticResourceVariableName<staticResource>());
	}
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

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override 
	{
		generator.BeginExpression();
		m_A->GenerateExpression(generator, context);
		switch (m_Op)
		{
		case '+':
			generator.WriteOperator(CodeGenerator::Operator::Add);
			break;
		case '-':
			generator.WriteOperator(CodeGenerator::Operator::Substract);
			break;
		case '/':
			generator.WriteOperator(CodeGenerator::Operator::Divide);
			break;
		case '*':
			generator.WriteOperator(CodeGenerator::Operator::Multiply);
			break;
		default:
			NOT_IMPLEMENTED;
		}
		m_B->GenerateExpression(generator, context);
		generator.EndExpression();
	}

private:
	Ptr<ValueNode<T>> m_A;
	Ptr<ValueNode<T>> m_B;
	char m_Op;
};

template<typename T>
class VectorBinaryArithmeticOperatorValueNode : public ValueNode<T>
{
	static_assert(
		std::is_same_v<T, Float2> || std::is_same_v<T, Float3> || std::is_same_v<T, Float4>,
		"BinaryArithmeticOperatorValueNode only supports Float2, Float3, and Float4 types"
		);

public:
	VectorBinaryArithmeticOperatorValueNode(ValueNode<T>* a, ValueNode<T>* b, char op) :
		m_A(Ptr<ValueNode<T>>(a)),
		m_B(Ptr<ValueNode<T>>(b)),
		m_Op(op) {}

	virtual T GetValue(ExecuteContext& context) const override
	{
		if (!m_A || !m_B)
		{
			ExecutionPrivate::Failure("VectorBinaryArithmeticOperatorValueNode", "Binary operator have missing values!");
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

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.BeginExpression();
		generator.FunctionCall("RenderNodeAPI." + GetVectorOperation());
		generator.FunctionArgumentsBegin();
		m_A->GenerateExpression(generator, context);
		generator.ArgumentsSeparator();
		m_B->GenerateExpression(generator, context);
		generator.FunctionArgumentsEnd();
		generator.EndExpression();
	}

private:
	std::string GetVectorOperation() const
	{
		switch (m_Op)
		{
		case '+': return "VectorAdd";
		case '-': return "VectorSubtract";
		case '/': return "VectorDivide";
		case '*': return "VectorMultiply";
		default:
			NOT_IMPLEMENTED;
		}
		return "VectorAdd";
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

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.BeginExpression();
		m_A->GenerateExpression(generator, context);
		if (m_Op == "==") generator.WriteOperator(CodeGenerator::Operator::Equal);
		else if (m_Op == "!=") return generator.WriteOperator(CodeGenerator::Operator::NotEqual);
		else if (m_Op == ">") return generator.WriteOperator(CodeGenerator::Operator::Greater);
		else if (m_Op == "<") return generator.WriteOperator(CodeGenerator::Operator::Less);
		else if (m_Op == ">=") return generator.WriteOperator(CodeGenerator::Operator::GreaterOrEqual);
		else if (m_Op == "<=") return generator.WriteOperator(CodeGenerator::Operator::LessOrEqual);
		else NOT_IMPLEMENTED;
		m_B->GenerateExpression(generator, context);
		generator.EndExpression();
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

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.BeginExpression();
		if (m_Op == "XOR")
		{
			// (a && !b) || (!a && b)
			generator.BeginExpression();
			m_A->GenerateExpression(generator, context);
			generator.WriteOperator(CodeGenerator::Operator::And);
			generator.WriteOperator(CodeGenerator::Operator::Not);
			m_B->GenerateExpression(generator, context);
			generator.EndExpression();
			generator.WriteOperator(CodeGenerator::Operator::Or);
			generator.BeginExpression();
			generator.WriteOperator(CodeGenerator::Operator::Not);
			m_A->GenerateExpression(generator, context);
			generator.WriteOperator(CodeGenerator::Operator::And);
			m_B->GenerateExpression(generator, context);
			generator.EndExpression();
		}
		else
		{
			m_A->GenerateExpression(generator, context);
			if (m_Op == "AND") generator.WriteOperator(CodeGenerator::Operator::And);
			else if (m_Op == "OR") generator.WriteOperator(CodeGenerator::Operator::Or);
			else NOT_IMPLEMENTED;
			m_B->GenerateExpression(generator, context);
		}
		generator.EndExpression();
	}

private:
	std::string GetOperatorString() const
	{
		if (m_Op == "AND") return "&&";
		if (m_Op == "OR") return "||";
		if (m_Op == "XOR") return "^";

		NOT_IMPLEMENTED;
		return "";
	}

private:
	Ptr<BoolValueNode> m_A;
	Ptr<BoolValueNode> m_B;
	std::string m_Op;
};

template<typename T>
class NormalizeVectorValueNode : public ValueNode<T>
{
public:
	NormalizeVectorValueNode(ValueNode<T>* input):
		m_Input(input)
	{ }

	virtual T GetValue(ExecuteContext& context) const
	{
		if (!m_Input)
		{
			ExecutionPrivate::Failure("NormalizeVectorValueNode", "Normalize operator has missing values!");
			context.Failure = true;
			return T{};
		}
		return glm::normalize(m_Input->GetValue(context));
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.FunctionCall("RenderNodeAPI.VectorNormalize");
		generator.FunctionArgumentsBegin();
		m_Input->GenerateExpression(generator, context);
		generator.FunctionArgumentsEnd();
	}

private:
	Ptr<ValueNode<T>> m_Input;
};

class CrossProductValueNode : public Float3ValueNode
{
public:
	CrossProductValueNode(Float3ValueNode* a, Float3ValueNode* b):
		m_A(a),
		m_B(b)
	{ }

	virtual Float3 GetValue(ExecuteContext& context) const
	{
		if (!m_A || !m_B)
		{
			ExecutionPrivate::Failure("CrossProductValueNode", "Missing operands!");
			context.Failure = true;
			return Float3{};
		}

		const Float3 a = m_A->GetValue(context);
		const Float3 b = m_B->GetValue(context);

		return glm::cross(a, b);
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.FunctionCall("RenderNodeAPI.VectorCross");
		generator.FunctionArgumentsBegin();
		m_A->GenerateExpression(generator, context);
		generator.ArgumentsSeparator();
		m_B->GenerateExpression(generator, context);
		generator.FunctionArgumentsEnd();
	}

private:
	Ptr<Float3ValueNode> m_A;
	Ptr<Float3ValueNode> m_B;
};

template<typename T>
class VariableValueNode : public ValueNode<T>
{
public:
	VariableValueNode(VariableID variableID) :
		m_VariableID(variableID) {}

	virtual T GetValue(ExecuteContext& context) const override
	{
		const Variable& variable = context.VariablePool.GetRef(m_VariableID);
		if (variable.Type == VariableType::Invalid)
		{
			ExecutionPrivate::Failure("VariableValueNode", "Variable not declared");
			context.Failure = true;
			return T{};
		}
		return variable.Get<T>();
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		Variable variable = context.VariablePool.GetRef(m_VariableID);
		if (variable.Type == VariableType::Invalid)
		{
			ExecutionPrivate::Failure("VariableValueNode", "Variable not declared");
			context.Failure = true;
			return;
		}
		generator.WriteVariable(variable);
	}

private:
	VariableID m_VariableID = 0;
};

template<typename T>
class IteratorValueNode : public ValueNode<T>
{
public:
	IteratorValueNode(PinID iteratorPin):
		m_IteratorPin(iteratorPin) {}

	virtual T GetValue(ExecuteContext& context) const override
	{
		return context.Iterators.Get<T>(m_IteratorPin);
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.WriteVariable("it_" + std::to_string(m_IteratorPin));
	}

private:
	PinID m_IteratorPin = 0;
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

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.WriteKeyword("new");
		generator.WriteVariable(ValueNodeHelpers::VectorClassName[vecSize - 1]);
		generator.FunctionArgumentsBegin();

		for (uint32_t i = 0; i < vecSize; i++)
		{
			m_Values[i]->GenerateExpression(generator, context);
			if (i != vecSize - 1)
			{
				generator.ArgumentsSeparator();
			}
		}
		generator.FunctionArgumentsEnd();
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

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		if (!m_Value)
		{
			ExecutionPrivate::Failure("SplitVectorValueNode", "Missing input!");
			context.Failure = true;
			return;
		}

		generator.BeginExpression();
		m_Value->GenerateExpression(generator, context);
		generator.ClassMemeberAccess(ValueNodeHelpers::VectorMembers[m_VecIndex]);
		generator.EndExpression();
	}

private:
	unsigned m_VecIndex;
	Ptr<ValueNode<U>> m_Value;
};

class Float4x4RotateValueNode : public Float4x4ValueNode
{
public:
	Float4x4RotateValueNode(Float4x4ValueNode* lastTransformNode, FloatValueNode* angleNode, Float3ValueNode* axisNode) :
		m_LastTransformNode(lastTransformNode),
		m_AngleNode(angleNode),
		m_AxisNode(axisNode) {}

	Float4x4 GetValue(ExecuteContext& context) const override
	{
		const Float4x4 lastTransform = m_LastTransformNode ? m_LastTransformNode->GetValue(context) : glm::identity<Float4x4>();
		const float angle = m_AngleNode->GetValue(context);
		const Float3 axis = m_AxisNode->GetValue(context);
		return glm::rotate(lastTransform, angle, axis);
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.FunctionCall("RenderNodeAPI.RotateFloat4x4");
		generator.FunctionArgumentsBegin();
		if (m_LastTransformNode)
		{
			m_LastTransformNode->GenerateExpression(generator, context);
		}
		else
		{
			generator.WriteNull();
		}
		generator.ArgumentsSeparator();
		m_AngleNode->GenerateExpression(generator, context);
		generator.ArgumentsSeparator();
		m_AxisNode->GenerateExpression(generator, context);
		generator.FunctionArgumentsEnd();
	}

private:
	Ptr<Float4x4ValueNode> m_LastTransformNode;
	Ptr<FloatValueNode> m_AngleNode;
	Ptr<Float3ValueNode> m_AxisNode;
};

class Float4x4TranslateValueNode : public Float4x4ValueNode
{
public:
	Float4x4TranslateValueNode(Float4x4ValueNode* lastTransformNode, Float3ValueNode* valueNode) :
		m_LastTransformNode(lastTransformNode),
		m_ValueNode(valueNode) {}

	Float4x4 GetValue(ExecuteContext& context) const override
	{
		const Float4x4 lastTransform = m_LastTransformNode ? m_LastTransformNode->GetValue(context) : glm::identity<Float4x4>();
		const Float3 value = m_ValueNode->GetValue(context);
		return glm::translate(lastTransform, value);
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.FunctionCall("RenderNodeAPI.TranslateFloat4x4");
		generator.FunctionArgumentsBegin();
		if (m_LastTransformNode)
		{
			m_LastTransformNode->GenerateExpression(generator, context);
		}
		else
		{
			generator.WriteNull();
		}
		generator.ArgumentsSeparator();
		m_ValueNode->GenerateExpression(generator, context);
		generator.FunctionArgumentsEnd();
	}

private:
	Ptr<Float4x4ValueNode> m_LastTransformNode;
	Ptr<Float3ValueNode> m_ValueNode;
};

class Float4x4ScaleValueNode : public Float4x4ValueNode
{
public:
	Float4x4ScaleValueNode(Float4x4ValueNode* lastTransformNode, Float3ValueNode* valueNode) :
		m_LastTransformNode(lastTransformNode),
		m_ValueNode(valueNode) {}

	Float4x4 GetValue(ExecuteContext& context) const override
	{
		const Float4x4 lastTransform = m_LastTransformNode ? m_LastTransformNode->GetValue(context) : glm::identity<Float4x4>();
		const Float3 value = m_ValueNode->GetValue(context);
		return glm::scale(lastTransform, value);
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.FunctionCall("RenderNodeAPI.ScaleFloat4x4");
		generator.FunctionArgumentsBegin();
		if (m_LastTransformNode)
		{
			m_LastTransformNode->GenerateExpression(generator, context);
		}
		else
		{
			generator.WriteNull();
		}
		generator.ArgumentsSeparator();
		m_ValueNode->GenerateExpression(generator, context);
		generator.FunctionArgumentsEnd();
	}

private:
	Ptr<Float4x4ValueNode> m_LastTransformNode;
	Ptr<Float3ValueNode> m_ValueNode;
};

class Float4x4LookAtValueNode : public Float4x4ValueNode
{
public:
	Float4x4LookAtValueNode(Float3ValueNode* eyeNode, Float3ValueNode* centerNode, Float3ValueNode* upNode) :
		m_EyeNode(eyeNode),
		m_CenterNode(centerNode),
		m_UpNode(upNode) {}

	Float4x4 GetValue(ExecuteContext& context) const override
	{
		const Float3 eye = m_EyeNode->GetValue(context);
		const Float3 center = m_CenterNode->GetValue(context);
		const Float3 up = m_UpNode->GetValue(context);
		return glm::lookAt(eye, center, up);
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.FunctionCall("RenderNodeAPI.LookAtFloat4x4");
		generator.FunctionArgumentsBegin();
		m_EyeNode->GenerateExpression(generator, context);
		generator.ArgumentsSeparator();
		m_CenterNode->GenerateExpression(generator, context);
		generator.ArgumentsSeparator();
		m_UpNode->GenerateExpression(generator, context);
		generator.FunctionArgumentsEnd();
	}

private:
	Ptr<Float3ValueNode> m_EyeNode;
	Ptr<Float3ValueNode> m_CenterNode;
	Ptr<Float3ValueNode> m_UpNode;
};

class Float4x4PerspectiveValueNode : public Float4x4ValueNode
{
public:
	Float4x4PerspectiveValueNode(FloatValueNode* fovNode, FloatValueNode* aspectNode, FloatValueNode* zNearNode, FloatValueNode* zFarNode) :
		m_FOVNode(fovNode),
		m_AspectNode(aspectNode),
		m_ZNearNode(zNearNode),
		m_ZFarNode(zFarNode)
	{}

	Float4x4 GetValue(ExecuteContext& context) const override
	{
		const float fov = m_FOVNode->GetValue(context);
		const float aspect = m_AspectNode->GetValue(context);
		const float znear = m_ZNearNode->GetValue(context);
		const float zfar = m_ZFarNode->GetValue(context);
		return glm::perspective(fov, aspect, znear, zfar);
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		generator.FunctionCall("RenderNodeAPI.PerspectiveFloat4x4");
		generator.FunctionArgumentsBegin();
		m_FOVNode->GenerateExpression(generator, context);
		generator.ArgumentsSeparator();
		m_AspectNode->GenerateExpression(generator, context);
		generator.ArgumentsSeparator();
		m_ZNearNode->GenerateExpression(generator, context);
		generator.ArgumentsSeparator();
		m_ZFarNode->GenerateExpression(generator, context);
		generator.FunctionArgumentsEnd();
	}

private:
	Ptr<FloatValueNode> m_FOVNode;
	Ptr<FloatValueNode> m_AspectNode;
	Ptr<FloatValueNode> m_ZNearNode;
	Ptr<FloatValueNode> m_ZFarNode;
};

class GetMeshValueNode : public MeshValueNode
{
public:
	GetMeshValueNode(SceneObjectValueNode* sceneObjectNode, const ValueNodeExtraInfo& extraInfo):
		MeshValueNode(extraInfo),
		m_SceneObjectNode(sceneObjectNode)
	{}

	Mesh* GetValue(ExecuteContext& context) const override
	{
		SceneObject* sceneObject = m_SceneObjectNode->GetValue(context);
		return &sceneObject->MeshData;
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		m_SceneObjectNode->GenerateExpression(generator, context);
		generator.ClassMemeberAccess("MeshData");
	}

private:
	Ptr<SceneObjectValueNode> m_SceneObjectNode;
};

template<typename T>
class RenderResourceVariableValueNodeT : public ValueNode<T>
{
public:
	RenderResourceVariableValueNodeT(VariableID variableID) :
		m_VariableID(variableID)
	{}

	T GetValue(ExecuteContext& context) const override
	{
		return context.RenderResources.GetResource<T>(m_VariableID);
	}

	virtual void GenerateExpression(CodeGenerator& generator, ExecuteContext& context) const override
	{
		Variable variable = context.VariablePool.GetRef(m_VariableID);
		if (variable.Type == VariableType::Invalid)
		{
			ExecutionPrivate::Failure("VariableValueNode", "Variable not declared");
			context.Failure = true;
			return;
		}
		generator.WriteVariable(variable);
	}

private:
	VariableID m_VariableID;
};