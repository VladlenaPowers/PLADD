#include "stdafx.h"
#include "ScenarioEditor.h"

#include "imgui.h"
#include <algorithm>
#include <sstream>

#define BACKGROUND_COLOR 0xffffffff
#define LINE_COLOR 0xff333333
#define LINE_RED_COLOR 0xff3333ff
#define IDLE_COLOR 0x00cccccc

static float UIScale = 1.0f;
static float jobWidth = 20.0f * UIScale;
static float borderPadding = 40.0f * UIScale;
static float jobSpacing = 5.0f * UIScale;
static float syncLineThickness = 2.0f * UIScale;
static float syncLineRunoff = 15.0f * UIScale;

ScenarioEditor::ScenarioEditor(ScheduleChangeListener* changeListener) : changeListener(changeListener), jr()
{
	int hues[16] = {
		0,
		164,
		21,
		87,
		212,
		142,
		30,
		130,
		142,
		0,
		164,
		21,
		87,
		212,
		142,
		30
	};


	colors = new float*[MAX_JOBS];
	for (size_t i = 0; i < MAX_JOBS; i++)
	{
		ImColor col = ImColor::HSV(255.0f / hues[i], 195.0f / 255.0f, 195.0f / 255.0f);
		ImVec4 comps = col;

		colors[i] = new float[3];
		colors[i][0] = comps.x;
		colors[i][1] = comps.y;
		colors[i][2] = comps.z;
	}

	colorCount = MAX_JOBS;


	selectedServer = -1;
	selectedJob = -1;

	selectedSyncPoint = -1;

	tLineSelected = false;

	snapshot = NULL;
}


ScenarioEditor::~ScenarioEditor()
{
}

void ScenarioEditor::OnGUI(Scenario & scenario)
{
	if (snapshot)
	{
		DrawJobRun(*snapshot);
	}

	if (scenario.isDirty)
	{
		jr = JobRun(scenario);
		jr.Simulate();
		scenario.isDirty = false;
	}


	ImGuiIO io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	ImVec2 reg = ImGui::GetContentRegionAvail();
	ImVec2 tl = ImGui::GetCursorScreenPos();
	ImVec2 br(tl.x + reg.x, tl.y + reg.x);
	ImVec2 tlCorner(tl);


	float timeScale = reg.x * UIScale / 5.0f / VAL_DEF / 10.0f;

	size_t rows = scenario.jobs.serverCount() + 1;
	dl->AddRectFilled(tl, ImVec2(tl.x + reg.x, tl.y + (2.0f * borderPadding) + (jobSpacing * (rows - 1)) + (jobWidth * rows)), BACKGROUND_COLOR);

	int removeServerI = -1;

	tlCorner.y += borderPadding;
	tlCorner.x = tl.x + borderPadding;
	float jobChartTop = tlCorner.y;
	for (size_t i = 0; i < scenario.jobs.serverCount(); i++)
	{
		/*ss.str(string());
		ss << i;
		dl->AddText(tlCorner, 0xffffffff, ss.str().c_str());
		tlCorner.x += jobWidth;*/
		size_t colI = 0;
		float preceedingEnd = tlCorner.x;
		for (size_t j = 0; j < scenario.jobs.jobCount(i); j++)
		{
			float jobLength = scenario.jobs.getJob(i, j) * timeScale;

			ImVec4 col;
			col.x = colors[colI][0];
			col.y = colors[colI][1];
			col.z = colors[colI][2];
			col.w = 1.0f;

			ImVec2 tlCornerJob(tlCorner.x + (jr.jobStarts[i][j] * timeScale), tlCorner.y);
			ImVec2 brCorner(tlCornerJob.x + jobLength, tlCornerJob.y + jobWidth);


			if (j <= jr.lastJob[i])
			{
				//draw idle time before this job
				dl->AddRectFilled(ImVec2(preceedingEnd, tlCornerJob.y), ImVec2(tlCornerJob.x, brCorner.y), IDLE_COLOR);
			}
			else
			{
				col.x *= 0.25f;
				col.y *= 0.25f;
				col.z *= 0.25f;
			}


			if ((io.MousePos.x > tlCornerJob.x && io.MousePos.x < brCorner.x) && (io.MousePos.y > tlCornerJob.y && io.MousePos.y < brCorner.y))
			{
				col.x *= 2.0f;
				col.y *= 2.0f;
				col.z *= 2.0f;
				if (io.MouseClicked[0])
				{
					selectedServer = i;
					selectedJob = j;
				}
			}

			dl->AddRectFilled(tlCornerJob, brCorner, ImGui::ColorConvertFloat4ToU32(col));

			if (i == selectedServer && j == selectedJob)
			{
				//draw white border
				dl->AddRect(tlCornerJob, brCorner, 0xffffffff);
			}

			preceedingEnd = brCorner.x;
			colI = (colI + 1) % MAX_JOBS;
		}


		ImGui::PushID(9000 + i);

		preceedingEnd += jobSpacing;

		if (scenario.jobs.jobCount(i) > 0)
		{
			ImGui::SetCursorScreenPos(ImVec2(preceedingEnd, tlCorner.y));
			if (ImGui::Button("-", ImVec2(jobWidth, jobWidth)))
			{
				changeListener->Push(scenario);
				scenario.jobs.removeJob(i, scenario.jobs.jobCount(i) - 1);
				scenario.isDirty = true;
			}

		}
		else 
		{
			ImGui::SetCursorScreenPos(ImVec2(preceedingEnd, tlCorner.y));
			if (ImGui::Button("X", ImVec2(jobWidth, jobWidth)))
			{
				changeListener->Push(scenario);
				removeServerI = i;
				scenario.isDirty = true;
			}
		}

		preceedingEnd += jobWidth;
		preceedingEnd += jobSpacing;

		ImGui::SetCursorScreenPos(ImVec2(preceedingEnd, tlCorner.y));
		if (ImGui::Button("+", ImVec2(jobWidth, jobWidth)))
		{
			changeListener->Push(scenario);
			scenario.jobs.addJob(i, VAL_DEF);
			scenario.isDirty = true;
		}

		ImGui::PopID();

		tlCorner.y += jobWidth;
		if (true)
		{
			tlCorner.y += jobSpacing;
		}
	}

	ImGui::SetCursorScreenPos(ImVec2(tlCorner.x + jobSpacing, tlCorner.y));
	if (ImGui::Button("+", ImVec2(jobWidth, jobWidth)))
	{
		changeListener->Push(scenario);
		scenario.jobs.addServer();
		scenario.isDirty = true;
	}

	tlCorner.y += jobWidth;

	float jobChartBottom = tlCorner.y;

	tlCorner.y += borderPadding;

	br.y = tlCorner.y;

	float horizontalPos = tl.x + borderPadding;
	dl->AddLine(ImVec2(horizontalPos, jobChartTop - syncLineRunoff), ImVec2(horizontalPos, jobChartBottom + syncLineRunoff), LINE_COLOR, syncLineThickness);

	for (size_t i = 0; i < scenario.syncPoints.size(); i++)
	{
		if (!tLineSelected && io.MouseDown[0])
		{
			if (selectedSyncPoint < 0)
			{
				if ((io.MousePos.y > tl.y && io.MousePos.y < br.y) && (io.MousePos.x > tl.x && io.MousePos.x < br.x))
				{
					float dist = io.MousePos.x - (tl.x + borderPadding + (scenario.syncPoints[i] * timeScale));
					if (dist < 0.0f)
						dist = -dist;
					if (dist <= 4.0f)
					{
						selectedSyncPoint = i;
					}
				}
			}
			else if (selectedSyncPoint == i)
			{
				scenario.isDirty = true;
				scenario.syncPoints[i] += io.MouseDelta.x / timeScale;

				if (scenario.useT)
				{
					if (scenario.syncPoints[i] > scenario.t)
					{
						scenario.syncPoints[i] = scenario.t;
						selectedSyncPoint = -1;

						sort(scenario.syncPoints.begin(), scenario.syncPoints.end());
					}
				}
			}
		}
		else
		{
			if (selectedSyncPoint == i)
			{
				changeListener->Push(scenario);
				selectedSyncPoint = -1;

				sort(scenario.syncPoints.begin(), scenario.syncPoints.end());
			}
		}

		horizontalPos = tl.x + borderPadding + (scenario.syncPoints[i] * timeScale);
		dl->AddLine(ImVec2(horizontalPos, jobChartTop - syncLineRunoff), ImVec2(horizontalPos, jobChartBottom + syncLineRunoff), LINE_COLOR, syncLineThickness);
	}

	if (scenario.useT)
	{
		if ((selectedSyncPoint < 0) && io.MouseDown[0])
		{
			if (!tLineSelected)
			{
				if ((io.MousePos.y > tl.y && io.MousePos.y < br.y) && (io.MousePos.x > tl.x && io.MousePos.x < br.x))
				{
					float dist = io.MousePos.x - (tl.x + borderPadding + (scenario.t * timeScale));
					if (dist < 0.0f)
						dist = -dist;
					if (dist <= 4.0f)
					{
						tLineSelected = true;
					}
				}
			}
			else
			{
				scenario.isDirty = true;
				scenario.t += io.MouseDelta.x / timeScale;

				for (size_t i = 0; i < scenario.syncPoints.size(); i++)
				{
					if (scenario.syncPoints[i] > scenario.t)
					{
						scenario.syncPoints[i] = scenario.t;
					}
				}
			}
		}
		else
		{
			if (tLineSelected)
			{
				changeListener->Push(scenario);
				tLineSelected = false;
			}
		}

		horizontalPos = tl.x + borderPadding + (scenario.t * timeScale);
		dl->AddLine(ImVec2(horizontalPos, jobChartTop - syncLineRunoff), ImVec2(horizontalPos, jobChartBottom + syncLineRunoff), LINE_RED_COLOR, syncLineThickness);
	}
	
	dl->AddRect(tl, ImVec2(tl.x + reg.x, tlCorner.y), LINE_COLOR);

	//this concludes the drawind code for the job run

	ImGui::PushItemWidth(110.0f);
	ImGui::SetCursorScreenPos(tl);
	ImGui::InvisibleButton("Background", ImVec2(reg.x, tlCorner.y - tl.y));

	if (removeServerI >= 0)
	{
		scenario.jobs.removeServer(removeServerI);
	}

	//ImGui::ColorEdit3("Job color", colors[selectedJob]);

	ImGuiStyle style = ImGui::GetStyle();

	float itemWidth = (reg.x - (8.0f * style.ItemSpacing.x)) / 8.0f;

	if (ImGui::Button("Capture", ImVec2(itemWidth, 0)))
	{
		if (snapshot)
		{
			delete snapshot;
			snapshot = NULL;
		}
		snapshot = new JobRun(scenario);
	}
	ImGui::SameLine();
	if (ImGui::Button("Hide", ImVec2(itemWidth, 0)))
	{
		if (snapshot)
		{
			delete snapshot;
			snapshot = NULL;
		}
	}

	const size_t SPACING = 40;

	if ((selectedServer >= 0) && (selectedJob >= 0))
	{
		if (selectedServer < scenario.jobs.serverCount() && selectedJob < scenario.jobs.jobCount(selectedServer))
		{
			ValType jobLength = scenario.jobs.getJob(selectedServer, selectedJob);

			ImGui::SameLine();
			ImGui::InvisibleButton("##InvButton3", ImVec2(SPACING, 1));
			ImGui::SameLine();

			if (ImGui::InputInt("Job", &jobLength, 10, 100))
			{
				changeListener->Push(scenario);
				scenario.jobs.setJob(selectedServer, selectedJob, jobLength);
			}
		}
		else
		{
			selectedServer = -1;
			selectedJob = -1;
		}
	}

	ImGui::SameLine();
	ImGui::InvisibleButton("##InvButton2", ImVec2(SPACING, 1));
	ImGui::SameLine();

	if (ImGui::InputFloat("UI Scale", &UIScale, 0.05f, 0.5f))
	{
		changeListener->Push(scenario);
	}

	if (scenario.useT)
	{
		if (ImGui::InputInt("T", &scenario.t, 10, 100))
		{
			changeListener->Push(scenario);
		}

		ImGui::SameLine();
		ImGui::InvisibleButton("##InvButton4", ImVec2(SPACING, 1));
		ImGui::SameLine();

		if (ImGui::Button("Don't use T"))
		{
			scenario.useT = false;
			scenario.isDirty = true;
		}
	}
	else
	{
		if (ImGui::Button("Use T"))
		{
			/*sort(scenario.syncPoints.begin(), scenario.syncPoints.end());
			if (scenario.syncPoints.size())
			{
				scenario.t = scenario.syncPoints.back() + VAL_DEF;
			}
			else
			{
				scenario.t = VAL_DEF;
			}*/
			scenario.useT = true;
			scenario.isDirty = true;
		}
	}

	stringstream ss;

	ss.str(string());
	if (jr.idleTime == VAL_INF || jr.idleTime < 0)
	{
		ss << "inf";
	}
	else
	{
		ss << jr.idleTime;
	}
	ImGui::LabelText(ss.str().c_str(), "Idle time:");

	ImGui::SameLine();
	ImGui::InvisibleButton("##InvButton", ImVec2(100, 1));
	ImGui::SameLine();

	ss.str(string());
	ss << jr.data.jobs.MaxJobCount();
	ImGui::LabelText(ss.str().c_str(), "Max Job count:");
}


void ScenarioEditor::DrawJobRun(JobRun & jobRun)
{
	ImVec2 reg = ImGui::GetContentRegionAvail();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	float timeScale = reg.x * UIScale / 5.0f / VAL_DEF / 10.0f;

	ImVec2 tl = ImGui::GetCursorScreenPos();
	ImVec2 tlCorner(tl);


	dl->AddRectFilled(tl, ImVec2(tl.x + reg.x, tl.y + (2.0f * borderPadding) + (jobSpacing * (jobRun.data.jobs.serverCount() - 1)) + (jobWidth * jobRun.data.jobs.serverCount())), BACKGROUND_COLOR);

	tlCorner.y += borderPadding;
	float jobChartTop = tlCorner.y;
	for (size_t i = 0; i < jobRun.data.jobs.serverCount(); i++)
	{
		tlCorner.x = tl.x + borderPadding;

		size_t colI = 0;
		float preceedingEnd = tlCorner.x;

		for (size_t j = 0; j < jobRun.data.jobs.jobCount(i); j++)
		{
			float jobLength = jobRun.data.jobs.getJob(i, j) * timeScale;

			ImVec4 col;
			col.x = colors[colI][0];
			col.y = colors[colI][1];
			col.z = colors[colI][2];
			col.w = 1.0f;

			ImVec2 tlCornerJob(tlCorner.x + (jobRun.jobStarts[i][j] * timeScale), tlCorner.y);
			ImVec2 brCorner(tlCornerJob.x + jobLength, tlCornerJob.y + jobWidth);


			if (j <= jobRun.lastJob[i])
			{
				//draw idle time before this job
				dl->AddRectFilled(ImVec2(preceedingEnd, tlCornerJob.y), ImVec2(tlCornerJob.x, brCorner.y), IDLE_COLOR);
			}
			else
			{
				col.x *= 0.25f;
				col.y *= 0.25f;
				col.z *= 0.25f;
			}

			dl->AddRectFilled(tlCornerJob, brCorner, ImGui::ColorConvertFloat4ToU32(col));

			preceedingEnd = brCorner.x;
			colI = (colI + 1) % MAX_JOBS;

		}

		tlCorner.y += jobWidth;
		if (i < jobRun.data.jobs.serverCount() - 1)
		{
			tlCorner.y += jobSpacing;
		}
	}

	float jobChartBottom = tlCorner.y;
	tlCorner.y += borderPadding;

	float jobChartB = tlCorner.y;

	float horizontalPos = tl.x + borderPadding;
	dl->AddLine(ImVec2(horizontalPos, jobChartTop - syncLineRunoff), ImVec2(horizontalPos, jobChartBottom + syncLineRunoff), LINE_COLOR, syncLineThickness);
	for (size_t i = 0; i < jobRun.data.syncPoints.size(); i++)
	{
		horizontalPos = tl.x + borderPadding + (jobRun.data.syncPoints[i] * timeScale);
		dl->AddLine(ImVec2(horizontalPos, jobChartTop - syncLineRunoff), ImVec2(horizontalPos, jobChartBottom + syncLineRunoff), LINE_COLOR, syncLineThickness);
	}

	if (jobRun.data.useT)
	{
		horizontalPos = tl.x + borderPadding + (jobRun.data.t * timeScale);
		dl->AddLine(ImVec2(horizontalPos, jobChartTop - syncLineRunoff), ImVec2(horizontalPos, jobChartBottom + syncLineRunoff), LINE_RED_COLOR, syncLineThickness);
	}

	dl->AddRect(tl, ImVec2(tl.x + reg.x, tlCorner.y), LINE_COLOR);
	ImGui::InvisibleButton("Background", ImVec2(reg.x, tlCorner.y - tl.y));
}

void ScenarioEditor::HandleTGUI()
{

}