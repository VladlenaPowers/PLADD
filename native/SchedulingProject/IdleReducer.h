#pragma once

#include "Scenario.h"
#include <vector>

using namespace std;

struct ReduceResults
{
	ValType idleTime;
	vector<ValType> syncPoints;
	size_t casesExplored;
	vector<ValType> finiteCaseTimes;
};

class IdleReducer
{
public:
	IdleReducer();
	virtual ~IdleReducer();

	bool Cancelled();
	bool Running();
	bool Finished();
	float GetProgress();
	ReduceResults GetResult();
	void Cancel();
	void SetProgress(double progress);
	void IncProgress(double amount);
	void SetRunning(bool state);
	virtual void Reduce(Scenario scenario);
	virtual void OnGUI();

protected:

	ReduceResults result;
	bool cancelled;
	bool running;
	double progress;
	bool finished;
};

