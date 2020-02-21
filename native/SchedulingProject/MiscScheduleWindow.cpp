#include "stdafx.h"
#include "MiscScheduleWindow.h"

#include "imgui.h"


MiscScheduleWindow::MiscScheduleWindow() : listener(this)
{
}


MiscScheduleWindow::~MiscScheduleWindow()
{
}

void MiscScheduleWindow::Push(Scenario & scenario)
{
	scenario.isDirty = true;
	undoes.push(scenario);

	if (undoes.size() > 200)
	{
		stack<Scenario> temp;

		while (undoes.size() > 50)
		{
			temp.push(undoes.top());
			undoes.pop();
		}
		while (undoes.size() > 0)
		{
			undoes.pop();
		}
		while (temp.size())
		{
			undoes.push(temp.top());
			temp.pop();
		}
	}

	while (redoes.size())
	{
		redoes.pop();
	}
}

ScheduleChangeListener* MiscScheduleWindow::GetListener()
{
	return &listener;
}

void MiscScheduleWindow::OnGUI(Scenario & scenario)
{
	bool undoVisible = false;
	if (undoes.size())
	{
		undoVisible = true;
		if (ImGui::Button("Undo"))
		{
			scenario.isDirty = true;
			redoes.push(scenario);
			scenario = undoes.top();
			undoes.pop();
		}
	}
	if (redoes.size())
	{
		if (undoVisible)
		{
			ImGui::SameLine();
		}
		if (ImGui::Button("Redo"))
		{
			scenario.isDirty = true;
			undoes.push(scenario);
			scenario = redoes.top();
			redoes.pop();
		}
	}
}
