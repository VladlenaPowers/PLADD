#pragma once
#include "IdleReducer.h"
class HybridGreedyIdleReducer :
	public IdleReducer
{
public:
	HybridGreedyIdleReducer();
	~HybridGreedyIdleReducer();

	void Reduce(Scenario scenario);
	void OnGUI();

private:

	int blockSize;
	int fixedTakes;

	ReduceResults OuterLoop(Jobs jobs, int remainingSyncPoints, ValType t, double progressInc);

	ReduceResults CalculateOptimal(Jobs jobs, int remainingSyncPoints, ValType t, double progressInc);

	bool useT;
};

