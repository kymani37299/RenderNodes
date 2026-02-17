#pragma once

#include <string>

#include "../../NodeGraph/VariablePool.h"

class EditorDialog
{
public:
	EditorDialog(const std::string id, bool openImmidiately = true):
		m_ID(id),
		m_ShouldOpen(openImmidiately) {}

	void Open()
	{
		m_ShouldOpen = true;
	}

	void Close();
	bool Draw();

protected:
	virtual void DrawContent() = 0;

private:
	bool m_IsOpen = false;

	bool m_ShouldOpen;
	std::string m_ID;
};

class NewVariableDialog : public EditorDialog
{
public:
	NewVariableDialog() :
		EditorDialog("New Variable")
	{
		m_Variable.Type = VariableType::Int;
		m_Variable.SetDefault();
	}

protected:
	void DrawContent() override;

private:
	Variable m_Variable{};
};

class GenerateCodeDialog : public EditorDialog
{
public:
	GenerateCodeDialog() :
		EditorDialog("Generate Code") { }

protected:
	void DrawContent() override;

private:
	std::string m_ProjectName;
};