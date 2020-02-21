#pragma once

#include <vector>

using namespace std;

class Jobs
{
public:
	Jobs();
	Jobs(vector<vector<ValType>> jobs);
	~Jobs();

	size_t serverCount();
	size_t jobCount(size_t server);

	ValType getJob(size_t server, size_t index);
	void setJob(size_t server, size_t index, ValType value);

	void split(Jobs &first, Jobs &remaining, size_t amount);
	void add(Jobs &range);
	ValType maxRunTime();

	void addServer();
	void addJob(size_t server, ValType length);
	void removeServer(size_t server);
	void removeJob(size_t server, size_t index);

	void jobsAfter(ValType syncPoint, Jobs & output, ValType & idleTime, bool useT);
	size_t MaxJobCount();

private:
	vector<vector<ValType>> jobs;
};

