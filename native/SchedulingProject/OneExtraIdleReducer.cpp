#include "stdafx.h"
#include "OneExtraIdleReducer.h"
#include <math.h>
#include <algorithm>
#include "Min.h"

struct ServerRecord
{
	vector<ValType> jobs;
	bool crossed;

	void operator=(const ServerRecord &rhs)
	{
		jobs = rhs.jobs;
		crossed = rhs.crossed;
	}
};

bool SortDescendingByFirstEndPoint(const ServerRecord & a, const ServerRecord & b)
{
	if (a.jobs.front() > b.jobs.front())
	{
		return true;
	}
	else if (a.jobs.front() == b.jobs.front())
	{
		if (a.crossed)
		{
			if (b.crossed)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			if (b.crossed)
			{
				return false;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}

bool SortServersByFirstJobLength(const ServerRecord & a, const ServerRecord & b)
{
	return a.jobs.front() > b.jobs.front();
}

OneExtraIdleReducer::OneExtraIdleReducer()
{
}


OneExtraIdleReducer::~OneExtraIdleReducer()
{
}

vector<vector<ValType>> cutFirstAt(ValType at, vector<vector<ValType>> & src)
{
	vector<vector<ValType>> output;

	for (size_t i = 0; i < src.size(); i++)
	{
		output.push_back(vector<ValType>());

		if (src[i].front() > at)
		{
			output[i].push_back(src[i].front() - at);
		}

		for (size_t j = 1; j < src[i].size(); j++)
		{
			output[i].push_back(src[i][j]);
		}
	}

	return output;
}

template<typename T>
vector<vector<T>> removeFirst(vector<vector<T>> & src)
{
	vector<vector<T>> output;

	for (size_t i = 0; i < src.size(); i++)
	{
		output.push_back(vector<T>());
		for (size_t j = 1; j < src[i].size(); j++)
		{
			output[i].push_back(src[i][j]);
		}
	}

	return output;
}

template<typename T>
T maxFirst(vector<vector<T>> & src)
{
	vector<vector<T>> output;

	T max = VAL_ZERO;
	for (size_t i = 0; i < src.size(); i++)
	{
		if (src[i].size())
		{
			if (src[i][0] > max)
			{
				max = src[i][0];
			}
		}
	}

	return max;
}

vector<ServerRecord> ClipSchedule(vector<ServerRecord> & servers, ValType cutTime)
{
	vector<ServerRecord> output;

	for (size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i].jobs.front() > cutTime)
		{
			output.push_back(ServerRecord());
			output.back().jobs = servers[i].jobs;
			output.back().jobs.front() -= cutTime;
			output.back().crossed = true;
		}
		else
		{
			if (servers[i].jobs.size() > 1)
			{
				output.push_back(ServerRecord());
				for (size_t j = 1; j < servers[i].jobs.size(); j++)
				{
					output.back().jobs.push_back(servers[i].jobs[j]);
				}
				output.back().crossed = servers[i].crossed;
			}
		}
	}

	return output;
}

ReduceResults ReduceAdjust(vector<ServerRecord> & servers)
{
	std::sort(servers.begin(), servers.end(), SortDescendingByFirstEndPoint);

	//sorting variables
	struct SortingRecord
	{
		ValType idletime = VAL_INF;//key
		ReduceResults result;
		ValType syncPoint;
	} minResult;
	//

	minResult.result.casesExplored = 0;
	minResult.result.idleTime = VAL_INF;

	bool greaterThanOne = false;
	bool oneCrossing = false;
	for (size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i].jobs.size() > 1)
		{
			greaterThanOne = true;
		}
		if (servers[i].crossed)
		{
			oneCrossing = true;
		}
	}

	if (!greaterThanOne)
	{
		if (oneCrossing)
		{
			minResult.result.idleTime = VAL_ZERO;
			minResult.result.casesExplored = 1;
			minResult.result.finiteCaseTimes.push_back(VAL_ZERO);
			return minResult.result;
		}
		else
		{
			return minResult.result;
		}
	}

	size_t casesExplored = 0;
	vector<ValType> finiteCaseIdleTimes;

	for (size_t i = 0; i < servers.size(); i++)
	{
		ValType syncPoint = servers[i].jobs[0];
		ValType idleTime = syncPoint * (servers.size() - i - 1);
		for (size_t j = i + 1; j < servers.size(); j++)
		{
			idleTime -= servers[j].jobs[0];
		}

		//cut schedule at sync point at the end point of the first job of the ith server
		vector<ServerRecord> subServers = ClipSchedule(servers, syncPoint);
		ReduceResults subResult = ReduceAdjust(subServers);

		for (size_t j = 0; j < subResult.finiteCaseTimes.size(); j++)
		{
			subResult.finiteCaseTimes[j] += idleTime;
		}

		casesExplored += subResult.casesExplored;
		finiteCaseIdleTimes.insert(finiteCaseIdleTimes.end(), subResult.finiteCaseTimes.begin(), subResult.finiteCaseTimes.end());

		if (subResult.idleTime < VAL_INF)
		{
			subResult.idleTime += idleTime;

			//sort
			if (subResult.idleTime < minResult.idletime)
			{
				minResult.idletime = subResult.idleTime;
				minResult.result = subResult;
				minResult.syncPoint = syncPoint;
			}
			//
		}

		if (servers[i].crossed)
		{
			break;
		}

	}

	//shift sync points and insert best syncPoint
	for (size_t i = 0; i < minResult.result.syncPoints.size(); i++)
	{
		minResult.result.syncPoints[i] += minResult.syncPoint;
	}
	minResult.result.syncPoints.insert(minResult.result.syncPoints.begin(), minResult.syncPoint);

	minResult.result.finiteCaseTimes = finiteCaseIdleTimes;
	minResult.result.casesExplored = casesExplored;

	return minResult.result;
}

void OneExtraIdleReducer::Reduce(Scenario scenario)
{
	running = true;
	cancelled = false;
	finished = false;

	vector<ServerRecord> servers;
	for (size_t i = 0; i < scenario.jobs.serverCount(); i++)
	{
		servers.push_back(ServerRecord());
		servers[i].jobs = vector<ValType>();
		servers[i].crossed = false;
	}

	result = ReduceAdjust(servers);

	//sort(result.finiteCaseTimes.begin(), result.finiteCaseTimes.end());

	running = false;
	finished = true;
}
void OneExtraIdleReducer::OnGUI()
{
}
