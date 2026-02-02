#include "EditorDialog.h"

#include "../../App/App.h"
#include "../../Common.h"
#include "EditorWidgets.h"

bool EditorDialog::Draw()
{
	ImNode::Suspend();

	if (m_ShouldOpen)
	{
		ImGui::OpenPopup(m_ID.c_str());
		m_ShouldOpen = false;
		m_IsOpen = true;
	}

	if (ImGui::BeginPopupModal(m_ID.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		DrawContent();
		ImGui::EndPopup();
	}

	ImNode::Resume();

	return m_IsOpen;
}

void EditorDialog::Close()
{
	m_IsOpen = false;
	ImGui::CloseCurrentPopup();
}

void NewVariableDialog::DrawContent()
{
	bool changed = EditorWidgets::ComboBox("Type", *reinterpret_cast<uint32_t*>(&m_Variable.Type), (uint32_t)VariableType::Int, (uint32_t)VariableType::Count - 1, [](uint32_t index) {
		return ToString((VariableType)index);
	});

	if (changed)
	{
		m_Variable.SetDefault();
	}

	ImGui::InputText("Name", m_Variable.Name);

	EditorWidgets::InputVariable("Value: ", m_Variable);

	const bool validVariable = !m_Variable.Name.empty() && !App::Get()->GetVariablePool().ContainsVariableWithName(m_Variable.Name);

	if (!validVariable) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

	if (ImGui::Button("Create") && validVariable)
	{
		App::Get()->GetVariablePool().CreateVariable(m_Variable);
		Close();
	}

	if (!validVariable) ImGui::PopStyleVar();

	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		Close();
	}
}