#include "lib.h"

void print_value(V v, int depth)
{
	String* s;
	switch (getType(v))
	{
		case T_IDENT:
			s = toString(v);
			printf("'%*s'", s->length, toCharArr(s));
			break;
		case T_STR:
			s = toString(v);
			printf("%*s", s->length, toCharArr(s));
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
		case T_STACK:
			if (depth < 4)
			{
				fputs("[ ", stdout);
				Stack *st = toStack(v);
				Node *n = st->head;
				while (n)
				{
					print_value(n->data, depth + 1);
					fputs(" ", stdout);
					n = n->next;
			}
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
		case T_CFUNC:
			printf("<func:%p>", toCFunc(v));
			break;
		case T_FUNC:
			printf("<func:%p>", toFunc(v));
			break;
	};
}

Error get(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V key = pop(S);
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
	push(S, add_ref(v));
	clear_ref(key);
	return Nothing;
}

Error getglobal(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V key = pop(S);
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
	push(S, add_ref(r));
	return Nothing;
}

Error set(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V key = pop(S);
	if (getType(key) != T_IDENT)
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
	if (getType(key) != T_IDENT)
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
	if (getType(key) != T_IDENT)
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
	switch (getType(r))
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
	V t = get_ident(gettype(v));
	push(S, t);
	clear_ref(v);
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
	putchar('\n');
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
		if (getType(p) == T_IDENT)
		{
			s = toString(p);
			if (s->length == 1 && toCharArr(s)[0] == ']')
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
		if (getType(key) == T_IDENT)
		{
			s = toString(key);
			if (s->length == 1 && toCharArr(s)[0] == '}')
			{
				clear_ref(key);
				push(S, v);
				return Nothing;
			}
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
	{
		V r = toNumber(v1) < toNumber(v2) ? v_true : v_false;
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
	push(S, add_ref(equal(v1, v2) ? v_true : v_false));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error ne(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	push(S, add_ref(equal(v1, v2) ? v_false : v_true));
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

V v_range;

Error range(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v1;
	V v2;
	V v = pop(S);
	if (getType(v) == T_STACK)
	{
		if (stack_size(toStack(v)) < 2)
		{
			clear_ref(v);
			return StackEmpty;
		}
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
	if (getType(v1) == T_NUM && getType(v2) == T_NUM)
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
			//push(S, new_cfunc(range));
			/* METHOD 3: just use a global value
			*/
			push(S, add_ref(v_range));
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
	if (getType(list) != T_STACK)
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
	if (getType(list) != T_STACK)
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
	Node* n = S->head;
	fputs("[ ", stdout);
	while (n != NULL)
	{
		print_value(n->data, 0);
		putchar(' ');
		n = n->next;
	}
	puts("]");
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
	if (getType(list) != T_STACK)
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
	if (getType(list) != T_STACK)
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
	if (getType(list) != T_STACK)
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
	if (getType(get_head(S)) != T_IDENT)
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
	if (getType(v) == T_FUNC)
	{
		push(scope_arr, new_function_scope(v));
		clear_ref(v);
	}
	else if (getType(v) == T_CFUNC)
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
	char line[256];
	fgets(line, 256, stdin);
	line[strlen(line) - 1] = '\0'; //removes trailing newline
	push(S, a_to_value(line));
	return Nothing;
}

Error copy(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	if (getType(v) != T_STACK)
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

Error call(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = get_head(S);
	if (getType(v) == T_IDENT)
	{
		Error e = get(h, S, scope_arr);
		if (e != Nothing)
		{
			return e;
		}
	}
	v = pop(S);
	if (getType(v) == T_FUNC)
	{
		push(scope_arr, new_function_scope(v));
		clear_ref(v);
	}
	else if (getType(v) == T_CFUNC)
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
	push(S, add_ref(S->head->next->data));
	return Nothing;
}

Error rotate(Header* h, Stack* S, Stack* scope_arr)
{
	require(3);
	Node *a = S->head;
	Node *b = a->next;
	Node *c = b->next;
	a->next = c->next;
	c->next = a;
	S->head = b;
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
	if (getType(v) != T_IDENT)
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
	switch (getType(v))
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
		return initfunc(h, S, scope_arr);
	}
	return Nothing;
}

Error has(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V container = pop(S);
	V key = pop(S);
	if (getType(container) != T_DICT)
	{
		clear_ref(container);
		clear_ref(key);
		return TypeError;
	}
	V v = get_hashmap(toHashMap(container), key);
	push(S, add_ref(v == NULL ? v_false : v_true));
	clear_ref(container);
	clear_ref(key);
	return Nothing;
}

Error get_from(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V container = pop(S);
	V key = pop(S);
	if (getType(container) != T_DICT)
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
	if (getType(container) != T_DICT)
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
	if (getType(num) != T_NUM)
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

Error export(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V modname = pop(S);
	if (getType(modname) != T_IDENT)
	{
		clear_ref(modname);
		return TypeError;
	}
	int modlength = toString(modname)->length;
	V functions = pop(S);
	if (getType(functions) != T_STACK)
	{
		clear_ref(modname);
		clear_ref(functions);
		return TypeError;
	}
	Stack *fs = toStack(functions);
	while (stack_size(fs))
	{
		V name = pop(fs);
		if (getType(fs->head->data) != T_IDENT)
		{
			clear_ref(modname);
			clear_ref(functions);
			clear_ref(name);
			return TypeError;
		}
		push(S, name);
		Error e = get(h, S, scope_arr);
		if (e != Nothing)
		{
			clear_ref(modname);
			clear_ref(functions);
			clear_ref(name);
			return e;
		}
		require(1);
		char *start = NULL;
		int strln = modlength + 1 + toString(name)->length;
		V globalname = empty_str_to_value(strln, &start);
		char *buffer = start;
		memcpy(buffer, getChars(modname), modlength);
		buffer += modlength;
		*buffer = '.';
		buffer++;
		memcpy(buffer, getChars(name), toString(name)->length);
		toString(globalname)->hash = string_hash(strln, start);
		globalname->type = T_IDENT;
		push(S, globalname);
		e = setglobal(h, S, scope_arr);
		clear_ref(name);
		clear_ref(globalname);
		if (e != Nothing)
		{
			return e;
		}
	}
	clear_ref(modname);
	clear_ref(functions);
	return Nothing;
}

Error quote(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = get_head(S);
	if (getType(v) != T_IDENT)
	{
		clear_ref(v);
		return TypeError;
	}
	String *s = toString(v);
	char *buff = malloc(s->length + 2);
	buff[0] = '"';
	memcpy(buff + 1, toCharArr(s), s->length + 1); 
	V r = get_ident(buff);
	free(buff);
	push(S, add_ref(r));
	V tmp = get_hashmap(&toScope(toFile(toScope(get_head(scope_arr))->file)->global)->hm, r);
	if (tmp != v)
	{
		Error e = setglobal(h, S, scope_arr);
		if (e != Nothing)
		{
			return e;
		}
		push(S, add_ref(r));
	}
	return Nothing;
}

Error print_var(Header *h, Stack *S, Stack *scope_arr)
{
	while (true)
	{
		require(1);
		V head = get_head(S);
		if (getType(head) == T_IDENT && toString(head)->length == 1 && getChars(head)[0] == ')')
		{
			clear_ref(pop(S));
			return Nothing;
		}
		print(h, S, scope_arr);
	}
}

Error print_var_nl(Header *h, Stack *S, Stack *scope_arr)
{
	Error e = print_var(h, S, scope_arr);
	if (e == Nothing)
	{
		putchar('\n');
	}
	return e;
}

Error to_num(Header *h, Stack *S, Stack *scope_arr)
{
	char *end;
	double r;
	require(1);
	V v = pop(S);
	int type = getType(v);
	if (type == T_NUM)
	{
		push(S, v);
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
	push(S, double_to_value(r));
	return Nothing;
}

Error to_str(Header *h, Stack *S, Stack *scope_arr)
{
	V v = pop(S);
	int type = getType(v);
	if (type == T_STR)
	{
		push(S, v);
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
	char *buff = malloc(fmin(5 + abs(log10(fabs(toNumber(v)))), 30));
	sprintf(buff, "%.15g", toNumber(v));
	push(S, a_to_value(buff));
	free(buff);
	return Nothing;
}

Error pass(Header *h, Stack *S, Stack *scope_arr)
{
	return Nothing;
}

Error rand_(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v_min = pop(S);
	V v_max = pop(S);
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
	push(S, double_to_value(ans));
	return Nothing;
}

Error keys(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V dict = pop(S);
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
	push(S, list);
	return Nothing;
}

Error exists_(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V key = pop(S);
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
			push(S, add_ref(v_false));
			return Nothing;
		}
		sc = toScope(sc->parent);
		v = get_hashmap(&sc->hm, key);
	}
	push(S, add_ref(v_true));
	clear_ref(key);
	clear_ref(v);
	return Nothing;
}

Error floor_(Header *h, Stack *S, Stack *scope_arr)
{
	require(1);
	V v = pop(S);
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	push(S, double_to_value(floor(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error ceil_(Header *h, Stack *S, Stack *scope_arr)
{
	require(1);
	V v = pop(S);
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	push(S, double_to_value(ceil(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error round_(Header *h, Stack *S, Stack *scope_arr)
{
	require(1);
	V v = pop(S);
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	push(S, double_to_value(round(toNumber(v))));
	clear_ref(v);
	return Nothing;
}

Error produce_set(Header *h, Stack *S, Stack *scope_arr)
{
	V v = new_dict();
	V val;
	String *s;
	while (stack_size(S) > 0)
	{
		val = pop(S);
		if (getType(val) == T_IDENT)
		{
			s = toString(val);
			if (s->length == 1 && toCharArr(s)[0] == '}')
			{
				clear_ref(val);
				push(S, v);
				return Nothing;
			}
		}
		set_hashmap(toHashMap(v), val, v_true);
	}
	return StackEmpty;
}

Error in_set(Header *h, Stack *S, Stack *scope_arr)
{
	require(2);
	V set = pop(S);
	if (getType(set) != T_DICT)
	{
		clear_ref(set);
		return TypeError;
	}
	V item = pop(S);
	push(S, add_ref(get_hashmap(toHashMap(set), item) == v_true ? v_true : v_false));
	clear_ref(set);
	clear_ref(item);
	return Nothing;
}

Error add_set(Header *h, Stack *S, Stack *scope_arr)
{
	require(2);
	V set = pop(S);
	if (getType(set) != T_DICT)
	{
		clear_ref(set);
		return TypeError;
	}
	V item = pop(S);
	set_hashmap(toHashMap(set), item, v_true);
	clear_ref(set);
	clear_ref(item);
	return Nothing;
}

Error remove_set(Header *h, Stack *S, Stack *scope_arr)
{
	require(2);
	V set = pop(S);
	if (getType(set) != T_DICT)
	{
		clear_ref(set);
		return TypeError;
	}
	V item = pop(S);
	set_hashmap(toHashMap(set), item, v_false);
	clear_ref(set);
	clear_ref(item);
	return Nothing;
}

Error plus_one(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	if (getType(v) == T_NUM)
	{
		V r = double_to_value(toNumber(v) + 1.0);
		clear_ref(v);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v);
		return TypeError;
	}
}

Error minus_one(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	if (getType(v) == T_NUM)
	{
		V r = double_to_value(toNumber(v) - 1.0);
		clear_ref(v);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v);
		return TypeError;
	}
}

Error undef(Header* h, Stack* S, Stack* scope_arr)
{
	return UnknownError;
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
	{"export", export},
	{"quote", quote},
	{"to-num", to_num},
	{"to-str", to_str},
	{"pass", pass},
	{"rand", rand_},
	{"keys", keys},
	{"?", exists_},
	{"exists?", exists_},
	{"floor", floor_},
	{"ceil", ceil_},
	{"round", round_},
	{"set{", produce_set},
	{"in-set?", in_set},
	{"add-set", add_set},
	{"remove-set", remove_set},
	{"++", plus_one},
	{"--", minus_one},
	{"undef", undef},
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
