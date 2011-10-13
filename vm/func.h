#ifndef FUNC_DEF
#define FUNC_DEF

#include "value.h"

#include <stdint.h>

typedef struct Func
{
	V defscope;
	uint32_t *start;
} Func;

#endif