#pragma once

#include <ostream>

#include "../Common.h"
#include "../NodeGraph/VariablePool.h"

class CodeGenerator 
{
public:
	enum class Operator
	{
		Add,
		Substract,
		Multiply,
		Divide,
		And,
		Or,
		Not,
		Asign,
		Equal,
		NotEqual,
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual,
	};

public:
	virtual void BeginExpression() = 0;
	virtual void EndExpression() = 0;

	virtual void WriteOperator(Operator op) = 0;
	virtual void WriteKeyword(const std::string& keyword) = 0;

	virtual void EndInstruction() = 0;

	virtual void BeginBlock() = 0;
	virtual void EndBlock() = 0;

	virtual void WriteNull() = 0;

	virtual void WriteConstant(bool value) = 0;
	virtual void WriteConstant(char value) = 0;
	virtual void WriteConstant(int value) = 0;
	virtual void WriteConstant(float value) = 0;
	virtual void WriteConstant(Float2 value) = 0;
	virtual void WriteConstant(Float3 value) = 0;
	virtual void WriteConstant(Float4 value) = 0;
	virtual void WriteConstant(Float4x4 value) = 0;
	virtual void WriteConstant(const char* value) = 0;
	void WriteConstant(const std::string& value) { WriteConstant(value.c_str()); }

	virtual void FunctionCall(const std::string& functionName) = 0;
	virtual void FunctionArgumentsBegin() = 0;
	virtual void FunctionArgumentsEnd() = 0;
	virtual void ArgumentsSeparator() = 0;

	virtual void WriteVariable(const Variable& variable) = 0;
	virtual void WriteVariable(const std::string& variableName) = 0;
	virtual void ClassMemeberAccess(const std::string& memberName) = 0;

	virtual void FunctionDeclaration(const std::string& functionName, const std::string& returnType, bool isStatic = false, bool isAsync = false) = 0;
	virtual void ClassDeclaration(const std::string& className) = 0;
	virtual void ClassMemberDeclaration(const std::string& memberName) = 0;

	virtual void BeginInlineObject() = 0;
	virtual void EndInlineObject() = 0;

	virtual void ArrayBegin() = 0;
	virtual void ArrayEnd() = 0;
};

class WebGPUCodeGenerator : public CodeGenerator
{
public:
	WebGPUCodeGenerator(std::ostream& out):
		m_Out(out) {}

	void BeginExpression() override
	{
		m_Out << "(";
	}

	void EndInstruction() override
	{
		m_Out << ";\n";
		AddIdentation();
	}
	
	void EndExpression() override
	{
		m_Out << ")";
	}

	void WriteKeyword(const std::string& keyword) override
	{
		m_Out << keyword << " ";
	}

	void WriteOperator(Operator op) override
	{
		m_Out << ToString(op);
	}

	void BeginBlock() override
	{
		m_Out << "\n";
		AddIdentation();

		m_Out << "{\n";
		m_IdentationLevel++;
		AddIdentation();
	}

	void EndBlock() override
	{
		m_Out << "\n";
		m_IdentationLevel--;
		AddIdentation();
		m_Out << "}\n";
		AddIdentation();
	}

	void WriteNull() override
	{
		m_Out << "null";
	}

	void WriteConstant(bool value) override 
	{
		m_Out << value ? "true" : "false";
	}

	void WriteConstant(char value) override
	{
		m_Out << "'" << value << "'";
	}

	void WriteConstant(int value) override 
	{
		m_Out << value;
	}

	void WriteConstant(float value) override 
	{
		m_Out << value;
	}

	void WriteConstant(Float2 value) override 
	{
		m_Out << "new Float2(" << value.x << "," << value.y << ")";
	}

	void WriteConstant(Float3 value) override 
	{
		m_Out << "new Float3(" << value.x << "," << value.y << "," << value.z << ")";
	}

	void WriteConstant(Float4 value) override 
	{
		m_Out << "new Float4(" << value.x << "," << value.y << "," << value.z << "," << value.w << ")";
	}

	void WriteConstant(Float4x4 value) override 
	{
		m_Out << "new Float4x4(";

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_Out << value[0][0];

				const bool lastElement = i == 3 && j == 3;
				if (!lastElement)
				{
					m_Out << ", ";
				}
			}
		}
		m_Out << ")";
	}

	void WriteConstant(const char* value) override
	{
		m_Out << "\"" << value << "\"";
	}

	void FunctionCall(const std::string& functionName) override
	{
		m_Out << functionName;
	}

	void FunctionArgumentsBegin() override
	{
		m_Out << "(";
	}

	void FunctionArgumentsEnd() override
	{
		m_Out << ")";
	}

	void ArgumentsSeparator() override
	{
		m_Out << ",";
	}

	void WriteVariable(const Variable& variable) override
	{
		m_Out << "this." << variable.Name;
	}

	void WriteVariable(const std::string& variableName) override
	{
		m_Out << variableName;
	}

	void ClassMemeberAccess(const std::string& memberName) override
	{
		m_Out << "." << memberName;
	}

	virtual void FunctionDeclaration(const std::string& functionName, const std::string& returnType, bool isStatic = false, bool isAsync = false) override
	{
		if (isStatic)
		{
			m_Out << "static ";
		}
		if (isAsync)
		{
			m_Out << "async ";
		}
		m_Out << functionName;
	}

	virtual void ClassDeclaration(const std::string& className) override
	{
		m_Out << "class " << className;
	}

	void ClassMemberDeclaration(const std::string& memberName) override
	{
		m_Out << memberName << ": ";
	}

	virtual void BeginInlineObject() override 
	{
		m_Out << "{ ";
	}

	virtual void EndInlineObject() override
	{
		m_Out << "} ";
	}

	virtual void ArrayBegin() override
	{
		m_Out << "[ ";
	}

	virtual void ArrayEnd() override
	{
		m_Out << " ]";
	}

private:
	void AddIdentation()
	{
		for (uint32_t i = 0; i < m_IdentationLevel; i++)
		{
			m_Out << "  ";
		}
	}

	std::string ToString(Operator op)
	{
		switch (op)
		{
		case CodeGenerator::Operator::Add: return " + ";
		case CodeGenerator::Operator::Substract: return " - ";
		case CodeGenerator::Operator::Multiply: return " * ";
		case CodeGenerator::Operator::Divide: return " / ";
		case CodeGenerator::Operator::And: return " && ";
		case CodeGenerator::Operator::Or: return " || ";
		case CodeGenerator::Operator::Not: return " !";
		case CodeGenerator::Operator::Asign: return " = ";
		case CodeGenerator::Operator::Equal: return " == ";
		case CodeGenerator::Operator::NotEqual: return " != ";
		case CodeGenerator::Operator::Less: return " < ";
		case CodeGenerator::Operator::LessOrEqual: return " <= ";
		case CodeGenerator::Operator::Greater: return " > ";
		case CodeGenerator::Operator::GreaterOrEqual: return " >= ";
		default:
			NOT_IMPLEMENTED;
			break;
		}
		return "";
	}

private:
	std::ostream& m_Out;

	uint32_t m_IdentationLevel = 0;
};