#include "EditorWidgets.h"

#include "../../Common.h"
#include "../../NodeGraph/VariablePool.h"
#include "../../Util/FileDialog.h"

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

	bool ComboBox(const char* label, uint32_t& selectedItem, uint32_t minValue, uint32_t maxValue, const std::function<const char* (uint32_t)>& toString)
	{
		bool changed = false;
		const char* preview = toString(selectedItem);
		if (ImGui::BeginCombo(label, preview))
		{
			for (uint32_t i = minValue; i <= maxValue; i++)
			{
				const bool selected = (i == selectedItem);

				if (ImGui::Selectable(toString(i), selected))
				{
					changed = selectedItem != i;
					selectedItem = i;
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		return changed;
	}

	void DrawVariable(const char* label, const Variable& variable)
	{
		switch (variable.Type)
		{
		case VariableType::Bool:
			break;
		case VariableType::Int:
			break;
		case VariableType::Float:
			break;
		case VariableType::Float2:
			break;
		case VariableType::Float3:
			break;
		case VariableType::Float4:
			break;
		case VariableType::Float4x4:
			break;
		case VariableType::Texture:
			break;
		case VariableType::Scene:
			break;
		case VariableType::Shader:
			break;
		default:
			NOT_IMPLEMENTED;
			break;
		}
	}
	
	void InputVariable(const char* label, Variable& variable)
	{
		switch (variable.Type)
		{
		case VariableType::Bool:
			ImGui::Checkbox(label, &variable.Get<bool>());
			break;
		case VariableType::Int:
			ImGui::InputInt(label, &variable.Get<int>());
			break;
		case VariableType::Float:
			ImGui::InputFloat(label, &variable.Get<float>());
			break;
		case VariableType::Float2:
			ImGui::InputFloat2(label, variable.Get<Float2>());
			break;
		case VariableType::Float3:
			ImGui::InputFloat3(label, variable.Get<Float3>());
			break;
		case VariableType::Float4:
			ImGui::InputFloat4(label, variable.Get<Float4>());
			break;
		case VariableType::Float4x4:
		{
			auto& value = variable.Get<Float4x4>();
			ImGui::BeginVertical("Matrix");
			for (unsigned i = 0; i < 4; i++)
			{
				ImGui::PushID(i);
				ImGui::BeginHorizontal("Matrix row");
				for (unsigned j = 0; j < 4; j++)
				{
					ImGui::PushID(j);
					ImGui::SetNextItemWidth(ImGui::ConstantSize(50.0f));
					ImGui::DragFloat("", &value[i][j]);
					ImGui::PopID();
				}
				ImGui::EndHorizontal();
				ImGui::PopID();
			}
			ImGui::EndVertical();
		} break;
		case VariableType::Texture:
		{
			auto& value = variable.Get<TextureData>();
			ImGui::InputInt("Width", &value.Width);
			ImGui::InputInt("Height", &value.Height);
			ImGui::Checkbox("Framebuffer", &value.Framebuffer);
			ImGui::Checkbox("Depth stencil", &value.DepthStencil);

			if (ImGui::Button("Select file..."))
			{
				std::string texPath{};
				const bool fileOpened = FileDialog::OpenTextureFile(texPath);
				if (fileOpened)
				{
					value.Path = texPath;
				}
			}
			ImGui::Text(value.Path.c_str());
		} break;
		case VariableType::Scene:
		{
			auto& value = variable.Get<SceneData>();
			if (ImGui::Button("Select file..."))
			{
				std::string path{};
				const bool fileOpened = FileDialog::OpenSceneFile(path);
				if (fileOpened)
				{
					value.Path = path;
				}
			}
			ImGui::Text(value.Path.c_str());
		} break;
		case VariableType::Shader:
		{
			auto& value = variable.Get<ShaderData>();
			if (ImGui::Button("Select file..."))
			{
				std::string path{};
				const bool fileOpened = FileDialog::OpenShaderFile(path);
				if (fileOpened)
				{
					value.Path = path;
				}
			}
			ImGui::Text(value.Path.c_str());
		} break;
		default:
			NOT_IMPLEMENTED;
			break;
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