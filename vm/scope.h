#ifndef SCOPE_DEF
#define SCOPE_DEF

#include "value.h"
#include "hashmap.h"

#include <stdint.h>

typedef struct Scope
{
	V func;
	V parent;
	uint32_t* pc;
	struct HashMap hm;
} Scope;

V new_scope(V);
V new_function_scope(V);

#endif