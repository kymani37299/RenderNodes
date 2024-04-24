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

		ImGui::SetNextItemWidth(ImGui::ConstantSize(150.0f));
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

	std::string tolower(std::string s) {
		for (char& c : s)
			c = std::tolower(c);
		return s;
	}

	bool MenuItem::MatchFilter(const std::string& filter) const
	{
		return filter == "" || tolower(m_Label).find(tolower(filter)) != std::string::npos;
	}

	void MenuItem::Render(const std::string& filter) const
	{
		if (MatchFilter(filter) && ImGui::MenuItem(m_Label.c_str()))
		{
			OnClick();
		}
	}

	Menu::~Menu()
	{
		if (m_IsRoot)
		{
			DeleteItems();
		}
	}

	void Menu::AddItem(MenuItem* item)
	{
		m_Items.push_back(item);
	}

	void Menu::AddMenu(const Menu& menu)
	{
		m_Menus.push_back(menu);
	}

	void Menu::Render(const std::string& filter) const
	{
		if (m_Items.size() == 0 && m_Menus.size() == 0) return;
		if (!MatchFilter(filter)) return;

		if (!m_IsRoot)
		{
			if (ImGui::BeginMenu(m_Label.c_str()))
			{
				RenderContent(filter);
				ImGui::EndMenu();
			}
		}
		else
		{
			RenderContent(filter);
		}
	}

	bool Menu::MatchFilter(const std::string& filter) const
	{
		if (filter == "")
			return true;

		bool match = false;
		for (const Menu& menu : m_Menus)
		{
			match = match || menu.MatchFilter(filter);
			if (match) break;
		}

		if (!match)
		{
			for (MenuItem* item : m_Items)
			{
				match = match || item->MatchFilter(filter);
				if (match) break;
			}
		}
		return match;
	}

	void Menu::RenderContent(const std::string& filter) const
	{
		for (const Menu& menu : m_Menus)
		{
			menu.Render(filter);
		}

		for (MenuItem* item : m_Items)
		{
			item->Render(filter);
		}
	}

	void Menu::DeleteItems()
	{
		for (Menu& menu : m_Menus)
		{
			menu.DeleteItems();
		}
		for (MenuItem* item : m_Items)
		{
			delete item;
		}
	}

}