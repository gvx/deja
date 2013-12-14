#include "lib.h"
#include "utf8.h"
#include "persist.h"
#include "mersenne.h"
#include "blob.h"
#include "eva.h"

#include <time.h>
#include <sys/time.h>

Error get(Stack* S, Stack* scope_arr)
{
	require(1);
	V key = popS();
	if (getType(key) != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}
	Scope *sc = toScope(get_head(scope_arr));
	V v = get_hashmap(toHashMap(sc->env), key);
	while (v == NULL)
	{
		if (sc->parent == NULL)
		{
			clear_ref(key);
			return NameError;
		}
		sc = toScope(sc->parent);
		v = get_hashmap(toHashMap(sc->env), key);
	}
	pushS(add_ref(v));
	clear_ref(key);
	return Nothing;
}

Error getglobal(Stack* S, Stack* scope_arr)
{
	require(1);
	V key = popS();
	if (getType(key) != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}
	V r = get_hashmap(toHashMap(toScope(toFile(toScope(get_head(scope_arr))->file)->global)->env), key);
	clear_ref(key);
	if (r == NULL)
	{
		return NameError;
	}
	pushS(add_ref(r));
	return Nothing;
}

Error set(Stack* S, Stack* scope_arr)
{
	require(2);
	V key = popS();
	if (getType(key) != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}
	V v = popS();
	Scope *sc = toScope(get_head(scope_arr));
	while (!change_hashmap(toHashMap(sc->env), key, v))
	{
		if (sc->parent == NULL)
		{
			//set in the global environment
			set_hashmap(toHashMap(sc->env), key, v);
			break;
		}
		else
		{
			sc = toScope(sc->parent);
		}
	}
	clear_ref(v);
	clear_ref(key);
	return Nothing;
}

Error setglobal(Stack* S, Stack* scope_arr)
{
	require(2);
	V key = popS();
	if (getType(key) != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}
	V v = popS();
	set_hashmap(toHashMap(toScope(toFile(toScope(get_head(scope_arr))->file)->global)->env), key, v);
	clear_ref(v);
	clear_ref(key);
	return Nothing;
}

Error setlocal(Stack* S, Stack* scope_arr)
{
	require(2);
	V key = popS();
	if (getType(key) != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}
	V v = popS();
	set_hashmap(toHashMap(toScope(get_head(scope_arr))->env), key, v);
	clear_ref(v);
	clear_ref(key);
	return Nothing;
}

Error add(Stack* S, Stack* scope_arr)
{
	require(2);
	V r;
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		r = double_to_value(toNumber(v1) + toNumber(v2));
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		r = new_frac(
			getNumer(v1) * getDenom(v2) +
			getNumer(v2) * getDenom(v1),
			getNumer(v1) * getDenom(v2)
		);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		r = new_frac(
			getNumer(v1) + toNumber(v2) * getDenom(v1),
			getDenom(v1)
		);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		r = new_frac(
			toNumber(v1) * getDenom(v2) + getNumer(v2),
			getDenom(v2)
		);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(r);
	return Nothing;
}

Error sub(Stack* S, Stack* scope_arr)
{
	require(2);
	V r;
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		r = double_to_value(toNumber(v1) - toNumber(v2));
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		r = new_frac(
			getNumer(v1) * getDenom(v2) -
			getNumer(v2) * getDenom(v1),
			getDenom(v1) * getDenom(v2)
		);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		r = new_frac(
			getNumer(v1) - toNumber(v2) * getDenom(v1),
			getDenom(v1)
		);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		r = new_frac(
			toNumber(v1) * getDenom(v2) - getNumer(v2),
			getDenom(v2)
		);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(r);
	return Nothing;
}

Error mul(Stack* S, Stack* scope_arr)
{
	require(2);
	V r;
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		r = double_to_value(toNumber(v1) * toNumber(v2));
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		r = new_frac(
			getNumer(v1) * getNumer(v2),
			getDenom(v1) * getDenom(v2)
		);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		r = new_frac(
			getNumer(v1) * toNumber(v2),
			getDenom(v1)
		);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		r = new_frac(
			toNumber(v1) * getNumer(v2),
			getDenom(v2)
		);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(r);
	return Nothing;
}

Error div_(Stack* S, Stack* scope_arr)
{
	require(2);
	V r;
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		if (toNumber(v2) == 0.0)
		{
			clear_ref(v1);
			clear_ref(v2);
			set_error_msg("division by zero");
			return ValueError;
		}
		r = double_to_value(toNumber(v1) / toNumber(v2));
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		r = new_frac(
			getNumer(v1) * getDenom(v2),
			getDenom(v1) * getNumer(v2)
		);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		if (toNumber(v2) == 0.0)
		{
			clear_ref(v1);
			clear_ref(v2);
			set_error_msg("division by zero");
			return ValueError;
		}
		r = new_frac(
			getNumer(v1),
			getDenom(v1) * toNumber(v2)
		);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		r = new_frac(
			toNumber(v1) * getDenom(v2),
			getNumer(v2)
		);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(r);
	return Nothing;
}

Error mod_(Stack* S, Stack* scope_arr)
{
	require(2);
	V r;
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		if (toNumber(v2) == 0.0)
		{
			clear_ref(v1);
			clear_ref(v2);
			set_error_msg("division by zero");
			return ValueError;
		}
		r = double_to_value(fmod(toNumber(v1), toNumber(v2)));
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		r = new_frac(
			(getNumer(v1) * getDenom(v2)) %
			(getNumer(v2) * getDenom(v1)),
			getDenom(v1) * getDenom(v2)
		);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		if (toNumber(v2) == 0.0)
		{
			clear_ref(v1);
			clear_ref(v2);
			set_error_msg("division by zero");
			return ValueError;
		}
		r = new_frac(
			getNumer(v1) %
			((long int)toNumber(v2) * getDenom(v1)),
			getDenom(v1)
		);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		r = new_frac(
			((long int)toNumber(v1) * getDenom(v2)) %
			getNumer(v2),
			getDenom(v2)
		);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(r);
	return Nothing;
}

const char* gettype(V r)
{
	switch (getType(r))
	{
		case T_IDENT:
			return "ident";
		case T_STR:
			return "str";
		case T_NUM:
			return "num";
		case T_LIST:
			return "list";
		case T_DICT:
			return "dict";
		case T_PAIR:
			return "pair";
		case T_FRAC:
			return "frac";
		case T_FUNC:
		case T_CFUNC:
		case T_SCOPE:
			return "func";
		case T_BLOB:
			return "blob";
		default:
			return "nil"; //not really true, but meh.
	}

}

Error type(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	V t = get_ident(gettype(v));
	pushS(t);
	clear_ref(v);
	return Nothing;
}

Error make_new_list(Stack* S, Stack* scope_arr)
{
	pushS(new_list());
	return Nothing;
}

Error make_new_dict(Stack* S, Stack* scope_arr)
{
	pushS(new_dict());
	return Nothing;
}

Error produce_list(Stack* S, Stack* scope_arr)
{
	V v = new_list();
	V p;
	while (stack_size(S) > 0)
	{
		p = popS();
		if (getType(p) == T_IDENT)
		{
			if (p == get_ident("]"))
			{
				clear_ref(p);
				pushS(v);
				return Nothing;
			}
		}
		push(toStack(v), p);
	}
	return StackEmpty;
}

Error produce_dict(Stack* S, Stack* scope_arr)
{
	V v = new_dict();
	V key;
	V val;
	while (stack_size(S) > 0)
	{
		key = popS();
		if (getType(key) == T_IDENT)
		{
			if (key == get_ident("}"))
			{
				clear_ref(key);
				pushS(v);
				return Nothing;
			}
		}
		val = popS();
		if (val == NULL)
		{
			return StackEmpty;
		}
		set_hashmap(toHashMap(v), key, val);
	}
	return StackEmpty;
}


Error if_(Stack* S, Stack* scope_arr)
{
	require(3);
	V v0 = popS();
	V v1 = popS();
	V v2 = popS();
	if (truthy(v0))
	{
		clear_ref(v2);
		pushS(v1);
	}
	else
	{
		clear_ref(v1);
		pushS(v2);
	}
	clear_ref(v0);
	return Nothing;
}

Error return_(Stack* S, Stack* scope_arr)
{
	V v = NULL;
	V file = toScope(get_head(scope_arr))->file;
	do
	{
		clear_base_ref(v);
		v = pop(scope_arr);
		if (v == NULL)
		{
			return Exit;
		}
	}
	while (!toScope(v)->is_func_scope && toScope(v)->file == file);
	clear_base_ref(v);
	if (stack_size(scope_arr) == 0)
	{
		return Exit;
	}
	return Nothing;
}

Error exit_(Stack* S, Stack* scope_arr)
{
	return Exit;
}

Error lt(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	__int128_t a, b;
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		clear_ref(v1);
		clear_ref(v2);
		pushS(add_ref(toNumber(v1) < toNumber(v2) ? v_true : v_false));
		return Nothing;
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		a = (__int128_t)getNumer(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		a = (__int128_t)getNumer(v1);
		b = toNumber(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		a = toNumber(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(add_ref(a < b ? v_true : v_false));
	return Nothing;
}

Error gt(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	__int128_t a, b;
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		clear_ref(v1);
		clear_ref(v2);
		pushS(add_ref(toNumber(v1) > toNumber(v2) ? v_true : v_false));
		return Nothing;
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		a = (__int128_t)getNumer(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		a = (__int128_t)getNumer(v1);
		b = toNumber(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		a = toNumber(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(add_ref(a > b ? v_true : v_false));
	return Nothing;
}

Error le(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	__int128_t a, b;
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		clear_ref(v1);
		clear_ref(v2);
		pushS(add_ref(toNumber(v1) <= toNumber(v2) ? v_true : v_false));
		return Nothing;
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		a = (__int128_t)getNumer(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		a = (__int128_t)getNumer(v1);
		b = toNumber(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		a = toNumber(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(add_ref(a <= b ? v_true : v_false));
	return Nothing;
}

Error ge(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	__int128_t a, b;
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		clear_ref(v1);
		clear_ref(v2);
		pushS(add_ref(toNumber(v1) >= toNumber(v2) ? v_true : v_false));
		return Nothing;
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		a = (__int128_t)getNumer(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		a = (__int128_t)getNumer(v1);
		b = toNumber(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		a = toNumber(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2);
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	clear_ref(v1);
	clear_ref(v2);
	pushS(add_ref(a >= b ? v_true : v_false));
	return Nothing;
}

Error eq(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	pushS(add_ref(equal(v1, v2) ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error ne(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	pushS(add_ref(equal(v1, v2) ? v_false : v_true));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error not(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	pushS(add_ref(truthy(v) ? v_false : v_true));
	clear_ref(v);
	return Nothing;
}

Error and(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	pushS(add_ref(truthy(v1) && truthy(v2) ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error or(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	pushS(add_ref(truthy(v1) || truthy(v2) ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error xor(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	pushS(add_ref(truthy(v1) != truthy(v2) ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error range(Stack* S, Stack* scope_arr)
{
	require(1);
	V v1;
	V v2;
	V v = popS();
	if (getType(v) == T_PAIR)
	{
		v1 = add_ref(toFirst(v));
		v2 = add_ref(toSecond(v));
		clear_ref(v);
	}
	else
	{
		require(1);
		v1 = v;
		v2 = popS();
	}
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		if (toNumber(v1) > toNumber(v2))
		{
			pushS(add_ref(v_false));
			clear_ref(v1);
			clear_ref(v2);
		}
		else
		{
			pushS(v1);
			pushS(new_pair(double_to_value(toNumber(v1) + 1.0), v2));
			pushS(cFuncToV(range));
		}
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error in(Stack* S, Stack* scope_arr)
{
	require(1);
	V list = popS();
	if (getType(list) != T_LIST)
	{
		clear_ref(list);
		return TypeError;
	}
	if (stack_size(toStack(list)) > 0)
	{
		V item = pop(toStack(list));
		pushS(item);
		pushS(list);
		pushS(cFuncToV(in));
	}
	else
	{
		pushS(add_ref(v_false));
		clear_ref(list);
	}
	return Nothing;
}

Error reversed(Stack* S, Stack* scope_arr)
{
	require(1);
	V list = popS();
	if (getType(list) != T_LIST)
	{
		clear_ref(list);
		return TypeError;
	}
	V rev = new_list();
	while (stack_size(toStack(list)) > 0)
	{
		push(toStack(rev), pop(toStack(list)));
	}
	pushS(rev);
	return Nothing;
}

Error swap(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	pushS(v1);
	pushS(v2);
	return Nothing;
}

Error push_to(Stack* S, Stack* scope_arr)
{
	require(2);
	V list = popS();
	if (getType(list) != T_LIST)
	{
		clear_ref(list);
		return TypeError;
	}
	V val = popS();
	push(toStack(list), val);
	clear_ref(list);
	return Nothing;
}

Error push_through(Stack* S, Stack* scope_arr)
{
	require(2);
	V list = popS();
	if (getType(list) != T_LIST)
	{
		clear_ref(list);
		return TypeError;
	}
	V val = popS();
	push(toStack(list), val);
	pushS(list);
	return Nothing;
}

Error pop_from(Stack* S, Stack* scope_arr)
{
	require(1);
	V list = popS();
	if (getType(list) != T_LIST)
	{
		clear_ref(list);
		return TypeError;
	}
	if (stack_size(toStack(list)) < 1)
	{
		clear_ref(list);
		return ValueError;
	}
	V val = pop(toStack(list));
	pushS(val);
	clear_ref(list);
	return Nothing;
}

Error tail_call(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = NULL;
	V file = toScope(get_head(scope_arr))->file;
	do
	{
		clear_base_ref(v);
		v = pop(scope_arr);
	}
	while (v && !toScope(v)->is_func_scope && toScope(v)->file == file);
	v = popS();
	if (getType(v) == T_FUNC)
	{
		push(scope_arr, add_rooted(new_function_scope(v, NULL)));
		clear_ref(v);
	}
	else if (getType(v) == T_CFUNC)
	{
		Error e = toCFunc(v)(S, scope_arr);
		clear_ref(v);
		return e;
	}
	else if (getType(v) == T_SCOPE)
	{
		call_scope(scope_arr, v);
	}
	else
	{
		pushS(v);
	}
	return Nothing;
}

Error self_tail(Stack* S, Stack* scope_arr)
{
	V v = NULL;
	V file = toScope(get_head(scope_arr))->file;
	do
	{
		clear_base_ref(v);
		v = pop(scope_arr);
		if (v == NULL)
		{
			return Exit;
		}
	}
	while (!toScope(v)->is_func_scope && toScope(v)->file == file);
	push(scope_arr, v);
	Scope *sc = toScope(v);
	sc->pc = toFunc(sc->func)->start;
	return Nothing;
}

Error copy(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	V new;
	switch (getType(v))
	{
		case T_LIST:
			new = new_list();
			copy_stack(toStack(v), toStack(new));
			break;
		case T_DICT:
			new = new_sized_dict(toHashMap(v)->size);
			copy_hashmap(toHashMap(v), toHashMap(new));
			break;
		case T_BLOB:
			new = clone_blob(v);
			break;
		default:
			new = add_ref(v);
			break;
	}
	pushS(new);
	clear_ref(v);
	return Nothing;
}

Error call(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) == T_FUNC)
	{
		push(scope_arr, add_rooted(new_function_scope(v, NULL)));
		clear_ref(v);
	}
	else if (getType(v) == T_CFUNC)
	{
		Error e = toCFunc(v)(S, scope_arr);
		clear_ref(v);
		return e;
	}
	else if (getType(v) == T_SCOPE)
	{
		call_scope(scope_arr, v);
	}
	else
	{
		pushS(v);
	}
	return Nothing;
}

Error dup(Stack* S, Stack* scope_arr)
{
	require(1);
	pushS(add_ref(get_head(S)));
	return Nothing;
}

Error drop(Stack* S, Stack* scope_arr)
{
	require(1);
	clear_ref(popS());
	return Nothing;
}

Error over(Stack* S, Stack* scope_arr)
{
	require(2);
	pushS(add_ref(S->nodes[S->used - 2]));
	return Nothing;
}

Error rotate(Stack* S, Stack* scope_arr)
{
	require(3);
	V v = S->nodes[S->used-3];
	S->nodes[S->used-3] = S->nodes[S->used-2];
	S->nodes[S->used-2] = S->nodes[S->used-1];
	S->nodes[S->used-1] = v;
	return Nothing;
}

Error error(Stack* S, Stack* scope_arr)
{
	return UserError;
}

Error raise_(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_IDENT)
	{
		clear_ref(v);
		return TypeError;
	}
	return ident_to_error(v);
}

Error reraise_(Stack* S, Stack* scope_arr)
{
	extern bool reraise;
	require(1);
	V v = popS();
	if (getType(v) != T_IDENT)
	{
		clear_ref(v);
		return TypeError;
	}
	reraise = true;
	return ident_to_error(v);
}

Error raise_msg(Stack* S, Stack* scope_arr)
{
	require(2);
	V err = popS();
	V msg = popS();
	if (getType(err) != T_IDENT || getType(msg) != T_STR)
	{
		clear_ref(err);
		clear_ref(msg);
		return TypeError;
	}
	set_error_msg(toNewString(msg)->text);
	clear_ref(msg);
	return ident_to_error(err);
}

Error len(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	switch (getType(v))
	{
		case T_STR:
			pushS(int_to_value(string_length(toNewString(v))));
			break;
		case T_LIST:
			pushS(int_to_value(stack_size(toStack(v))));
			break;
		case T_BLOB:
			pushS(int_to_value(toBlob(v)->size));
			break;
		default:
			clear_ref(v);
			return TypeError;
	}
	clear_ref(v);
	return Nothing;
}

Error yield(Stack* S, Stack* scope_arr)
{
	V cont = get_head(scope_arr);
	pushS(add_ref(cont));
	return return_(S, scope_arr);
}

void open_lib(CFunc[], HashMap*);

Error loadlib(Stack* S, Stack* scope_arr)
{
	require(1);
	V name = popS();
	if (getType(name) != T_STR)
	{
		return TypeError;
	}
	void* lib_handle = dlopen(toNewString(name)->text, RTLD_NOW);
	if (lib_handle == NULL)
	{
		fprintf(stderr, "%s\n", dlerror());
		return UnknownError;
	}
	CFunc *lib = dlsym(lib_handle, "deja_vu");
	if (lib == NULL)
	{
		fprintf(stderr, "%s\n", dlerror());
		return UnknownError;
	}
	open_lib(lib, toHashMap(toScope(toFile(toScope(get_head(scope_arr))->file)->global)->env));
	CFuncP initfunc = dlsym(lib_handle, "deja_vu_init");
	if (initfunc != NULL)
	{
		return initfunc(S, scope_arr);
	}
	return Nothing;
}

Error has(Stack* S, Stack* scope_arr)
{
	require(2);
	V container = popS();
	V key = popS();
	if (getType(container) != T_DICT)
	{
		clear_ref(container);
		clear_ref(key);
		return TypeError;
	}
	V v = get_hashmap(toHashMap(container), key);
	pushS(add_ref(v == NULL ? v_false : v_true));
	clear_ref(container);
	clear_ref(key);
	return Nothing;
}

Error get_from(Stack* S, Stack* scope_arr)
{
	require(2);
	V container = popS();
	V key = popS();
	V v;
	Error e = Nothing;
	char t = getType(container);
	if (t == T_DICT)
	{
		v = get_dict(toHashMap(container), key);
	}
	else if (t == T_LIST)
	{
		if (getType(key) != T_NUM)
		{
			e = TypeError;
			goto cleanup;
			return TypeError;
		}
		int index = (int)toNumber(key);
		Stack *s = toStack(container);
		if (index < 0)
			index = s->used + index;
		if (index < 0 || index >= s->used)
		{
			v = NULL;
		}
		else
		{
			v = s->nodes[index];
		}
	}
	else if (t == T_BLOB)
	{
		if (getType(key) != T_NUM)
		{
			e = TypeError;
			goto cleanup;
		}
		int index = (int)toNumber(key);
		int byte = getbyte_blob(container, index);
		if (byte < 0)
		{
			set_error_msg("Index out of range");
			e = ValueError;
			goto cleanup;
		}
		v = int_to_value(byte);
	}
	else
	{
		e = TypeError;
		goto cleanup;
	}
	if (v == NULL)
	{
		e = ValueError;
		goto cleanup;
	}
	pushS(add_ref(v));
	cleanup:
	clear_ref(container);
	clear_ref(key);
	return e;
}

Error set_to(Stack* S, Stack* scope_arr)
{
	require(3);
	V container = popS();
	V key = popS();
	V value = popS();
	Error e = Nothing;
	char t = getType(container);
	if (t == T_DICT)
	{
		set_hashmap(toHashMap(container), key, value);
	}
	else if (t == T_LIST)
	{
		if (getType(key) != T_NUM)
		{
			e = TypeError;
			goto cleanup;
		}
		int index = (int)toNumber(key);
		Stack *s = toStack(container);
		if (index < 0)
			index = s->used + index;
		if (index < 0 || index >= s->used)
		{
			e = ValueError;
			goto cleanup;
		}
		else
		{
			s->nodes[index] = add_ref(value);
		}
	}
	else if (t == T_BLOB)
	{
		if (getType(key) != T_NUM || getType(value) != T_NUM)
		{
			e = TypeError;
			goto cleanup;
		}
		int num = toNumber(value);
		if (num < 0 || num > 255)
		{
			set_error_msg("Value not in range [0,255]");
			e = ValueError;
			goto cleanup;
		}
		int byte = setbyte_blob(container, toNumber(key), num);
		if (byte < 0)
		{
			set_error_msg("Index out of range");
			e = ValueError;
			goto cleanup;
		}
	}
	else
	{
		e = TypeError;
		goto cleanup;
	}
	cleanup:
	clear_ref(key);
	clear_ref(value);
	clear_ref(container);
	return e;
}

Error delete_from(Stack* S, Stack* scope_arr)
{
	require(2);
	V container = popS();
	V key = popS();
	if (getType(container) != T_DICT)
	{
		clear_ref(key);
		clear_ref(container);
		return TypeError;
	}
	delete_hashmap(toHashMap(container), key);
	clear_ref(key);
	clear_ref(container);
	return Nothing;
}

Error rep(Stack* S, Stack* scope_arr)
{
	require(2);
	V num = popS();
	if (getType(num) != T_NUM)
	{
		clear_ref(num);
		return TypeError;
	}
	V val = popS();
	int i;
	for (i = toNumber(num); i > 0; i--)
	{
		pushS(add_ref(val));
	}
	clear_ref(val);
	clear_ref(num);
	return Nothing;
}

Error to_num(Stack *S, Stack *scope_arr)
{
	char *end;
	double r;
	require(1);
	V v = popS();
	int type = getType(v);
	if (type == T_NUM)
	{
		pushS(v);
		return Nothing;
	}
	else if (type == T_FRAC)
	{
		pushS(double_to_value((double)(toNumerator(v))/(double)(toDenominator(v))));
		clear_ref(v);
		return Nothing;
	}
	else if (type != T_STR)
	{
		clear_ref(v);
		return TypeError;
	}
	r = strtod(toNewString(v)->text, &end);
	clear_ref(v);
	if (end[0] != '\0')
	{
		return ValueError;
	}
	pushS(double_to_value(r));
	return Nothing;
}

char int_str_buffer[32];
Error to_str(Stack *S, Stack *scope_arr)
{
	V v = popS();
	int type = getType(v);
	if (type == T_STR)
	{
		pushS(v);
		return Nothing;
	}
	else if (type != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	if (!toNumber(v))
	{
		pushS(str_to_string(1, "0"));
		clear_ref(v);
		return Nothing;
	}
	char *buff = int_str_buffer;
	sprintf(buff, "%.15g", toNumber(v));
	pushS(a_to_string(buff));
	clear_ref(v);
	return Nothing;
}

Error pass(Stack *S, Stack *scope_arr)
{
	return Nothing;
}

Error rand_(Stack* S, Stack* scope_arr)
{
	require(2);
	V v_min = popS();
	V v_max = popS();
	if (getType(v_min) != T_NUM || getType(v_max) != T_NUM)
	{
		clear_ref(v_min);
		clear_ref(v_max);
		return TypeError;
	}
	double min = toNumber(v_min);
	double max = toNumber(v_max);
	double ans = min + rand() / (RAND_MAX + 1.0) * (max - min);
	clear_ref(v_min);
	clear_ref(v_max);
	pushS(double_to_value(ans));
	return Nothing;
}

Error keys(Stack* S, Stack* scope_arr)
{
	require(1);
	V dict = popS();
	if (getType(dict) != T_DICT)
	{
		clear_ref(dict);
		return TypeError;
	}
	V list = new_list();
	HashMap *hm = toHashMap(dict);
	if (hm->map != NULL)
	{
		Stack *s = toStack(list);
		int i;
		Bucket *b;
		for (i = 0; i < hm->size; i++)
		{
			b = hm->map[i];
			while(b != NULL)
			{
				push(s, add_ref(b->key));
				b = b->next;
			}
		}
	}
	pushS(list);
	clear_ref(dict);
	return Nothing;
}

Error values(Stack* S, Stack* scope_arr)
{
	require(1);
	V dict = popS();
	if (getType(dict) != T_DICT)
	{
		clear_ref(dict);
		return TypeError;
	}
	V list = new_list();
	HashMap *hm = toHashMap(dict);
	if (hm->map != NULL)
	{
		Stack *s = toStack(list);
		int i;
		Bucket *b;
		for (i = 0; i < hm->size; i++)
		{
			b = hm->map[i];
			while(b != NULL)
			{
				push(s, add_ref(b->value));
				b = b->next;
			}
		}
	}
	pushS(list);
	clear_ref(dict);
	return Nothing;
}

Error pairs(Stack* S, Stack* scope_arr)
{
	V k, v;
	require(1);
	V dict = popS();
	if (getType(dict) != T_DICT)
	{
		clear_ref(dict);
		return TypeError;
	}
	V list = new_list();
	HashMap *hm = toHashMap(dict);
	if (hm->map != NULL)
	{
		Stack *s = toStack(list);
		int i;
		Bucket *b;
		for (i = 0; i < hm->size; i++)
		{
			b = hm->map[i];
			while(b != NULL)
			{
				k = b->key;
				v = b->value;
				if (!is_simple(k) || !is_simple(v))
				{
					clear_ref(dict);
					clear_ref(list);
					set_error_msg("keys and values should have simple types");
					return TypeError;
				}
				push(s, new_pair(add_ref(k), add_ref(v)));
				b = b->next;
			}
		}
	}
	pushS(list);
	clear_ref(dict);
	return Nothing;
}

Error exists_(Stack* S, Stack* scope_arr)
{
	require(1);
	V key = popS();
	if (getType(key) != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}

	Scope *sc = toScope(get_head(scope_arr));
	V v = get_hashmap(toHashMap(sc->env), key);
	while (v == NULL)
	{
		if (sc->parent == NULL)
		{
			clear_ref(key);
			pushS(add_ref(v_false));
			return Nothing;
		}
		sc = toScope(sc->parent);
		v = get_hashmap(toHashMap(sc->env), key);
	}
	pushS(add_ref(v_true));
	clear_ref(key);
	return Nothing;
}

Error floor_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(floor(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error ceil_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(ceil(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error round_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(round(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error produce_set(Stack *S, Stack *scope_arr)
{
	V v = new_dict();
	V val;
	while (stack_size(S) > 0)
	{
		val = popS();
		if (getType(val) == T_IDENT)
		{
			if (val == get_ident("}"))
			{
				clear_ref(val);
				pushS(v);
				return Nothing;
			}
		}
		set_hashmap(toHashMap(v), val, v_true);
	}
	return StackEmpty;
}

Error plus_one(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	if (isInt(v) && canBeInt(toInt(v) + 1))
	{
		pushS(intToV(toInt(v) + 1));
		return Nothing;
	}
	else if (getType(v) == T_NUM)
	{
		V r = double_to_value(toNumber(v) + 1.0);
		clear_ref(v);
		pushS(r);
		return Nothing;
	}
	else
	{
		clear_ref(v);
		return TypeError;
	}
}

Error minus_one(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	if (isInt(v) && canBeInt(toInt(v) - 1))
	{
		pushS(intToV(toInt(v) - 1));
		return Nothing;
	}
	else if (getType(v) == T_NUM)
	{
		V r = double_to_value(toNumber(v) - 1.0);
		clear_ref(v);
		pushS(r);
		return Nothing;
	}
	else
	{
		clear_ref(v);
		return TypeError;
	}
}

Error undef(Stack* S, Stack* scope_arr)
{
	return UnknownError;
}

Error choose(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_LIST)
	{
		clear_ref(v);
		return TypeError;
	}
	int n = stack_size(toStack(v)) - 1;
	n = (int)floor(rand() / (RAND_MAX + 1.0) * n);
	pushS(add_ref(toStack(v)->nodes[n]));
	clear_ref(v);
	return Nothing;
}

Error flatten(Stack* S, Stack* scope_arr)
{
	require(1);
	V list = popS();
	if (getType(list) != T_LIST)
	{
		clear_ref(list);
		return TypeError;
	}
	int n;
	Stack *s = toStack(list);
	for (n = 0; n < s->used; n++)
	{
		pushS(add_ref(s->nodes[n]));
	}
	return Nothing;
}

Error make_pair(Stack* S, Stack* scope_arr)
{
	require(2);
	V first = popS();
	V second = popS();
	if (!is_simple(first) || !is_simple(second))
	{
		clear_ref(first);
		clear_ref(second);
		return TypeError;
	}
	pushS(new_pair(first, second));
	return Nothing;
}

Error get_first(Stack* S, Stack* scope_arr)
{
	require(1);
	V pair = popS();
	if (getType(pair) != T_PAIR)
	{
		clear_ref(pair);
		return TypeError;
	}
	pushS(add_ref(toFirst(pair)));
	clear_ref(pair);
	return Nothing;
}

Error get_second(Stack* S, Stack* scope_arr)
{
	require(1);
	V pair = popS();
	if (getType(pair) != T_PAIR)
	{
		clear_ref(pair);
		return TypeError;
	}
	pushS(add_ref(toSecond(pair)));
	clear_ref(pair);
	return Nothing;
}

Error get_both(Stack* S, Stack* scope_arr)
{
	require(1);
	V pair = popS();
	if (getType(pair) != T_PAIR)
	{
		clear_ref(pair);
		return TypeError;
	}
	pushS(add_ref(toSecond(pair)));
	pushS(add_ref(toFirst(pair)));
	clear_ref(pair);
	return Nothing;
}

Error abs_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(fabs(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error sin_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(sin(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error cos_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(cos(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error tan_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(tan(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error asin_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(asin(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error acos_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(acos(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error atan_(Stack *S, Stack *scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	pushS(double_to_value(atan(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error make_frac(Stack* S, Stack* scope_arr)
{
	require(2);
	V n = popS();
	V d = popS();
	if (!isInt(n) || !isInt(d))
	{
		clear_ref(n);
		clear_ref(d);
		return TypeError;
	}
	if (toInt(d) == 0)
	{
		set_error_msg("division by zero");
		return ValueError;
	}
	pushS(new_frac(toInt(n), toInt(d)));
	return Nothing;
}

Error clear(Stack *S, Stack *scope_arr)
{
	while (stack_size(S))
	{
		clear_ref(popS());
	}
	return Nothing;
}

Error time_(Stack *S, Stack *scope_arr)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	pushS(new_frac(tv.tv_usec + tv.tv_sec * 1000000, 1000000));
	return Nothing;
}

Error persist_value(Stack *S, Stack *scope_arr)
{
	require(2);
	V location = popS();
	if (getType(location) != T_STR)
	{
		clear_ref(location);
		return TypeError;
	}
	if (!valid_persist_name(location))
	{
		clear_ref(location);
		set_error_msg("invalid name for persisted data file");
		return ValueError;
	}
	V pval = popS();
	char *path = make_persist_path(location);
	if (!persist(path, pval))
	{
		clear_ref(location);
		clear_ref(pval);
		free(path);
		return UnknownError;
	}
	clear_ref(location);
	clear_ref(pval);
	free(path);
	return Nothing;
}

Error persist_stack(Stack *S, Stack *scope_arr)
{
	require(1);
	V location = popS();
	if (getType(location) != T_STR)
	{
		clear_ref(location);
		return TypeError;
	}
	if (!valid_persist_name(location))
	{
		clear_ref(location);
		set_error_msg("invalid name for persisted data file");
		return ValueError;
	}
	char *path = make_persist_path(location);
	if (!persist_all(path, S))
	{
		clear_ref(location);
		free(path);
		return UnknownError;
	}
	clear_ref(location);
	free(path);
	return Nothing;
}

Error unpersist(Stack *S, Stack *scope_arr)
{
	require(1);
	V location = popS();
	if (getType(location) != T_STR)
	{
		clear_ref(location);
		return TypeError;
	}
	if (!valid_persist_name(location))
	{
		clear_ref(location);
		set_error_msg("invalid name for persisted data file");
		return ValueError;
	}
	char *path = make_persist_path(location);
	V file = load_file(a_to_string(path), toFile(toScope(get_head(scope_arr))->file)->global);
	free(path);
	if (file == NULL)
	{
		return IllegalFile;
	}
	if (getType(file) == T_FILE)
	{
		push(scope_arr, add_rooted(new_file_scope(file)));
	}
	clear_ref(file);
	return Nothing;
}

Error chance(Stack *S, Stack *scope_arr)
{
	require(1);
	V p = popS();
	long threshold;
	if (getType(p) == T_NUM)
	{
		threshold = toNumber(p) * RAND_MAX;
	}
	else if (getType(p) == T_FRAC)
	{
		threshold = toNumerator(p) * RAND_MAX / toDenominator(p);
	}
	else
	{
		clear_ref(p);
		return TypeError;
	}
	if (threshold < 0 || threshold > RAND_MAX)
	{
		clear_ref(p);
		set_error_msg("probability should be between 0 and 1");
		return ValueError;
	}

	pushS(add_ref(rand() < threshold ? v_true : v_false));
	clear_ref(p);
	return Nothing;
}

Error set_default(Stack *S, Stack *scope_arr)
{
	require(2);
	V p = popS();
	if (getType(p) == T_DICT)
	{
		dictDefault(toHashMap(p)) = popS();
		clear_ref(p);
		return Nothing;
	}
	else
	{
		clear_ref(p);
		return TypeError;
	}
}

Error opt_get(Stack *S, Stack *scope_arr)
{
	require(2);
	V dct = popS();
	V key = popS();
	if (getType(dct) != T_DICT)
	{
		clear_ref(dct);
		clear_ref(key);
		return TypeError;
	}
	V r = get_hashmap(toHashMap(dct), key);
	if (r)
	{
		pushS(add_ref(r));
	}
	pushS(add_ref(r ? v_true: v_false));
	clear_ref(dct);
	clear_ref(key);
	return Nothing;
}

extern bool vm_interrupt;
#define LE(a, b) (toNumber(a) <= toNumber(b))
void merge_sort(size_t n, size_t start, size_t end, V *key_arr, V *obj_arr, V *workspace)
{
	if (vm_interrupt)
		return;
	int len = end - start;
	if (len < 2)
		return;
	int i;
	for (i = start + 1; i < end; i++)
		if (!LE(key_arr[i - 1], key_arr[i]))
			goto unsorted;
	return;
	unsorted:
	if (len == 2)
	{
		V tmp = key_arr[start];
		key_arr[start] = key_arr[start + 1];
		key_arr[start + 1] = tmp;
		if (obj_arr)
		{
			tmp = obj_arr[start];
			obj_arr[start] = obj_arr[start + 1];
			obj_arr[start + 1] = tmp;
		}
		return;
	}

	int leftlen = len / 2;
	int rightlen = len - leftlen;
	int middle = start + leftlen;

	merge_sort(n, start, middle, key_arr, obj_arr, workspace);
	merge_sort(n, middle, end, key_arr, obj_arr, workspace);

	memcpy(workspace, key_arr + start, leftlen * sizeof(V));
	if (obj_arr)
	{
		memcpy(workspace + leftlen, obj_arr + start, leftlen * sizeof(V));
	}

	int lefti = 0, righti = 0;

	while (lefti < leftlen && righti < rightlen)
	{
		if (LE(workspace[lefti], key_arr[middle + righti]))
		{
			key_arr[start + lefti + righti] = workspace[lefti];
			if (obj_arr)
			{
				obj_arr[start + lefti + righti] = workspace[leftlen + lefti];
			}
			lefti++;
		}
		else
		{
			key_arr[start + lefti + righti] = key_arr[middle + righti];
			if (obj_arr)
			{
				obj_arr[start + lefti + righti] = obj_arr[middle + righti];
			}
			righti++;
		}
	}
	if (lefti < leftlen)
	{
		memcpy(key_arr + start + lefti + righti, workspace + lefti, (leftlen - lefti) * sizeof(V));
		if (obj_arr)
		{
			memcpy(obj_arr + start + lefti + righti, workspace + leftlen + lefti, (leftlen - lefti) * sizeof(V));
		}
	}
	// if (righti < rightlen) : everything is already in place
}

Error sort_list(Stack *S, Stack *scope_arr)
{
	require(2);
	V keys = popS();
	V list = popS();
	V *key_arr;
	V *obj_arr;
	size_t n;
	if (getType(list) != T_LIST)
	{
		clear_ref(keys);
		clear_ref(list);
		return TypeError;
	}
	if (keys == v_false)
	{
		key_arr = toStack(list)->nodes;
		obj_arr = NULL;
		n = stack_size(toStack(list));
	}
	else if (getType(keys) != T_LIST)
	{
		clear_ref(keys);
		clear_ref(list);
		return TypeError;
	}
	else
	{
		key_arr = toStack(keys)->nodes;
		obj_arr = toStack(list)->nodes;
		n = stack_size(toStack(list));
		if (n != stack_size(toStack(keys)))
		{
			clear_ref(keys);
			clear_ref(list);
			return ValueError;
		}
	}

	V *workspace = malloc((obj_arr ? n : n/2) * sizeof(V));

	merge_sort(n, 0, n, key_arr, obj_arr, workspace);

	free(workspace);

	clear_ref(keys);
	pushS(list);
	return Nothing;
}

extern Stack *traceback;
Error print_traceback(Stack *S, Stack *scope_arr)
{
	handle_error(UserError, traceback);
	return Nothing;
}

Error list_globals(Stack *S, Stack *scope_arr)
{
	V list = new_list();
	HashMap *hm = toHashMap(toScope(toFile(toScope(get_head(scope_arr))->file)->global)->env);
	if (hm->map != NULL)
	{
		Stack *s = toStack(list);
		int i;
		Bucket *b;
		for (i = 0; i < hm->size; i++)
		{
			b = hm->map[i];
			while(b != NULL)
			{
				push(s, add_ref(b->key));
				b = b->next;
			}
		}
	}
	pushS(list);
	return Nothing;
}

static CFunc stdlib[] = {
	{"get", get},
	{"getglobal", getglobal},
	{"set", set},
	{"setglobal", setglobal},
	{"local", setlocal},
	{"+", add},
	{"-", sub},
	{"*", mul},
	{"/", div_},
	{"%", mod_},
	{"type", type},
	{"[]", make_new_list},
	{"[", produce_list},
	{"if", if_},
	{"return", return_},
	{"exit", exit_},
	{"<", lt},
	{">", gt},
	{"=", eq},
	{"<=", le},
	{">=", ge},
	{"/=", ne},
	{"not", not},
	{"and", and},
	{"or", or},
	{"xor", xor},
	{"range", range},
	{"in", in},
	{"reversed", reversed},
	{"swap", swap},
	{"push-to", push_to},
	{"push-through", push_through},
	{"pop-from", pop_from},
	{"tail-call", tail_call},
	{"recurse", self_tail},
	{"copy", copy},
	{"call", call},
	{"dup", dup},
	{"drop", drop},
	{"over", over},
	{"rot", rotate},
	{"error", error},
	{"raise", raise_},
	{"reraise", reraise_},
	{"Raise", raise_msg},
	{"len", len},
	{"yield", yield},
	{"loadlib", loadlib},
	{"{}", make_new_dict},
	{"{", produce_dict},
	{"has", has},
	{"get-from", get_from},
	{"set-to", set_to},
	{"delete-from", delete_from},
	{"rep", rep},
	{"to-num", to_num},
	{"to-str", to_str},
	{"pass", pass},
	{"rand", rand_},
	{"keys", keys},
	{"values", values},
	{"pairs", pairs},
	{"exists?", exists_},
	{"floor", floor_},
	{"ceil", ceil_},
	{"round", round_},
	{"set{", produce_set},
	{"++", plus_one},
	{"--", minus_one},
	{"undef", undef},
	{"choose", choose},
	{"flatten", flatten},
	{"&", make_pair},
	{"&<", get_first},
	{"&>", get_second},
	{"&<>", get_both},
	{"abs", abs_},
	{"sin", sin_},
	{"cos", cos_},
	{"tan", tan_},
	{"asin", asin_},
	{"acos", acos_},
	{"atan", atan_},
	{"//", make_frac},
	{"clear", clear},
	{"time", time_},
	{"persist", persist_value},
	{"persist-stack", persist_stack},
	{"unpersist", unpersist},
	{"chance", chance},
	{"set-default", set_default},
	{"random-int", random_int},
	{"opt-get", opt_get},
	{"(sort)", sort_list},
	{"print-traceback", print_traceback},
	{"(list-globals)", list_globals},
	//blob
	{"make-blob", make_blob},
	{"get-from-blob", getbyte_blob_},
	{"set-to-blob", setbyte_blob_},
	{"resize-blob", resize_blob_},
	{"clone-blob", clone_blob_},
	{"blit-blob", blit_blob_},
	//strlib
	{"concat(", concat},
	{"concat", concat_list},
	{"contains", contains},
	{"starts-with", starts_with},
	{"ends-with", ends_with},
	{"split", split},
	{"join", join},
	{"slice", slice},
	{"ord", ord},
	{"chr", chr},
	{"find", find},
	{"chars", chars},
	{"count", count},
	{"split-any", split_any},
	{"is-digit", is_digit},
	{NULL, NULL}
};

static char* autonyms[] = {"(", ")", "]", "}", NULL};

void open_lib(CFunc lib[], HashMap* hm)
{
	int i = 0;
	while (lib[i].name != NULL)
	{
		V v = cFuncToV(lib[i].cfunc);
		set_hashmap(hm, get_ident(lib[i].name), v);
		clear_ref(v);
		i++;
	}
}

V open_std_lib(HashMap* hm)
{
	open_lib(stdlib, hm);
	char** k;
	for (k = autonyms; *k; k++)
	{
		V j = get_ident(*k);
		set_hashmap(hm, j, j);
	}
	v_true = make_new_value(T_NUM, true, sizeof(double));
	toDouble(v_true) = 1.0;
	v_false = make_new_value(T_NUM, true, sizeof(double));
	toDouble(v_false) = 0.0;
	set_hashmap(hm, get_ident("true"), v_true);
	set_hashmap(hm, get_ident("false"), v_false);

	// Open EVA
	V v_eva = new_dict();
	extern CFunc eva[];
	open_lib(eva, toHashMap(v_eva));
	set_hashmap(hm, get_ident("eva"), v_eva);

	srand((unsigned int)time(NULL));
	struct timespec tm;
	clock_gettime(CLOCK_MONOTONIC, &tm);
	init_random(tm.tv_sec ^ (tm.tv_sec >> 32) ^ tm.tv_nsec ^ (tm.tv_nsec >> 32));

	return v_eva;
}
