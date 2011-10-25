#include <stdlib.h>

#include "gc.h"
#include "value.h"
#include "scope.h"
#include "func.h"
#include "types.h"
#include "file.h"

V new_scope(V parent)
{
	V sc = new_value(T_SCOPE);
	Scope* pscope = toScope(parent);
	Scope* scope = malloc(sizeof(Scope));
	scope->is_func_scope = false;
	scope->parent = add_ref(parent);
	scope->func = pscope->func == NULL ? NULL : add_ref(pscope->func);
	scope->file = pscope->file == NULL ? NULL : add_ref(pscope->file);
	scope->pc = pscope->pc;
	sc->data.object = scope;
	hashmap_from_scope(sc, 16);
	return sc;
}

V new_function_scope(V function)
{
	V sc = new_value(T_SCOPE);
	Scope* scope = malloc(sizeof(Scope));
	scope->is_func_scope = true;
	scope->parent = add_ref(toFunc(function)->defscope);
	scope->func = add_ref(function);
	scope->file = add_ref(toScope(scope->parent)->file);
	scope->pc = toFunc(function)->start;
	sc->data.object = scope;
	hashmap_from_scope(sc, 32);
	return sc;
}

V new_file_scope(V file)
{
	V sc = new_value(T_SCOPE);
	Scope* scope = malloc(sizeof(Scope));
	scope->is_func_scope = true;
	scope->parent = add_ref(toFile(file)->global);
	scope->func = NULL;
	scope->file = add_ref(file);
	scope->pc = toFile(file)->code - 1;
	sc->data.object = scope;
	hashmap_from_scope(sc, 64);
	return sc;
}

V new_global_scope(void)
{
	V sc = new_value(T_SCOPE);
	Scope* scope = malloc(sizeof(Scope));
	scope->is_func_scope = false;
	scope->parent = NULL;
	scope->func = NULL;
	scope->file = NULL;
	scope->pc = NULL;
	sc->data.object = scope;
	hashmap_from_scope(sc, 128);
	return sc;
}
