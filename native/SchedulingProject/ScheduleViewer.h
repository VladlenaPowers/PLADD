#pragma once

#include "JobRun.h"
#include "Scenario.h"
#include <vector>
#include <stack>

#include "ReduceWindow.h"
#include "ScenarioEditor.h"
#include "ScheduleFileWindow.h"
#include "ScheduleModificationWindow.h"
#include "MiscScheduleWindow.h"
#include "ScheduleChangeListener.h"

using namespace std;

class ScheduleViewer
{
public:
	ScheduleViewer();
	~ScheduleViewer();

	void OnGUI();

private:

	ReduceWindow* reduceWindow;
	ScenarioEditor* editorWindow;
	ScheduleFileWindow* fileWindow;
	ScheduleModificationWindow* modificationWindow;
	MiscScheduleWindow* miscWindow;

	Scenario jd;

	UserSettings settings;

};

