#include "stdafx.h"
#include "ScheduleFileWindow.h"

#include "imgui.h"
#include <fstream>

ScheduleFileWindow::ScheduleFileWindow(ScheduleChangeListener* changeListener) : changeListener(changeListener)
{
	canLoadFile = false;
	filePath = new char[FILEPATH_BUF_SIZE];
	for (size_t i = 0; i < FILEPATH_BUF_SIZE; i++)
	{
		filePath[i] = '\0';
	}
}


ScheduleFileWindow::~ScheduleFileWindow()
{
}

void ScheduleFileWindow::OnGUI(Scenario & scenario)
{
	bool checkFileExists = ImGui::InputText("Path", filePath, FILEPATH_BUF_SIZE);
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		scenario.SaveToFile(filePath);
	}
	if (checkFileExists)
	{
		ifstream f(filePath);
		canLoadFile = f.good();
	}
	if (canLoadFile)
	{
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			changeListener->Push(scenario);
			scenario = Scenario::LoadFromFile(filePath);
		}
	}
}
