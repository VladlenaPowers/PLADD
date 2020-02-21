#pragma once


#include "Jobs.h"

using namespace std;

class Scenario
{
public:
	Scenario();
	Scenario(Jobs jobs, vector<ValType> syncPoints, ValType t, bool useT);
	~Scenario();

	void SaveToFile(string filePath);
	static Scenario LoadFromFile(string filePath);

	bool useT;
	ValType t;
	Jobs jobs;
	vector<ValType> syncPoints;

	bool isDirty;
};

