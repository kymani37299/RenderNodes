#include "EditorWidgets.h"

#include "../../Common.h"

namespace EditorWidgets
{
	void InputText(const char* label, std::string& text, bool disabled)
	{
		if (disabled)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		constexpr unsigned BUF_SIZE = 50;
		char buf[BUF_SIZE];
		strcpy_s(buf, text.c_str());

		ImGui::SetNextItemWidth(150.0f);
		if (ImGui::InputText(label, buf, BUF_SIZE))
		{
			text = std::string{ buf };
		}

		if (disabled)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

}