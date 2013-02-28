#include "lib.h"
#include "utf8.h"
#include "persist.h"

#include <time.h>
#include <sys/time.h>

void print_list_value(Stack*, int, int);

void print_value(V v, int depth)
{
	String* s;
	ITreeNode* i;
	switch (getType(v))
	{
		case T_IDENT:
			i = toIdent(v);
			printf(":%*s", i->length, i->data);
			break;
		case T_STR:
			s = toString(v);
			printf("\"%*s\"", s->length, toCharArr(s));
			break;
		case T_NUM:
			if (v == v_true)
			{
				fputs("true", stdout);
			}
			else if (v == v_false)
			{
				fputs("false", stdout);
			}
			else
			{
				printf("%.15g", toNumber(v));
			}
			break;
		case T_LIST:
			if (depth < 4)
			{
				fputs("[ ", stdout);
				print_list_value(toStack(v), toStack(v)->used - 1, depth);
				fputs("]", stdout);
			}
			else
			{
				fputs("[...]", stdout);
			}
			break;
		case T_DICT:
			if (depth < 4)
			{
				fputs("{", stdout);
				int i;
				HashMap *hm = toHashMap(v);
				if (hm->map != NULL)
				{
					for (i = 0; i < hm->size; i++)
					{
						Bucket *b = hm->map[i];
						while (b)
						{
							putchar(' ');
							print_value(b->key, depth + 1);
							putchar(' ');
							print_value(b->value, depth + 1);
							b = b->next;
						}
					}
				}
				fputs(" }", stdout);
			}
			else
			{
				fputs("{...}", stdout);
			}
			break;
		case T_PAIR:
			// note: pairs are not cyclic, so no need to increase the depth
			fputs("& ", stdout);
			print_value(toFirst(v), depth);
			fputs(" ", stdout);
			print_value(toSecond(v), depth);
			break;
		case T_FRAC:
			printf("%ld/%ld", toNumerator(v), toDenominator(v));
			break;
		case T_CFUNC:
			printf("<func:%p>", toCFunc(v));
			break;
		case T_FUNC:
			printf("<func:%p>", toFunc(v));
			break;
	};
}

void print_list_value(Stack *s, int n, int depth)
{
	if (n < 0)
		return;
	print_list_value(s, n - 1, depth);
	print_value(s->nodes[n], depth + 1);
	fputs(" ", stdout);
}

void print_list_value_rev(Stack *s, int n, int depth)
{
	if (n >= s->used)
		return;
	print_list_value_rev(s, n + 1, depth);
	print_value(s->nodes[n], depth + 1);
	fputs(" ", stdout);
}

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
	V v = get_hashmap(&sc->hm, key);
	while (v == NULL)
	{
		if (sc->parent == NULL)
		{
			clear_ref(key);
			return NameError;
		}
		sc = toScope(sc->parent);
		v = get_hashmap(&sc->hm, key);
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
	V r = get_hashmap(&toScope(toFile(toScope(get_head(scope_arr))->file)->global)->hm, key);
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
	while (!change_hashmap(&sc->hm, key, v))
	{
		if (sc->parent == NULL)
		{
			//set in the global environment
			set_hashmap(&sc->hm, key, v);
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
	set_hashmap(&toScope(toFile(toScope(get_head(scope_arr))->file)->global)->hm, key, v);
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
	set_hashmap(&toScope(get_head(scope_arr))->hm, key, v);
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
			error_msg = "division by zero";
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
			error_msg = "division by zero";
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
			error_msg = "division by zero";
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
			error_msg = "division by zero";
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
			return "func";
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

Error print(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	print_value(v, 0);
	clear_ref(v);
	return Nothing;
}

Error print_nl(Stack* S, Stack* scope_arr)
{
	require(1);
	print(S, scope_arr);
	putchar('\n');
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
		clear_ref(v);
		v = pop(scope_arr);
		if (v == NULL)
		{
			return Exit;
		}
	}
	while (!toScope(v)->is_func_scope && toScope(v)->file == file);
	clear_ref(v);
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
		a = (__int128_t)toNumber(v1);
		b = (__int128_t)toNumber(v2);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		a = (__int128_t)getNumer(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		a = (__int128_t)getNumer(v1);
		b = (__int128_t)toNumber(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		a = (__int128_t)toNumber(v1) * (__int128_t)getDenom(v2);
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
		a = (__int128_t)toNumber(v1);
		b = (__int128_t)toNumber(v2);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		a = (__int128_t)getNumer(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		a = (__int128_t)getNumer(v1);
		b = (__int128_t)toNumber(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		a = (__int128_t)toNumber(v1) * (__int128_t)getDenom(v2);
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
		a = (__int128_t)toNumber(v1);
		b = (__int128_t)toNumber(v2);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		a = (__int128_t)getNumer(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		a = (__int128_t)getNumer(v1);
		b = (__int128_t)toNumber(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		a = (__int128_t)toNumber(v1) * (__int128_t)getDenom(v2);
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
		a = (__int128_t)toNumber(v1);
		b = (__int128_t)toNumber(v2);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_FRAC)
	{
		a = (__int128_t)getNumer(v1) * (__int128_t)getDenom(v2);
		b = (__int128_t)getNumer(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_FRAC && getType(v2) == T_NUM)
	{
		a = (__int128_t)getNumer(v1);
		b = (__int128_t)toNumber(v2) * (__int128_t)getDenom(v1);
	}
	else if (getType(v1) == T_NUM && getType(v2) == T_FRAC)
	{
		a = (__int128_t)toNumber(v1) * (__int128_t)getDenom(v2);
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

V v_range;

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
			/* METHOD 1: look up
			   more computation
			   fails if the global "range" is overwritten
			*/
			//pushS(add_ref(get_hashmap(&toScope(toFile(toScope(scope_arr->head->data)->file)->global)->hm, get_ident("range"))));
			/* METHOD 2: create value
			   less computation
			   works even if the global "range" is overwritten
			*/
			//pushS(new_cfunc(range));
			/* METHOD 3: just use a global value
			*/
			pushS(add_ref(v_range));
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
		pushS(new_cfunc(in));
	}
	else
	{
		pushS(add_ref(v_false));
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

Error print_stack(Stack* S, Stack* scope_arr)
{
	fputs("[ ", stdout);
	print_list_value_rev(S, 0, 0);
	puts("]");
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
	if (getType(get_head(S)) != T_IDENT)
	{
		return TypeError;
	}
	Error e = get(S, scope_arr);
	if (e != Nothing)
	{
		return e;
	}
	V v = NULL;
	V file = toScope(get_head(scope_arr))->file;
	do
	{
		clear_ref(v);
		v = pop(scope_arr);
		if (v == NULL)
		{
			return Exit;
		}
	}
	while (!toScope(v)->is_func_scope && toScope(v)->file == file);
	v = popS();
	if (getType(v) == T_FUNC)
	{
		push(scope_arr, new_function_scope(v));
		clear_ref(v);
	}
	else if (getType(v) == T_CFUNC)
	{
		e = toCFunc(v)(S, scope_arr);
		clear_ref(v);
		return e;
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
		clear_ref(v);
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

Error print_depth(Stack* S, Stack* scope_arr)
{
	printf("(depth:%d)\n", stack_size(scope_arr));
	return Nothing;
}

Error input(Stack* S, Stack* scope_arr)
{
	char line[256];
	if (!fgets(line, 256, stdin))
	{
		pushS(add_ref(v_false));
		return Nothing;
	}
	if (line[strlen(line) - 1] == '\n')
	{
		line[strlen(line) - 1] = '\0'; //removes trailing newline
	}
	if (!valid_utf8(strlen(line), line))
	{
		return UnicodeError;
	}
	pushS(a_to_value(line));
	return Nothing;
}

Error copy(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	V new;
	if (getType(v) == T_LIST)
	{
		new = new_list();
		copy_stack(toStack(v), toStack(new));
	}
	else if (getType(v) == T_DICT)
	{
		HashMap *hm = toHashMap(v);
		new = new_sized_dict(hm->size);
		copy_hashmap(hm, toHashMap(new));
	}
	else
	{
		new = add_ref(v);
	}
	pushS(new);
	clear_ref(v);
	return Nothing;
}

Error use(Stack* S, Stack* scope_arr)
{
	require(1);
	V fname = popS();
	if (getType(fname) != T_IDENT)
	{
		return TypeError;
	}
	V file = load_file(find_file(fname), toFile(toScope(get_head(scope_arr))->file)->global);
	if (file == NULL)
	{
		return IllegalFile;
	}
	if (getType(file) == T_FILE)
	{
		push(scope_arr, new_file_scope(file));
	}
	clear_ref(file);
	return Nothing;
}

Error call(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = get_head(S);
	if (getType(v) == T_IDENT)
	{
		Error e = get(S, scope_arr);
		if (e != Nothing)
		{
			return e;
		}
	}
	v = popS();
	if (getType(v) == T_FUNC)
	{
		push(scope_arr, new_function_scope(v));
		clear_ref(v);
	}
	else if (getType(v) == T_CFUNC)
	{
		Error e = toCFunc(v)(S, scope_arr);
		clear_ref(v);
		return e;
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
		return TypeError;
	}
	reraise = true;
	return ident_to_error(v);
}

Error catch_if(Stack* S, Stack* scope_arr)
{
	require(2);
	over(S, scope_arr);
	V err = popS();
	eq(S, scope_arr);
	V res = popS();
	if (truthy(res))
	{
		clear_ref(res);
		pushS(err);
		return Nothing;
	}
	clear_ref(res);
	return ident_to_error(err);
}

Error len(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	switch (getType(v))
	{
		case T_STR:
			pushS(int_to_value(toString(v)->length));
			break;
		case T_LIST:
			pushS(int_to_value(stack_size(toStack(v))));
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
	pushS(toScope(get_head(scope_arr))->func);
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
	void* lib_handle = dlopen(getChars(name), RTLD_NOW);
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
	open_lib(lib, &toScope(toFile(toScope(get_head(scope_arr))->file)->global)->hm);
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
	V v = real_get_hashmap(toHashMap(container), key);
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
	if (getType(container) == T_DICT)
	{
		v = get_hashmap(toHashMap(container), key);
	}
	else if (getType(container) == T_LIST)
	{
		if (getType(key) != T_NUM)
		{
			clear_ref(container);
			clear_ref(key);
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
	else
	{
		clear_ref(container);
		clear_ref(key);
		return TypeError;
	}
	if (v == NULL)
	{
		clear_ref(container);
		clear_ref(key);
		return ValueError;
	}
	pushS(add_ref(v));
	clear_ref(container);
	clear_ref(key);
	return Nothing;
}

Error set_to(Stack* S, Stack* scope_arr)
{
	require(3);
	V container = popS();
	V key = popS();
	V value = popS();
	if (getType(container) == T_DICT)
	{
		set_hashmap(toHashMap(container), key, value);
	}
	else if (getType(container) == T_LIST)
	{
		if (getType(key) != T_NUM)
		{
			clear_ref(container);
			clear_ref(key);
			clear_ref(value);
			return TypeError;
		}
		int index = (int)toNumber(key);
		Stack *s = toStack(container);
		if (index < 0)
			index = s->used + index;
		if (index < 0 || index >= s->used)
		{
			clear_ref(key);
			clear_ref(value);
			clear_ref(container);
			return ValueError;
		}
		else
		{
			s->nodes[index] = add_ref(value);
		}
	}
	else
	{
		clear_ref(key);
		clear_ref(value);
		clear_ref(container);
		return TypeError;
	}
	clear_ref(key);
	clear_ref(value);
	clear_ref(container);
	return Nothing;
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

Error print_var(Stack *S, Stack *scope_arr)
{
	while (true)
	{
		require(1);
		V head = get_head(S);
		if (getType(head) == T_IDENT && head == get_ident(")"))
		{
			clear_ref(popS());
			return Nothing;
		}
		print(S, scope_arr);
		putchar(' ');
	}
}

Error print_var_nl(Stack *S, Stack *scope_arr)
{
	Error e = print_var(S, scope_arr);
	if (e == Nothing)
	{
		putchar('\n');
	}
	return e;
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
	r = strtod(getChars(v), &end);
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
	// allocate a reasonable amount
	// need to take scientific notation etc. in account
	// also numbers near 0, which are long and have a negative log10
	// and negative numbers
	if (!toNumber(v))
	{
		pushS(str_to_value(1, "0"));
		clear_ref(v);
		return Nothing;
	}
	char *buff = int_str_buffer;
	sprintf(buff, "%.15g", toNumber(v));
	pushS(a_to_value(buff));
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
					error_msg = "keys and values should have simple types";
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
	V v = get_hashmap(&sc->hm, key);
	while (v == NULL)
	{
		if (sc->parent == NULL)
		{
			clear_ref(key);
			pushS(add_ref(v_false));
			return Nothing;
		}
		sc = toScope(sc->parent);
		v = get_hashmap(&sc->hm, key);
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
	if (getType(v) == T_NUM)
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
	if (getType(v) == T_NUM)
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

Error file_info(Stack* S, Stack* scope_arr)
{
	File *f = toFile(toScope(get_head(scope_arr))->file);
	Header *h = &f->header;
	printf("(source size:%d, literals:%d, filename:%s, source:%s, globals:%d)\n",
		h->size, h->n_literals, getChars(f->name), getChars(f->source),
		toScope(f->global)->hm.used);
	return Nothing;
}

Error print_f(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	switch (getType(v))
	{
		case T_STR:
			printf("%*s", toString(v)->length, getChars(v));
			break;
		case T_NUM:
			printf("%.15g", toNumber(v));
			break;
		default:
			print_value(v, 1);
	}
	clear_ref(v);
	return Nothing;
}

Error print_f_nl(Stack* S, Stack* scope_arr)
{
	require(1);
	print_f(S, scope_arr);
	putchar('\n');
	return Nothing;
}

Error print_f_var(Stack *S, Stack *scope_arr)
{
	while (true)
	{
		require(1);
		V head = get_head(S);
		if (getType(head) == T_IDENT && head == get_ident(")"))
		{
			clear_ref(popS());
			return Nothing;
		}
		print_f(S, scope_arr);
	}
}

Error print_f_var_nl(Stack *S, Stack *scope_arr)
{
	Error e = print_f_var(S, scope_arr);
	if (e == Nothing)
	{
		putchar('\n');
	}
	return e;
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

Error print_ident_count(Stack *S, Stack *scope_arr)
{
	printf("(idents:%d)\n", ident_count());
	return Nothing;
}

Error print_ident_depth(Stack *S, Stack *scope_arr)
{
	printf("(ident-depth:%d)\n", ident_depth());
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
		error_msg = "division by zero";
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
		error_msg = "invalid name for persisted data file";
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
		error_msg = "invalid name for persisted data file";
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
		error_msg = "invalid name for persisted data file";
		return ValueError;
	}
	char *path = make_persist_path(location);
	V file = load_file(a_to_value(path), toFile(toScope(get_head(scope_arr))->file)->global);
	free(path);
	if (file == NULL)
	{
		return IllegalFile;
	}
	if (getType(file) == T_FILE)
	{
		push(scope_arr, new_file_scope(file));
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
		error_msg = "probability should be between 0 and 1";
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
		toHashMap(p)->asdefault = popS();
		clear_ref(p);
		return Nothing;
	}
	else
	{
		clear_ref(p);
		return TypeError;
	}
}

static CFunc stdlib[] = {
	{"get", get},
	{"getglobal", getglobal},
	{"set", set},
	{"setglobal", setglobal},
	{"local", setlocal},
	{"+", add},
	{"add", add},
	{"-", sub},
	{"sub", sub},
	{"*", mul},
	{"mul", mul},
	{"/", div_},
	{"div", div_},
	{"%", mod_},
	{"mod", mod_},
	{".", print_nl},
	{".\\", print},
	{".\\(", print_var},
	{".(", print_var_nl},
	{"print\\", print_f},
	{"print", print_f_nl},
	{"print\\(", print_f_var},
	{"print(", print_f_var_nl},
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
	{"!=", ne},
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
	{"(print-stack)", print_stack},
	{"(print-depth)", print_depth},
	{"input", input},
	{"copy", copy},
	{"use", use},
	{"call", call},
	{"dup", dup},
	{"drop", drop},
	{"over", over},
	{"rot", rotate},
	{"error", error},
	{"raise", raise_},
	{"reraise", reraise_},
	{"catch-if", catch_if},
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
	{"?", exists_},
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
	{"(file-info)", file_info},
	{"abs", abs_},
	{"sin", sin_},
	{"cos", cos_},
	{"tan", tan_},
	{"asin", asin_},
	{"acos", acos_},
	{"atan", atan_},
	{"(ident-count)", print_ident_count},
	{"(ident-depth)", print_ident_depth},
	{"//", make_frac},
	{"clear", clear},
	{"time", time_},
	{"persist", persist_value},
	{"persist-stack", persist_stack},
	{"unpersist", unpersist},
	{"chance", chance},
	{"set-default", set_default},
	//strlib
	{"concat", concat},
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
	{NULL, NULL}
};

static char* autonyms[] = {"(", ")", "]", "}", NULL};

void open_lib(CFunc lib[], HashMap* hm)
{
	int i = 0;
	while (lib[i].name != NULL)
	{
		V v = new_cfunc(lib[i].cfunc);
		set_hashmap(hm, get_ident(lib[i].name), v);
		clear_ref(v);
		i++;
	}
}

void open_std_lib(HashMap* hm)
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
	v_range = new_cfunc(range);
	set_hashmap(hm, get_ident("true"), v_true);
	set_hashmap(hm, get_ident("false"), v_false);

	srand((unsigned int)time(NULL));
}

V new_cfunc(CFuncP func)
{
	V v = make_new_value(T_CFUNC, true, sizeof(CFuncP));
	toCFunc(v) = func;
	return v;
}
