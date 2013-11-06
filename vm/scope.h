#ifndef SCOPE_DEF
#define SCOPE_DEF

#include "hashmap.h"

#include <stdint.h>

typedef struct Scope
{
	V file;
	V func;
	V parent;
	int index;
	uint32_t is_func_scope : 4;
	uint32_t is_error_handler : 4;
	uint32_t linenr : 24;
	uint32_t* pc;
	struct HashMap hm;
} Scope;

typedef struct
{
	Value v;
	Scope sc;
} ValueScope;

#define MAXCACHE 1024
ValueScope SCOPECACHE[MAXCACHE];
int MAXSCOPE;

V new_scope(V);
V new_function_scope(V);
V new_file_scope(V);
V new_global_scope(void);

#endif
