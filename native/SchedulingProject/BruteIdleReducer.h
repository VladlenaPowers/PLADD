#pragma once

#include "IdleReducer.h"

class BruteIdleReducer :
	public IdleReducer
{
public:
	BruteIdleReducer();
	~BruteIdleReducer();

	void Reduce(Scenario scenario);
	void OnGUI();

private:

	ReduceResults CalculateOptimal(Jobs jobs, int remainingSyncPoints, ValType t, double progressInc);

	bool useT;
};

