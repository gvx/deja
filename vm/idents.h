#ifndef IDENTS_DEF
#define IDENTS_DEF

#include "types.h"
#include "gc.h"
#include <stdlib.h>

V lookup_ident(size_t, const char*);
int ident_count();
int ident_depth();

#endif
