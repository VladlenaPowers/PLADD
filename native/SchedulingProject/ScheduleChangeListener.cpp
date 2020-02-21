#include "stdafx.h"
#include "ScheduleChangeListener.h"

#include "MiscScheduleWindow.h"

ScheduleChangeListener::ScheduleChangeListener(MiscScheduleWindow* window) : window(window)
{
}


ScheduleChangeListener::~ScheduleChangeListener()
{
}

void ScheduleChangeListener::Push(Scenario &jd)
{
	window->Push(jd);
}
