#pragma once

#include "Scenario.h"
#include "ScheduleChangeListener.h"
#include "IdleReducer.h"

#include <thread>

class ReduceWindow
{
public:
	ReduceWindow(ScheduleChangeListener* changeListener);
	~ReduceWindow();

	void OnGUI(Scenario& scenario);

	int reducerPreference;
	ValType idleTime;
	int finiteCases;
	int totalCases;

	bool waitingResult;

	vector<ValType> finiteCaseTimes;

	IdleReducer* activeReducer;
	ScheduleChangeListener* changeListener;
	thread reducerThread;

	char* filePath;
};

