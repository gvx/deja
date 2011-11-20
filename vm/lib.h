#ifndef LIB_DEF
#define LIB_DEF

#include <stdio.h>
#include <math.h>
#include <dlfcn.h>

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
#include "opcodes.h"
#include "module.h"

#include "strlib.h"

typedef struct
{
	char *name;
	Error (*cfunc)(Header*, Stack*, Stack*);
} CFunc;

V new_cfunc(Error (*)(Header*, Stack*, Stack*));
V v_true;
V v_false;
void open_lib(CFunc[], HashMap*);
void open_std_lib(HashMap*);

#define require(x) if (stack_size(S) < (x)) return StackEmpty;

Error return_(Header*, Stack*, Stack*);

#endif