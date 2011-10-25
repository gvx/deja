#ifndef SCOPE_DEF
#define SCOPE_DEF

#include "value.h"
#include "hashmap.h"

#include <stdint.h>

typedef struct Scope
{
	V file;
	V func;
	V parent;
	bool is_func_scope;
	uint32_t linenr;
	uint32_t* pc;
	struct HashMap hm;
} Scope;

V new_scope(V);
V new_function_scope(V);
V new_file_scope(V);
V new_global_scope(void);

#endif