#include "lib.h"

void print_value(V v)
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
			printf("%g", toNumber(v));
			break;
		case T_STACK:
			printf("<stack>");
			break;
		case T_CFUNC:
		case T_FUNC:
			printf("<func>");
			break;
	};
}

Error get(Header* h, Stack* S, Stack* scope_arr)
{
	if (stack_size(S) < 1)
	{
		return StackEmpty;
	}
	V key = pop(S);
	if (key->type != T_IDENT)
	{
		return ValueError;
	}
	Scope *sc = toScope(get_head(scope_arr));
	V v = get_hashmap(&sc->hm, key);
	while (v == NULL)
	{
		sc = toScope(sc->parent);
		if (sc == NULL)
		{
			return NameError;
		}
		v = get_hashmap(&sc->hm, key);
	}
	push(S, add_ref(v));
	clear_ref(key);
	return Nothing;
}

Error add(Header* h, Stack* S, Stack* scope_arr)
{
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
		return ValueError;
	}
}

Error sub(Header* h, Stack* S, Stack* scope_arr)
{
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
		return ValueError;
	}
}

Error mul(Header* h, Stack* S, Stack* scope_arr)
{
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
		return ValueError;
	}
}

Error div_(Header* h, Stack* S, Stack* scope_arr)
{
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
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
		return ValueError;
	}
}

Error mod_(Header* h, Stack* S, Stack* scope_arr)
{
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
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
		return ValueError;
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
		case T_FUNC:
		case T_CFUNC:
			return "func";
		default:
			return "nil"; //not really true, but meh.
	}

}

Error type(Header* h, Stack* S, Stack* scope_arr)
{
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
	V v = pop(S);
	print_value(v);
	clear_ref(v);
	return Nothing;
}

Error print_nl(Header* h, Stack* S, Stack* scope_arr)
{
	print(h, S, scope_arr);
	printf("\n");
	return Nothing;
}

Error make_new_list(Header* h, Stack* S, Stack* scope_arr)
{
	V v = newlist();
	push(S, v);
	return Nothing;
}

Error produce_list(Header* h, Stack* S, Stack* scope_arr)
{
	V v = newlist();
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

Error if_(Header* h, Stack* S, Stack* scope_arr)
{
	V v0 = pop(S);
	V v1 = pop(S);
	V v2 = pop(S);
	if (truthy(v0))
	{
		push(S, v1);
	}
	else
	{
		push(S, v2);
	}
	clear_ref(v0);
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error return_(Header* h, Stack* S, Stack* scope_arr)
{
	V v = NULL;
	V scope = scope_arr->head->data;
	Func* f = toFunc(toScope(scope)->func);
	do
	{
		clear_ref(v);
		v = pop(scope_arr);
		if (v == NULL)
		{
			return Exit;
		}
	}
	while (toFunc(toScope(v)->func) == f);
	push(scope_arr, v);
	return Nothing;
}

Error exit_(Header* h, Stack* S, Stack* scope_arr)
{
	return Exit;
}

Error lt(Header* h, Stack* S, Stack* scope_arr)
{
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = double_to_value(toNumber(v1) < toNumber(v2));
		clear_ref(v1);
		clear_ref(v2);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return ValueError;
	}
}

Error gt(Header* h, Stack* S, Stack* scope_arr)
{
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_NUM && v2->type == T_NUM)
	{
		V r = double_to_value(toNumber(v1) > toNumber(v2));
		clear_ref(v1);
		clear_ref(v2);
		push(S, r);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return ValueError;
	}
}

Error eq(Header* h, Stack* S, Stack* scope_arr)
{
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
	}
	push(S, int_to_value(t));
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error not(Header* h, Stack* S, Stack* scope_arr)
{
	V v = pop(S);
	push(S, int_to_value(truthy(v) ? 0 : 1));
	clear_ref(v);
	return Nothing;
}

Error range(Header* h, Stack* S, Stack* scope_arr)
{
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
			V list = newlist();
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
			V r = new_value(T_CFUNC);
			r->data.object = range;
			push(S, r);
		}
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return ValueError;
	}
}

Error in(Header* h, Stack* S, Stack* scope_arr)
{
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return ValueError;
	}
	if (stack_size(toStack(list)) > 0)
	{
		V item = pop(toStack(list));
		push(S, item);
		push(S, list);
		V r = new_value(T_CFUNC);
		r->data.object = in;
		push(S, r);
	}
	else
	{
		push(S, int_to_value(0));
	}
	return Nothing;
}

Error reversed(Header* h, Stack* S, Stack* scope_arr)
{
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return ValueError;
	}
	V rev = newlist();
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
	printf("[ ");
	while (n != NULL)
	{
		print_value(n->data);
		printf(" ");
		n = n->next;
	}
	printf("]\n");
	return Nothing;
}

Error swap(Header* h, Stack* S, Stack* scope_arr)
{
	if (stack_size(S) < 2)
	{
		return StackEmpty;
	}
	V v1 = pop(S);
	V v2 = pop(S);
	push(S, v1);
	push(S, v2);
	return Nothing;
}

Error push_to(Header* h, Stack* S, Stack* scope_arr)
{
	if (stack_size(S) < 2)
	{
		return StackEmpty;
	}
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return ValueError;
	}
	V val = pop(S);
	push(toStack(list), val);
	clear_ref(list);
	return Nothing;
}

Error push_through(Header* h, Stack* S, Stack* scope_arr)
{
	if (stack_size(S) < 2)
	{
		return StackEmpty;
	}
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return ValueError;
	}
	V val = pop(S);
	push(toStack(list), val);
	push(S, list);
	return Nothing;
}

Error pop_from(Header* h, Stack* S, Stack* scope_arr)
{
	if (stack_size(S) < 1)
	{
		return StackEmpty;
	}
	V list = pop(S);
	if (list->type != T_STACK)
	{
		clear_ref(list);
		return ValueError;
	}
	V val = pop(toStack(list));
	push(S, val);
	clear_ref(list);
	return Nothing;
}

Error tail_call(Header* h, Stack* S, Stack* scope_arr)
{
	if (stack_size(S) < 1)
	{
		return StackEmpty;
	}
	V v = NULL;
	if (get_head(S)->type != T_IDENT)
	{
		return ValueError;
	}
	Error e = get(h, S, scope_arr);
	if (e != Nothing)
	{
		return e;
	}
	Func* f = toFunc(toScope(get_head(scope_arr))->func);
	do
	{
		clear_ref(v);
		v = pop(scope_arr);
		if (v == NULL)
		{
			return Exit;
		}
	}
	while (toFunc(toScope(v)->func) == f);
	push(scope_arr, v);
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
	Func* f = toFunc(toScope(get_head(scope_arr))->func);
	do
	{
		clear_ref(v);
		v = pop(scope_arr);
		if (v == NULL)
		{
			return Exit;
		}
	}
	while (toFunc(toScope(get_head(scope_arr))->func) == f);
	push(scope_arr, v);
	Scope* sc = toScope(v);
	sc->pc = f->start;
	return Nothing;
}

static CFunc stdlib[] = {
	{"get", get},
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
	{"not", not},
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
	{NULL, NULL}
};

static char* autonyms[] = {"(", ")", "]", NULL};

void open_lib(CFunc lib[], HashMap* hm)
{
	int i = 0;
	V s;
	V v;
	while (lib[i].name != NULL)
	{
		s = get_ident(lib[i].name); 
		v = new_value(T_CFUNC);
		v->data.object = lib[i].cfunc;
		set_hashmap(hm, s, v);
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
	set_hashmap(hm, get_ident("true"), int_to_value(1));
	set_hashmap(hm, get_ident("false"), int_to_value(0));
}