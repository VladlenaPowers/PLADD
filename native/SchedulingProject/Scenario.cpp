#include "stdafx.h"
#include "Scenario.h"

#include <Windows.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include "Min.h"

using namespace std;

Scenario::Scenario() : jobs(vector<vector<ValType>>()), isDirty(true)
{
	isDirty = true;
}
Scenario::Scenario(Jobs jobs, vector<ValType> syncPoints, ValType t, bool useT) : jobs(jobs), syncPoints(syncPoints), t(t), useT(useT), isDirty(true)
{
}

void Scenario::SaveToFile(string filePath)
{
	ofstream ofs(filePath.c_str(), ifstream::out);

	ofs << "v3" << endl;
	ofs << jobs.serverCount() << endl;

	for (size_t i = 0; i < jobs.serverCount(); i++)
	{
		ofs << jobs.jobCount(i) << endl;
		for (size_t j = 0; j < jobs.jobCount(i); j++)
		{
			ofs << jobs.getJob(i, j) << endl;
		}
	}

	ofs << syncPoints.size() << endl;
	for (size_t i = 0; i < syncPoints.size(); i++)
	{
		ofs << syncPoints[i] << endl;
	}

	ofs << t << endl;

	ofs << (size_t)useT << endl;
}

Scenario Scenario::LoadFromFile(string filePath)
{
	ifstream ifs(filePath.c_str(), ifstream::in);

	string line;

	getline(ifs, line);

	if (line.compare("v3") == 0)
	{
		getline(ifs, line);
		int serverCount = stoi(line);

		vector<vector<ValType>> jobs;
		for (size_t i = 0; i < serverCount; i++)
		{
			getline(ifs, line);
			int jobCount = stoi(line);

			jobs.push_back(vector<ValType>());
			for (size_t j = 0; j < jobCount; j++)
			{
				getline(ifs, line);
				jobs[i].push_back(stoi(line));
			}
		}

		getline(ifs, line);
		int sPointCount = stoi(line);

		vector<int> syncPoints;
		for (size_t i = 0; i < sPointCount; i++)
		{
			getline(ifs, line);
			syncPoints.push_back(stoi(line));
		}

		getline(ifs, line);
		ValType t = stoi(line);

		getline(ifs, line);
		bool useT = stoi(line);

		return Scenario(jobs, syncPoints, t, useT);
	}
	else if (line.compare("v2") == 0)
	{
		getline(ifs, line);
		int serverCount = stoi(line);

		vector<vector<ValType>> jobs;
		for (size_t i = 0; i < serverCount; i++)
		{
			getline(ifs, line);
			int jobCount = stoi(line);

			jobs.push_back(vector<ValType>());
			for (size_t j = 0; j < jobCount; j++)
			{
				getline(ifs, line);
				jobs[i].push_back(stoi(line));
			}
		}

		getline(ifs, line);
		int sPointCount = stoi(line);

		vector<int> syncPoints;
		for (size_t i = 0; i < sPointCount; i++)
		{
			getline(ifs, line);
			syncPoints.push_back(stoi(line));
		}

		getline(ifs, line);
		ValType t = stoi(line);

		return Scenario(jobs, syncPoints, t, false);
	}
	else
	{
		int serverCount = stoi(line);
		getline(ifs, line);
		int jobCount = stoi(line);

		vector<vector<ValType>> jobs;
		for (size_t i = 0; i < serverCount; i++)
		{
			jobs.push_back(vector<ValType>());
			for (size_t j = 0; j < jobCount; j++)
			{
				getline(ifs, line);
				jobs[i].push_back(stoi(line));
			}
		}

		getline(ifs, line);
		int sPointCount = stoi(line);

		vector<int> syncPoints;
		for (size_t i = 0; i < sPointCount; i++)
		{
			getline(ifs, line);
			syncPoints.push_back(stoi(line));
		}

		return Scenario(jobs, syncPoints, VAL_DEF * 10, false);
	}
}

Scenario::~Scenario()
{
}
