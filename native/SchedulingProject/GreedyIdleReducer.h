#pragma once

#include "IdleReducer.h"

class GreedyIdleReducer :
	public IdleReducer
{
public:
	GreedyIdleReducer();
	~GreedyIdleReducer();

	void Reduce(Scenario scenario);
	void OnGUI();

private:

	ReduceResults CalculateOptimal(Jobs jobs, int remainingSyncPoints, ValType t);

	bool useT;
};

