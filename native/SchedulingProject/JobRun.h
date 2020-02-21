#pragma once

#include "Scenario.h"

class JobRun
{
public:
	JobRun();
	JobRun(Scenario & data);
	~JobRun();

	void Simulate();

	Scenario data;
	vector<vector<ValType>> jobStarts;
	vector<size_t> lastJob;

	ValType idleTime;
};

