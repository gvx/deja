#ifndef LIB_DEF
#define LIB_DEF

#include <stdio.h>
#include <math.h>

#include "scope.h"
#include "header.h"
#include "stack.h"
#include "error.h"
#include "hashmap.h"
#include "types.h"
#include "value.h"
#include "gc.h"
#include "file.h"
#include "func.h"

typedef struct
{
	char *name;
	Error (*cfunc)(Header*, Stack*, Stack*);
} CFunc;

void open_lib(HashMap*);

#endif