#include <stdlib.h>

#include "gc.h"
#include "value.h"
#include "scope.h"
#include "func.h"
#include "types.h"

V new_scope(V parent)
{
	V sc = new_value(T_SCOPE);
	Scope* scope = malloc(sizeof(Scope));
	if (parent == NULL)
	{
		scope->parent = NULL;
		scope->func = NULL;
	}
	else
	{
		scope->parent = add_ref(parent);
		scope->func = add_ref(((Scope*)parent->data.object)->func);
	}
	sc->data.object = scope;
	hashmap_from_scope(sc, 16);
	return sc;
}

V new_function_scope(V function)
{
	V sc = new_value(T_SCOPE);
	Scope* scope = malloc(sizeof(Scope));
	scope->parent = add_ref(((Func*)function->data.object)->defscope);
	scope->func = add_ref(function);
	sc->data.object = scope;
	hashmap_from_scope(sc, 32);
	return sc;
}