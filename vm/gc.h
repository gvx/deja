#ifndef GC_DEF
#define GC_DEF

#include "value.h"

V new_value(int);
V add_ref(V);
void clear_ref(V);
void collect_cycles(void);

#endif