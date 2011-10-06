#ifndef FUNC_DEF
#define FUNC_DEF

#include "value.h"

typedef struct Func
{
	V defscope;
	int* start;
} Func;

#endif