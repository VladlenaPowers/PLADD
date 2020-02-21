#include "stdafx.h"
#include "GreedyIdleReducer.h"

#include "JobRun.h"

GreedyIdleReducer::GreedyIdleReducer()
{
}


GreedyIdleReducer::~GreedyIdleReducer()
{
}

ReduceResults GreedyIdleReducer::CalculateOptimal(Jobs jobs, int remainingSyncPoints, ValType t)
{
	if (remainingSyncPoints == 0)
	{
		ReduceResults output;
		output.idleTime = VAL_ZERO;
		output.casesExplored = 1;
		output.finiteCaseTimes.push_back(VAL_ZERO);
		return output;
	}
	else
	{
		int largestServer = -1;
		ValType tempT;

		if (remainingSyncPoints == 1)
		{
			tempT = t;
		}
		else
		{
			//find the largest first job
			for (size_t i = 0; i < jobs.serverCount(); i++)
			{
				if (jobs.jobCount(i))
				{
					ValType currJob = jobs.getJob(i, 0);
					if (largestServer < 0 || (currJob > jobs.getJob(largestServer, 0)))
					{
						largestServer = i;
					}
				}
			}
			tempT = jobs.getJob(largestServer, 0);
			tempT = (tempT < t) ? tempT : t;
		}

		if (remainingSyncPoints != 1 && largestServer < 0)
		{
			//no job to place syncPoint at
			ReduceResults output;
			output.idleTime = VAL_ZERO;
			output.casesExplored = 1;
			output.finiteCaseTimes.push_back(VAL_ZERO);
			return output;
		}
		else
		{

			int best = -1;
			ValType bestIdleTime = VAL_INF;
			for (size_t j = 0; j < jobs.serverCount(); j++)
			{
				if (jobs.jobCount(j) > 0)
				{
					if (j != largestServer && jobs.getJob(j, 0) < tempT)
					{
						vector<ValType> syncPoints;
						syncPoints.push_back(jobs.getJob(j, 0));

						Scenario tempScenario(jobs, syncPoints, tempT, true);
						JobRun run(tempScenario);
						run.Simulate();

						if (best < 0 || run.idleTime < bestIdleTime)
						{
							best = j;
							bestIdleTime = run.idleTime;
						}
					}
				}
			}

			if (best < 0)
			{
				//there is no job left
				ReduceResults output;
				output.idleTime = VAL_ZERO;
				output.casesExplored = 1;
				output.finiteCaseTimes.push_back(VAL_ZERO);
				return output;
			}
			else
			{
				ValType syncPoint = jobs.getJob(best, 0);

				Jobs leftOver;
				ValType idleTime;
				jobs.jobsAfter(syncPoint, leftOver, idleTime, useT);

				ReduceResults subResult = CalculateOptimal(leftOver, remainingSyncPoints - 1, t - syncPoint);

				for (size_t i = 0; i < subResult.syncPoints.size(); i++)
				{
					subResult.syncPoints[i] += syncPoint;
				}
				subResult.syncPoints.insert(subResult.syncPoints.begin(), syncPoint);

				return subResult;
			}
		}
	}
}

void GreedyIdleReducer::Reduce(Scenario scenario)
{
	running = true;
	cancelled = false;
	finished = false;

	//syncPointCount = (syncPointCount < (jobs[0].size() - 1)) ? jobs[0].size() - 1 : syncPointCount;

	useT = scenario.useT;

	int syncPointCount = scenario.jobs.MaxJobCount() - 1;

	if (syncPointCount > 0)
	{
		result = CalculateOptimal(scenario.jobs, syncPointCount, scenario.t);

		//sort(result.finiteCaseTimes.begin(), result.finiteCaseTimes.end());
	}

	running = false;
	finished = true;
}
void GreedyIdleReducer::OnGUI()
{
	//ImGui::Checkbox("Use T", &useT);
}
