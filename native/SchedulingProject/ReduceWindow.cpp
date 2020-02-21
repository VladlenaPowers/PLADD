#include "stdafx.h"
#include "ReduceWindow.h"

#include "imgui.h"
#include <map>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>

#include "BruteIdleReducer.h"
#include "OneExtraIdleReducer.h"
#include "HybridGreedyIdleReducer.h"
#include "GreedyIdleReducer.h"

using namespace std;

#define REDUCER_BRUTE 0
#define REDUCER_HYBRID_GREEDY 1
#define REDUCER_ONE_EXTRA 2
#define REDUCER_GREEDY 3

map<int, IdleReducer*> reducers = {
	{ REDUCER_BRUTE, static_cast<IdleReducer*>(new BruteIdleReducer()) },
	{ REDUCER_HYBRID_GREEDY, static_cast<IdleReducer*>(new HybridGreedyIdleReducer()) },
	{ REDUCER_ONE_EXTRA, static_cast<IdleReducer*>(new OneExtraIdleReducer()) },
	{ REDUCER_GREEDY, static_cast<IdleReducer*>(new GreedyIdleReducer()) }
};

void ReduceFunction(IdleReducer* reducer, Scenario* scenarioP)
{
	reducer->Reduce(*scenarioP);

	//Scenario* scenarioP = new Scenario(scenario);

	delete scenarioP;
}

ReduceWindow::ReduceWindow(ScheduleChangeListener* changeListener) : changeListener(changeListener)
{
	reducerPreference = REDUCER_BRUTE;
	activeReducer = reducers[reducerPreference];

	finiteCases = -1;
	totalCases = -1;

	idleTime = VAL_ZERO;

	filePath = new char[FILEPATH_BUF_SIZE];
	for (size_t i = 0; i < FILEPATH_BUF_SIZE; i++)
	{
		filePath[i] = '\0';
	}

	strcpy_s(filePath, FILEPATH_BUF_SIZE, "data.csv");

}


ReduceWindow::~ReduceWindow()
{
}

void ExportCSV(char* filePath, vector<ValType> values)
{
	ofstream ofs(filePath, ifstream::out);

	for (size_t i = 0; i < values.size(); i++)
	{
		ofs << (values[i] / (float)VAL_DEF) << endl;
	}

}

void ReduceWindow::OnGUI(Scenario & scenario)
{
	ImGuiStyle style = ImGui::GetStyle();
	ImVec2 reg = ImGui::GetContentRegionAvail();

	float windowWidth = (ImGui::GetWindowContentRegionWidth() * 0.5f) - style.FramePadding.x;

	//
	ImGui::BeginChild("Statistics", ImVec2(windowWidth, reg.y), true);
	{
		ImGui::PushItemWidth(100.0f);

		stringstream ss;

		ss.str(string());
		if (idleTime == VAL_INF || idleTime < 0)
		{
			ss << "inf";
		}
		else
		{
			ss << idleTime;
		}
		ImGui::LabelText(ss.str().c_str(), "Min idle time: ");

		ss.str(string());
		ss << finiteCases;
		ImGui::LabelText(ss.str().c_str(), "Finite cases: ");

		ss.str(string());
		ss << totalCases;
		ImGui::LabelText(ss.str().c_str(), "Total cases: ");

		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	//

	ImGui::SameLine();

	//
	ImGui::BeginChild("Modification Window", ImVec2(windowWidth, reg.y), true);
	{
		if (waitingResult)
		{
			if (activeReducer->Running())
			{
				if (ImGui::Button("Cancel"))
				{
					activeReducer->Cancel();
				}

				//draw progress bar
				ImVec2 size(ImGui::GetContentRegionAvailWidth(), 25.0f);
				ImVec2 pos = ImGui::GetCursorScreenPos();

				float progress = activeReducer->GetProgress();

				ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + (size.x * progress), pos.y + size.y), 0xffffffff); // fill
				ImGui::GetWindowDrawList()->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), 0xffffffff); // border
				ImGui::Dummy(size);
			}
			else if (activeReducer->Finished())
			{
				//get result
				ReduceResults results = activeReducer->GetResult();
				scenario.syncPoints = results.syncPoints;
				idleTime = results.idleTime;
				finiteCaseTimes = results.finiteCaseTimes;
				finiteCases = finiteCaseTimes.size();
				totalCases = results.casesExplored;
				scenario.isDirty = true;

				reducerThread.join();

				waitingResult = false;
			}

		}
		else
		{
			if (ImGui::RadioButton("Brute", &reducerPreference, REDUCER_BRUTE) ||
				ImGui::RadioButton("Hybrid Greedy", &reducerPreference, REDUCER_HYBRID_GREEDY) ||
				//ImGui::RadioButton("One Extra", &reducerPreference, REDUCER_ONE_EXTRA) ||
				ImGui::RadioButton("Greedy", &reducerPreference, REDUCER_GREEDY))
			{
				activeReducer = reducers[reducerPreference];
			}

			ImGuiStyle& style = ImGui::GetStyle();
			float windowWidth = ImGui::GetWindowContentRegionWidth() - style.FramePadding.x;
			//ImGui::BeginChild("Reduce Parameters", ImVec2(windowWidth, 40), true);
			//activeReducer->OnGUI();
			//ImGui::EndChild();

			if (ImGui::Button("Reduce"))
			{
				changeListener->Push(scenario);

				activeReducer->SetRunning(true);

				Scenario* temp = new Scenario(scenario);
				reducerThread = thread(ReduceFunction, activeReducer, temp);
				/*reducerThread = thread([&]() {
					activeReducer->Reduce(temp);
				});*/

				waitingResult = true;
			}
		}


		//ImGui::PushItemWidth((windowWidth - style.ItemSpacing.x) * 0.5f);

		ImGui::InputText("##Path", filePath, FILEPATH_BUF_SIZE);
		ImGui::SameLine();
		if (ImGui::Button("Export CSV"))
		{
			ExportCSV(filePath, finiteCaseTimes);
		}

		activeReducer->OnGUI();

		//ImGui::PopItemWidth();
	}

	ImGui::EndChild();
	//

}
