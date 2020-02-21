#include "stdafx.h"
#include "IdleReducer.h"


IdleReducer::IdleReducer()
{
	running = false;
	cancelled = false;
	finished = false;
}


IdleReducer::~IdleReducer()
{
}

bool IdleReducer::Cancelled()
{
	return cancelled;
}
bool IdleReducer::Running()
{
	return running;
}
bool IdleReducer::Finished()
{
	return finished;
}

float IdleReducer::GetProgress()
{
	return progress;
}
ReduceResults IdleReducer::GetResult()
{
	return result;
}

void IdleReducer::Cancel()
{
	cancelled = true;
}

void IdleReducer::SetProgress(double progress)
{
	IdleReducer::progress = progress;
}
void IdleReducer::IncProgress(double amount)
{
	progress += amount;
	if (progress > 1.0f)
	{
		progress = 1.0f;
	}
}
void IdleReducer::SetRunning(bool state)
{
	running = state;
}

void IdleReducer::Reduce(Scenario scenario)
{
}
void IdleReducer::OnGUI()
{
}