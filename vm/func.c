#include "func.h"
#include "types.h"
#include "gc.h"

V new_func(V scope, uint32_t *pc)
{
	V v = make_new_value(T_FUNC, false, sizeof(Func));
	Func *f = toFunc(v);
	f->defscope = add_ref(scope);
	f->start = pc;
	return v;
}
