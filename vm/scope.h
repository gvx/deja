#ifndef SCOPE_DEF
#define SCOPE_DEF

#include "value.h"
#include "hashmap.h"

typedef struct Scope
{
	V func;
	V parent;
	struct HashMap hm;
} Scope;

#endif