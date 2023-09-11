#pragma once

#include <imgui.h>
#include <string>

#include "DataTypes.h"

namespace ImGui
{
	inline float ConstantSize(float size)
	{
		return size * ImGui::GetIO().FontGlobalScale;
	}

	template<unsigned int BUF_SIZE = 25>
	inline bool InputText(const char* label, std::string& str)
	{
		static thread_local char BUF[BUF_SIZE];

		strcpy_s(BUF, str.c_str());
		bool ret = ImGui::InputText(label, BUF, BUF_SIZE);

		if (ret)
		{
			str = std::string(BUF);
		}

		return ret;
	}

	inline bool DragFloat2(const char* label, Float2& value)
	{
		static thread_local float v[2];

		v[0] = value.r;
		v[1] = value.g;

		bool ret = ImGui::DragFloat2(label, v);

		if (ret)
		{
			value.r = v[0];
			value.g = v[1];
		}

		return ret;
	}

	inline bool DragFloat3(const char* label, Float3& value)
	{
		static thread_local float v[3];

		v[0] = value.r;
		v[1] = value.g;
		v[2] = value.b;

		bool ret = ImGui::DragFloat3(label, v);

		if (ret)
		{
			value.r = v[0];
			value.g = v[1];
			value.b = v[2];
		}

		return ret;
	}

	inline bool DragFloat4(const char* label, Float4& value)
	{
		static thread_local float v[4];

		v[0] = value.r;
		v[1] = value.g;
		v[2] = value.b;
		v[3] = value.a;

		bool ret = ImGui::DragFloat4(label, v);

		if (ret)
		{
			value.r = v[0];
			value.g = v[1];
			value.b = v[2];
			value.a = v[3];
		}

		return ret;
	}
}