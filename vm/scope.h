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
	int index;
	bool is_func_scope;
	bool is_error_handler;
	uint32_t linenr;
	uint32_t* pc;
	struct HashMap hm;
} Scope;

typedef struct
{
	Value v;
	Scope sc;
} ValueScope;

#define MAXCACHE 1024
static ValueScope SCOPECACHE[MAXCACHE];
static int MAXSCOPE = 0;

V new_scope(V);
V new_function_scope(V);
V new_file_scope(V);
V new_global_scope(void);

#endif