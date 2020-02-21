#pragma once

#include "Scenario.h"

class MiscScheduleWindow;

class ScheduleChangeListener
{
public:
	ScheduleChangeListener(MiscScheduleWindow* window);
	~ScheduleChangeListener();

	void Push(Scenario &jd);

	MiscScheduleWindow* window;
};

