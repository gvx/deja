#include "lib.h"

void print_value(V v, int depth)
{
	String* s;
	switch (v->type)
	{
		case T_IDENT:
			s = toString(v);
			printf("'%*s'", s->length, s->data);
			break;
		case T_STR:
			s = toString(v);
			printf("%*s", s->length, s->data);
			break;
		case T_NUM:
			printf("%.15g", toNumber(v));
			break;
		case T_STACK:
			if (depth < 4)
			{
				printf("[ ");
				Stack *st = toStack(v);
				StackArray *n = st->head;
				int i;
				while (n)
				{
					for (i = 0; i < n->numitems; i++)
					{
						print_value(n->items[i], depth + 1);
						printf(" ");
					}
					n = n->next;
				}
				printf("]");
			}
			else
			{
				printf("[...]");
			}
			break;
		case T_DICT:
			if (depth < 4)
			{
				printf("{");
				int i;
				HashMap *hm = toHashMap(v);
				for (i = 0; i < hm->size; i++)
				{
					Bucket *b = hm->map[i];
					while (b)
					{
						printf(" '%*s' ", b->keysize, b->key);
						print_value(b->value, depth + 1);
						b = b->next;
					}
				}
				printf(" }");
			}
			else
			{
				printf("{...}");
			}
			break;
		case T_CFUNC:
		case T_FUNC:
			printf("<func>");
			break;
	};
}

Error get(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V key = pop(S);
	if (key->type != T_IDENT)
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
	push(S, add_ref(v));
	clear_ref(key);
	return Nothing;
}

Error getglobal(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V key = pop(S);
	if (key->type != T_IDENT)
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
	push(S, add_ref(r));
	return Nothing;
}

Error set(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V key = pop(S);
	if (key->type != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}
	V v = pop(S);
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

Error setglobal(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V key = pop(S);
	if (key->type != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}
	V v = pop(S);
	set_hashmap(&toScope(toFile(toScope(get_head(scope_arr))->file)->global)->hm, key, v);
	clear_ref(v);
	clear_ref(key);
	return Nothing;
}

Error setlocal(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V key = pop(S);
	if (key->type != T_IDENT)
	{
		clear_ref(key);
		return TypeError;
	}
	V v = pop(S);
	set_hashmap(&toScope(get_head(scope_arr))->hm, key, v);
	clear_ref(v);
	clear_ref(key);
	return Nothing;
}

Error add(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = double_to_value(toNumber(v1) + toNumber(v2));
		clear_ref(v1);
		clear_ref(v2);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error sub(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = double_to_value(toNumber(v1) - toNumber(v2));
		clear_ref(v1);
		clear_ref(v2);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error mul(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = double_to_value(toNumber(v1) * toNumber(v2));
		clear_ref(v1);
		clear_ref(v2);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error div_(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		if (toNumber(v2) == 0.0)
		{
			clear_ref(v1);
			clear_ref(v2);
			return ValueError;
		}
		V r = double_to_value(toNumber(v1) / toNumber(v2));
		clear_ref(v1);
		clear_ref(v2);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error mod_(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		if (toNumber(v2) == 0.0)
		{
			clear_ref(v1);
			clear_ref(v2);
			return ValueError;
		}
		V r = double_to_value(fmod(toNumber(v1), toNumber(v2)));
		clear_ref(v1);
		clear_ref(v2);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

const char* gettype(V r)
{
	switch (r->type)
	{
		case T_IDENT:
			return "ident";
		case T_STR:
			return "str";
		case T_NUM:
			return "num";
		case T_STACK:
			return "list";
		case T_DICT:
			return "dict";
		case T_FUNC:
		case T_CFUNC:
			return "func";
		default:
			return "nil"; //not really true, but meh.
	}

}

Error type(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	if (v->type != T_IDENT)
	{
		clear_ref(v);
		return TypeError;
	}
	V t;
	V r = get_hashmap(&toScope(toFile(toScope(get_head(scope_arr))->file)->global)->hm, v);
	clear_ref(v);
	if (r == NULL)
	{
		t = get_ident("nil");
	}
	else
	{
		t = get_ident(gettype(r));
	}
	clear_ref(r);
	push(S, t);
	return Nothing;
}

Error print(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	print_value(v, 0);
	clear_ref(v);
	return Nothing;
}

Error print_nl(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	print(h, S, scope_arr);
	printf("\n");
	return Nothing;
}

Error make_new_list(Header* h, Stack* S, Stack* scope_arr)
{
	push(S, new_list());
	return Nothing;
}

Error make_new_dict(Header* h, Stack* S, Stack* scope_arr)
{
	push(S, new_dict());
	return Nothing;
}

Error produce_list(Header* h, Stack* S, Stack* scope_arr)
{
	V v = new_list();
	V p;
	String *s;
	while (stack_size(S) > 0)
	{
		p = pop(S);
		if (p->type == T_IDENT)
		{
			s = toString(p);
			if (s->length == 1 && s->data[0] == ']')
			{
				clear_ref(p);
				push(S, v);
				return Nothing;
			}
		}
		push(toStack(v), p);
	}
	return StackEmpty;
}

Error produce_dict(Header* h, Stack* S, Stack* scope_arr)
{
	V v = new_dict();
	V key;
	V val;
	String *s;
	while (stack_size(S) > 0)
	{
		key = pop(S);
		if (key->type != T_IDENT)
		{
			clear_ref(key);
			return TypeError;
		}
		s = toString(key);
		if (s->length == 1 && s->data[0] == '}')
		{
			clear_ref(key);
			push(S, v);
			return Nothing;
		}
		val = pop(S);
		if (val == NULL)
		{
			return StackEmpty;
		}
		set_hashmap(toHashMap(v), key, val);
	}
	return StackEmpty;
}


Error if_(Header* h, Stack* S, Stack* scope_arr)
{
	require(3);
	V v0 = pop(S);
	V v1 = pop(S);
	V v2 = pop(S);
	if (truthy(v0))
	{
		clear_ref(v2);
		push(S, v1);
	}
	else
	{
		clear_ref(v1);
		push(S, v2);
	}
	clear_ref(v0);
	return Nothing;
}

Error return_(Header* h, Stack* S, Stack* scope_arr)
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

Error exit_(Header* h, Stack* S, Stack* scope_arr)
{
	return Exit;
}

Error lt(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = toNumber(v1) <= toNumber(v2) ? v_true : v_false;
		clear_ref(v1);
		clear_ref(v2);
		push(S, add_ref(r));
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error gt(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = toNumber(v1) > toNumber(v2) ? v_true : v_false;
		clear_ref(v1);
		clear_ref(v2);
		push(S, add_ref(r));
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error le(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = toNumber(v1) <= toNumber(v2) ? v_true : v_false;
		clear_ref(v1);
		clear_ref(v2);
		push(S, add_ref(r));
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error ge(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = toNumber(v1) >= toNumber(v2) ? v_true : v_false;
		clear_ref(v1);
		clear_ref(v2);
		push(S, add_ref(r));
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error eq(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	int t = 0;
	if (v1 == v2) //identical objects
	{
		t = 1;
	}
	else if (v1->type == v2->type)
	{
		if (v1->type == T_NUM)
		{
			t = toNumber(v1) == toNumber(v2);
		}
		else if (v1->type == T_IDENT || v1->type == T_STR)
		{
			String* s1 = toString(v1);
			String* s2 = toString(v2);
			if (s1->length == s2->length)
			{
				t = !memcmp(s1->data, s2->data, s1->length);
			}
		}
		else
		{
			t = v1->data.object == v2->data.object;
		}
	}
	push(S, add_ref(t ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error ne(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	int t = 1;
	if (v1 == v2) //identical objects
	{
		t = 0;
	}
	else if (v1->type == v2->type)
	{
		if (v1->type == T_NUM)
		{
			t = toNumber(v1) != toNumber(v2);
		}
		else if (v1->type == T_IDENT || v1->type == T_STR)
		{
			String* s1 = toString(v1);
			String* s2 = toString(v2);
			if (s1->length == s2->length)
			{
				t = !!memcmp(s1->data, s2->data, s1->length);
			}
		}
	}
	push(S, add_ref(t ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error not(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	push(S, add_ref(truthy(v) ? v_false : v_true));
	clear_ref(v);
	return Nothing;
}

Error and(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	push(S, add_ref(truthy(v1) && truthy(v2) ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error or(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	push(S, add_ref(truthy(v1) || truthy(v2) ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error xor(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	push(S, add_ref(truthy(v1) != truthy(v2) ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}


Error range(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v1;
	V v2;
	V v = pop(S);
	if (v->type == T_STACK)
	{
		v1 = pop(toStack(v));
		v2 = pop(toStack(v));
		clear_ref(v);
	}
	else
	{
		require(1);
		v1 = v;
		v2 = pop(S);
	}
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		if (toNumber(v1) > toNumber(v2))
		{
			push(S, int_to_value(0));
			clear_ref(v1);
			clear_ref(v2);
		}
		else
		{
			push(S, v1);
			V list = new_list();
			push(toStack(list), v2);
			push(toStack(list), double_to_value(toNumber(v1) + 1.0));
			push(S, list);
			/* METHOD 1: look up
			   more computation
			   fails if the global "range" is overwritten
			*/
			//push(S, add_ref(get_hashmap(&toScope(toFile(toScope(scope_arr->head->data)->file)->global)->hm, get_ident("range"))));
			/* METHOD 2: create value
			   less computation
			   works even if the global "range" is overwritten
			*/
			push(S, new_cfunc(range));
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

Error in(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return TypeError;
	}
	if (stack_size(toStack(list)) > 0)
	{
		V item = pop(toStack(list));
		push(S, item);
		push(S, list);
		push(S, new_cfunc(in));
	}
	else
	{
		push(S, int_to_value(0));
	}
	return Nothing;
}

Error reversed(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return TypeError;
	}
	V rev = new_list();
	while (stack_size(toStack(list)) > 0)
	{
		push(toStack(rev), pop(toStack(list)));
	}
	push(S, rev);
	return Nothing;
}

Error print_stack(Header* h, Stack* S, Stack* scope_arr)
{
	int i;
	StackArray* n = S->head;
	printf("[ ");
	while (n != NULL)
	{
		for (i = 0; i < n->numitems; i++)
		{
			print_value(n->items[i], 0);
			printf(" ");
		}
		n = n->next;
	}
	printf("]\n");
	return Nothing;
}

Error swap(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	push(S, v1);
	push(S, v2);
	return Nothing;
}

Error push_to(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return TypeError;
	}
	V val = pop(S);
	push(toStack(list), val);
	clear_ref(list);
	return Nothing;
}

Error push_through(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return TypeError;
	}
	V val = pop(S);
	push(toStack(list), val);
	push(S, list);
	return Nothing;
}

Error pop_from(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return TypeError;
	}
	V val = pop(toStack(list));
	push(S, val);
	clear_ref(list);
	return Nothing;
}

Error tail_call(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	if (get_head(S)->type != T_IDENT)
	{
		return TypeError;
	}
	Error e = get(h, S, scope_arr);
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
	v = pop(S);
	if (v->type == T_FUNC)
	{
		push(scope_arr, new_function_scope(v));
		clear_ref(v);
	}
	else if (v->type == T_CFUNC)
	{
		e = toCFunc(v)(h, S, scope_arr);
		clear_ref(v);
		return e;
	}
	else
	{
		push(S, v);
	}
	return Nothing;
}

Error self_tail(Header* h, Stack* S, Stack* scope_arr)
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

Error print_depth(Header* h, Stack* S, Stack* scope_arr)
{
	printf("(depth:%d)\n", stack_size(scope_arr));
	return Nothing;
}

Error input(Header* h, Stack* S, Stack* scope_arr)
{
	char line[80];
	fgets(line, 80, stdin);
	push(S, a_to_value(line));
	return Nothing;
}

Error copy(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	if (v->type != T_STACK)
	{
		clear_ref(v);
		return TypeError;
	}
	V new = new_list();
	copy_stack(toStack(v), toStack(new));
	push(S, new);
	return Nothing;
}

Error use(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V fname = pop(S);
	if (fname->type != T_IDENT)
	{
		return TypeError;
	}
	V file = load_file(find_file(fname), toFile(toScope(get_head(scope_arr))->file)->global);
	if (file == NULL)
	{
		return IllegalFile;
	}
	if (file->type == T_FILE)
	{
		push(scope_arr, new_file_scope(file));
	}
	clear_ref(file);
	return Nothing;
}

Error call(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = get_head(S);
	if (v->type == T_IDENT)
	{
		Error e = get(h, S, scope_arr);
		if (e != Nothing)
		{
			return e;
		}
	}
	v = pop(S);
	if (v->type == T_FUNC)
	{
		push(scope_arr, new_function_scope(v));
		clear_ref(v);
	}
	else if (v->type == T_CFUNC)
	{
		Error e = toCFunc(v)(h, S, scope_arr);
		clear_ref(v);
		return e;
	}
	else
	{
		push(S, v);
	}
	return Nothing;
}

Error dup(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	push(S, add_ref(get_head(S)));
	return Nothing;
}

Error drop(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	clear_ref(pop(S));
	return Nothing;
}

Error over(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	push(S, add_ref(S->head->items[1]));
	return Nothing;
}

Error rotate(Header* h, Stack* S, Stack* scope_arr)
{
	require(3);
	V *items = S->head->items;
	V tmp = items[0];
	items[0] = items[1];
	items[1] = items[2];
	items[2] = tmp;
	return Nothing;
}

Error error(Header* h, Stack* S, Stack* scope_arr)
{
	return UserError;
}

Error raise_(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	if (v->type != T_IDENT)
	{
		return TypeError;
	}
	return ident_to_error(v);
}

Error catch_if(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	over(h, S, scope_arr);
	V err = pop(S);
	eq(h, S, scope_arr);
	V res = pop(S);
	if (truthy(res))
	{
		clear_ref(res);
		push(S, err);
		return Nothing;
	}
	clear_ref(res);
	return ident_to_error(err);
}

Error len(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	switch (v->type)
	{
		case T_STR:
		case T_IDENT:
			push(S, int_to_value(toString(v)->length));
			break;
		case T_STACK:
			push(S, int_to_value(stack_size(toStack(v))));
			break;
		default:
			clear_ref(v);
			return TypeError;
	}
	clear_ref(v);
	return Nothing;
}

Error yield(Header* h, Stack* S, Stack* scope_arr)
{
	push(S, toScope(get_head(scope_arr))->func);
	return return_(h, S, scope_arr);
}

void open_lib(CFunc[], HashMap*);

Error loadlib(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V name = pop(S);
	if (name->type != T_STR)
	{
		return TypeError;
	}
	void* lib_handle = dlopen(toString(name)->data, RTLD_NOW);
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
	return Nothing;
}

Error has(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V container = pop(S);
	V key = pop(S);
	if (container->type != T_DICT)
	{
		clear_ref(container);
		clear_ref(key);
		return TypeError;
	}
	V v = get_hashmap(toHashMap(container), key);
	push(S, v == NULL ? v_true : v_false);
	clear_ref(container);
	clear_ref(key);
	return Nothing;
}

Error get_from(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V container = pop(S);
	V key = pop(S);
	if (container->type != T_DICT)
	{
		clear_ref(container);
		clear_ref(key);
		return TypeError;
	}
	V v = get_hashmap(toHashMap(container), key);
	if (v == NULL)
	{
		clear_ref(container);
		clear_ref(key);
		return ValueError;
	}
	push(S, add_ref(v));
	clear_ref(container);
	clear_ref(key);
	return Nothing;
}

Error set_to(Header* h, Stack* S, Stack* scope_arr)
{
	require(3);
	V container = pop(S);
	V key = pop(S);
	V value = pop(S);
	if (container->type != T_DICT || key->type != T_IDENT)
	{
		clear_ref(key);
		clear_ref(value);
		clear_ref(container);
		return TypeError;
	}
	set_hashmap(toHashMap(container), key, value);
	clear_ref(key);
	clear_ref(value);
	clear_ref(container);
	return Nothing;
}

Error rep(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V num = pop(S);
	if (num->type != T_NUM)
	{
		clear_ref(num);
		return TypeError;
	}
	V val = pop(S);
	int i;
	for (i = toNumber(num); i > 0; i--)
	{
		push(S, add_ref(val));
	}
	clear_ref(val);
	clear_ref(num);
	return Nothing;
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
	{"catch-if", catch_if},
	{"len", len},
	{"yield", yield},
	{"loadlib", loadlib},
	{"{}", make_new_dict},
	{"{", produce_dict},
	{"has", has},
	{"get-from", get_from},
	{"set-to", set_to},
	{"rep", rep},
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
	{NULL, NULL}
};

static char* autonyms[] = {"(", ")", "]", "}", NULL};

void open_lib(CFunc lib[], HashMap* hm)
{
	int i = 0;
	while (lib[i].name != NULL)
	{
		set_hashmap(hm, get_ident(lib[i].name), new_cfunc(lib[i].cfunc));
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
	v_true = int_to_value(1);
	v_false = int_to_value(0);
	set_hashmap(hm, get_ident("true"), v_true);
	set_hashmap(hm, get_ident("false"), v_false);
}

V new_cfunc(Error (*func)(Header*, Stack*, Stack*))
{
	V v = new_value(T_CFUNC);
	v->data.object = func;
	v->color = Green;
	return v;
}