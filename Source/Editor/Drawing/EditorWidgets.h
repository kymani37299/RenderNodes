#pragma once

#include <string>
#include <vector>

namespace EditorWidgets
{
	void InputText(const char* label, std::string& text, bool disabled = false);

	class MenuItem
	{
	public:
		MenuItem(const std::string& label) :
			m_Label(label) {}

		virtual ~MenuItem() {}

		bool MatchFilter(const std::string& filter) const;
		void Render(const std::string& filter) const;

		virtual void OnClick() const = 0;
		const std::string& GetLabel() const { return m_Label; }

	private:
		std::string m_Label = "";
	};

	class Menu
	{
	public:
		Menu(const std::string& label, bool isRoot = false) :
			m_Label(label),
			m_IsRoot(isRoot) {}

		~Menu();

		void AddItem(MenuItem* item);
		void AddMenu(const Menu& menu);

		void Render(const std::string& filter) const;
	private:
		bool MatchFilter(const std::string& filter) const;
		void RenderContent(const std::string& filter) const;

		void DeleteItems();

	private:
		bool m_IsRoot;
		std::string m_Label;
		std::vector<MenuItem*> m_Items;
		std::vector<Menu> m_Menus;
	};
}