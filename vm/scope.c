#include <stdlib.h>

#include "scope.h"
#include "func.h"
#include "file.h"

V create_scope()
{
	if (MAXSCOPE < MAXCACHE)
	{
		ValueScope *sc = &SCOPECACHE[MAXSCOPE];
		sc->v.buffered = false;
		sc->v.type = T_SCOPE;
		sc->v.refs = 1;
		sc->v.color = Black;
		sc->sc.index = ++MAXSCOPE;
		return (V)sc;
	}
	else
	{
		V val = make_new_value(T_SCOPE, false, sizeof(Scope));
		toScope(val)->index = 0;
		return val;
	}
}

V new_scope(V parent)
{
	V sc = create_scope();
	Scope* pscope = toScope(parent);
	Scope* scope = toScope(sc);
	scope->is_func_scope = false;
	scope->is_error_handler = false;
	scope->parent = add_ref(parent);
	scope->func = pscope->func == NULL ? NULL : add_ref(pscope->func);
	scope->file = pscope->file == NULL ? NULL : add_ref(pscope->file);
	scope->callname = pscope->callname == NULL ? NULL : add_ref(pscope->callname);
	scope->pc = pscope->pc;
	scope->linenr = pscope->linenr;
	hashmap_from_scope(sc, 16);
	return sc;
}

V new_function_scope(V function, V callname)
{
	V sc = create_scope();
	Scope* scope = toScope(sc);
	scope->is_func_scope = true;
	scope->is_error_handler = false;
	scope->parent = add_ref(toFunc(function)->defscope);
	scope->func = add_ref(function);
	scope->file = add_ref(toScope(scope->parent)->file);
	scope->callname = callname == NULL ? NULL : add_ref(callname);
	scope->pc = toFunc(function)->start;
	hashmap_from_scope(sc, 32);
	return sc;
}

V new_file_scope(V file)
{
	V sc = create_scope();
	Scope* scope = toScope(sc);
	scope->is_func_scope = true;
	scope->is_error_handler = false;
	scope->parent = add_ref(toFile(file)->global);
	scope->func = NULL;
	scope->file = add_ref(file);
	scope->callname = NULL;
	scope->pc = toFile(file)->code - 1;
	hashmap_from_scope(sc, 64);
	return sc;
}

V new_global_scope(void)
{
	V sc = create_scope();
	Scope* scope = toScope(sc);
	scope->is_func_scope = false;
	scope->parent = NULL;
	scope->func = NULL;
	scope->file = NULL;
	scope->callname = NULL;
	scope->pc = NULL;
	hashmap_from_scope(sc, 256);
	return sc;
}
