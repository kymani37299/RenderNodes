#include "AppConsole.h"

#include "../Common.h"

void AppConsole::Draw()
{
	ImGui::Begin("Console");
	ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

	if (ImGui::Button("Clear")) Clear();

	ImGui::Separator();

	ImGui::TextUnformatted(m_TextBuffer.c_str());

	ImGui::End();
}

void AppConsole::Clear()
{
	m_TextBuffer = "";
}

void AppConsole::Log(const std::string& msg)
{
	m_TextBuffer += msg + "\n";
}
