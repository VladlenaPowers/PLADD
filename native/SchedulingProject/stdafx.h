// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define FILEPATH_BUF_SIZE 256

#define MAX_JOBS 16

typedef int ValType;

#define VAL_ZERO 0
#define VAL_INF INT_MAX

#define VAL_DEF 10000

struct UserSettings
{
	bool includeRedundantSyncPoints;
};

// TODO: reference additional headers your program requires here
