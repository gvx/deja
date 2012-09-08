#ifndef LIB_DEF
#define LIB_DEF

#include <stdio.h>
#include <math.h>
#include <dlfcn.h>

#include "scope.h"
#include "func.h"
#include "opcodes.h"
#include "module.h"
#include "idents.h"

#include "strlib.h"

typedef Error (*CFuncP)(Stack*, Stack*);

typedef struct
{
	char *name;
	CFuncP cfunc;
} CFunc;

V new_cfunc(CFuncP);
V v_true;
V v_false;
void open_lib(CFunc[], HashMap*);
void open_std_lib(HashMap*);

#define require(x) if (stack_size(S) < (x)) return StackEmpty;

Error return_(Stack*, Stack*);

#endif
