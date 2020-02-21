#pragma once

#include "Scenario.h"
#include "JobRun.h"

#include "ScheduleChangeListener.h"

class ScenarioEditor
{
public:
	ScenarioEditor(ScheduleChangeListener* changeListener);
	~ScenarioEditor();

	void OnGUI(Scenario & scenario);
	void DrawJobRun(JobRun & jobRun);

	JobRun jr;

	float** colors;
	int colorCount;

	int selectedServer;
	int selectedJob;

	int selectedSyncPoint;

	bool tLineSelected;

	ScheduleChangeListener* changeListener;

	JobRun* snapshot;

private:

	void HandleTGUI();

};

