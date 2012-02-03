#ifndef GC_DEF
#define GC_DEF

#include "value.h"

#define new_value(t) make_new_value(t, false, 0)

V make_new_value(int, bool, int);
V add_ref(V);
void clear_ref(V);
void collect_cycles(void);

#endif
