#include "stdafx.h"
#include "ScheduleViewer.h"

#include "imgui.h"
#include <algorithm>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>

#include "OneExtraIdleReducer.h"
#include "BruteIdleReducer.h"

using namespace std;

bool CompareFirstElements(vector<ValType> & a, vector<ValType> & b)
{
	return a.front() < b.front();
}
static void FiniteCasesAux(vector<vector<ValType>> jobs, int remainingSyncPoints, long & count)
{
	sort(jobs.begin(), jobs.end(), CompareFirstElements);

	bool ranOnce = false;
	for (size_t i = 0; i < jobs.size(); i++)
	{
		bool skip = (i > 0) ? jobs[i].front() == jobs[i-1].front() : false;
		if (jobs[i].size() > 1 && !skip)
		{
			ranOnce = true;
			if (remainingSyncPoints == 0)
			{
				return;
			}

			//in this context we have chosen the end of the ith job to place
			//a syncronization point. we now slice the jobs to create a
			//sub-problem to solve.
			vector<vector<ValType>> jobs2;
			for (size_t j = 0; j < jobs.size(); j++)
			{
				if (jobs[j].size() > 1)
				{
					jobs2.push_back(vector<ValType>());
					if (j != i)
					{
						if (jobs[j].front() > jobs[i].front())
						{
							jobs2.back().push_back(jobs[j].front() - jobs[i].front());
						}
					}
					for (size_t k = 1; k < jobs[j].size(); k++)
					{
						jobs2.back().push_back(jobs[j][k]);
					}
				}
			}

			FiniteCasesAux(jobs2, remainingSyncPoints - 1, count);
		}
	}

	if (!ranOnce && (remainingSyncPoints == 0))
	{
		count++;
	}
}
static size_t FiniteCases(vector<vector<ValType>> jobs, int remainingSyncPoints)
{
	long temp = 0;

	FiniteCasesAux(jobs, remainingSyncPoints, temp);

	return temp;
}

ScheduleViewer::ScheduleViewer()
{

	vector<int> syncPoints;
	syncPoints.push_back(20000);
	syncPoints.push_back(40000);
	syncPoints.push_back(60000);
	syncPoints.push_back(80000);

	vector<vector<int>> exJobs;
	exJobs.push_back(vector<int>());
	exJobs.back().push_back(35500);
	exJobs.back().push_back(VAL_DEF);
	exJobs.back().push_back(VAL_DEF);
	exJobs.push_back(vector<int>());
	exJobs.back().push_back(16000);
	exJobs.back().push_back(16000);
	exJobs.back().push_back(34700);
	exJobs.push_back(vector<int>());
	exJobs.back().push_back(VAL_DEF);
	exJobs.back().push_back(38500);
	exJobs.back().push_back(VAL_DEF);

	jd = Scenario(exJobs, syncPoints, 10 * VAL_DEF, false);

	settings.includeRedundantSyncPoints = false;



	miscWindow = new MiscScheduleWindow();
	ScheduleChangeListener* changeListener = miscWindow->GetListener();
	reduceWindow = new ReduceWindow(changeListener);
	editorWindow = new ScenarioEditor(changeListener);
	fileWindow = new ScheduleFileWindow(changeListener);
	modificationWindow = new ScheduleModificationWindow(changeListener);
}


ScheduleViewer::~ScheduleViewer()
{
}

void ScheduleViewer::OnGUI()
{
	static float syncPointButtonSize = 20.0f;
	static int colorJobHover = 0xffffffff;

	ImGuiIO io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	//Draw working schedule

	//get the boundaries of this item
	ImVec2 reg = ImGui::GetContentRegionAvail();
	ImVec2 tl = ImGui::GetCursorScreenPos();
	ImVec2 br(tl.x + reg.x, tl.y + reg.x);
	ImVec2 tlCorner(tl);

	stringstream ss;

	ImGuiStyle style = ImGui::GetStyle();

	editorWindow->OnGUI(jd);

	float windowWidth = (ImGui::GetWindowContentRegionWidth() * 0.5f) - style.FramePadding.x;

	const int FIRST_ROW_HEIGHT = 300;

	ImGui::BeginChild("Reduce Window", ImVec2(windowWidth, FIRST_ROW_HEIGHT), true);

	reduceWindow->OnGUI(jd);

	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("Modification Window", ImVec2(windowWidth, FIRST_ROW_HEIGHT), true);

	modificationWindow->OnGUI(jd);

	ImGui::EndChild();

	ImGui::BeginChild("File Window", ImVec2(windowWidth, 50), true);

	fileWindow->OnGUI(jd);

	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("Misc Window", ImVec2(windowWidth, 50), true);

	miscWindow->OnGUI(jd);

	ImGui::EndChild();

}