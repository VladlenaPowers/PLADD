#pragma once

#include "Scenario.h"
#include "ScheduleChangeListener.h"

class ScheduleModificationWindow
{
public:
	ScheduleModificationWindow(ScheduleChangeListener* changeListener);
	~ScheduleModificationWindow();

	void (*modifyCallback)(Scenario & scenario);

	void RandomizeNormal(Scenario & scenario);
	void RandomizeUniform(Scenario & scenario);
	void Constant(Scenario & scenario);
	void OnGUI(Scenario & scenario);

	float uniformMin;
	float uniformMax;

	float normalMean;
	float normalSigma;

	ScheduleChangeListener* changeListener;

	float constant;
};

