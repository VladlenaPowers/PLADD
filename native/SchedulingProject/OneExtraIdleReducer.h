#pragma once

#include "IdleReducer.h"

class OneExtraIdleReducer :
	public IdleReducer
{
public:
	OneExtraIdleReducer();
	~OneExtraIdleReducer();

	void Reduce(Scenario scenario);
	void OnGUI();
};

