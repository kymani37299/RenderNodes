#pragma once

#include <variant>
#include <unordered_map>

#include "../IDGen.h"
#include "../Common.h"

struct TextureData
{
	std::string Path = "";
	int Width = 0;
	int Height = 0;
	bool Framebuffer = false;
	bool DepthStencil = false;
};

struct ShaderData
{
	std::string Path = "";
};

struct SceneData
{
	std::string Path = "";
};

// DO NOT CHANGE ORDER OF VALUES
// IT WILL AFFECT HOW WE LOAD OLD SAVE FILES
// ALWAYS APPEND ON END
enum class VariableType
{
	Invalid,
	Bool,
	Int,
	Float,
	Float2,
	Float3,
	Float4,
	Float4x4,
	Shader,
	Texture,
	Scene,

	Count,
};

using VariableData = std::variant<
	bool,
	int,
	float,
	Float2,
	Float3,
	Float4,
	Float4x4,
	TextureData,
	ShaderData,
	SceneData
>;

struct Variable
{
	std::string Name = "";
	VariableType Type = VariableType::Invalid;
	VariableData Data;

	template<typename T>
	T& Get()
	{
		return std::get<T>(Data);
	}

	template<typename T>
	const T& Get() const
	{
		return std::get<T>(Data);
	}

	void SetDefault()
	{
		switch (Type)
		{
		case VariableType::Invalid:
			ASSERT_M(0, "Trying to set default value on VariableType::Invalid");
			break;
		case VariableType::Bool:
			Data = false;
			break;
		case VariableType::Int:
			Data = 0;
			break;
		case VariableType::Float:
			Data = 0.0f;
			break;
		case VariableType::Float2:
			Data = Float2{};
			break;
		case VariableType::Float3:
			Data = Float3{};
			break;
		case VariableType::Float4:
			Data = Float4{};
			break;
		case VariableType::Float4x4:
			Data = Float4x4{};
			break;
		case VariableType::Texture:
			Data = TextureData{};
			break;
		case VariableType::Shader:
			Data = ShaderData{};
			break;
		case VariableType::Scene:
			Data = SceneData{};
			break;
		default:
			NOT_IMPLEMENTED;
		}
	}
};

class VariablePool
{
public:
	static constexpr VariableID ID_DT = 1;
	static constexpr VariableID ID_CubeMesh = 2;

public:
	Variable& CreateVariable(const Variable& variable)
	{
		static Variable nullVar{};

		if (variable.Type == VariableType::Invalid)
			return nullVar;

		if (variable.Name.empty() || ContainsVariableWithName(variable.Name))
			return nullVar;

		VariableID id = IDGen::Generate();
		m_Pool[id] = variable;
		return m_Pool[id];
	}

	void LoadVariable(VariableID id, const Variable& variable)
	{
		m_Pool[id] = variable;
	}

	const Variable& GetRef(VariableID id) const
	{
		const auto& it = m_Pool.find(id);
		if (it != m_Pool.end())
		{
			return it->second;
		}
		else
		{
			static Variable nullVar{};
			return nullVar;
		}
	}

	Variable& GetRef(VariableID id)
	{
		const auto& it = m_Pool.find(id);
		if (it != m_Pool.end())
		{
			return it->second;
		}
		else
		{
			static Variable nullVar{};
			return nullVar;
		}
	}

	Variable& GetRefOrCreate(VariableID id, VariableType type, const std::string& name)
	{
		auto& variable = GetRef(id);
		if (variable.Type != VariableType::Invalid)
		{
			ASSERT(variable.Type == type);
			ASSERT(variable.Name == name);
			return variable;
		}
		else
		{
			ASSERT(!ContainsVariableWithName(name));

			Variable variable{};
			variable.Name = name;
			variable.Type = type;
			variable.SetDefault();

			m_Pool[id] = variable;
			return m_Pool[id];
		}
	}

	bool ContainsVariableWithName(const std::string& name)
	{
		bool found = false;
		for (const auto& it : m_Pool)
		{
			if (it.second.Name == name)
			{
				found = true;
				break;
			}
		}
		return found;
	}

	template<typename Fn>
	void ForEachVariable(Fn& func)
	{
		for (auto& it : m_Pool)
		{
			func(it.first, it.second);
		}
	}

	template<typename Fn>
	void ForEachVariable(Fn& func) const
	{
		for (const auto& it : m_Pool)
		{
			func(it.first, it.second);
		}
	}
private:
	std::unordered_map<VariableID, Variable> m_Pool{};
};

inline const char* ToString(VariableType variableType)
{
	switch (variableType)
	{
		ENUM_STR_CASE(VariableType, Invalid);
		ENUM_STR_CASE(VariableType, Bool);
		ENUM_STR_CASE(VariableType, Int);
		ENUM_STR_CASE(VariableType, Float);
		ENUM_STR_CASE(VariableType, Float2);
		ENUM_STR_CASE(VariableType, Float3);
		ENUM_STR_CASE(VariableType, Float4);
		ENUM_STR_CASE(VariableType, Float4x4);
		ENUM_STR_CASE(VariableType, Shader);
		ENUM_STR_CASE(VariableType, Texture);
		ENUM_STR_CASE(VariableType, Scene);
	default:
		NOT_IMPLEMENTED;
	}
	return "Invalid";
}