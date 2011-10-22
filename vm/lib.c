#include "lib.h"

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
	while (stack_size(S))
	{
		p = pop(S);
		if (p->type == T_IDENT)
		{
			s = toString(p);
			if (s->length == 1 && s->data[0] == ']')
			{
				clear_ref(p);
				return Nothing;
			}
		}
		push(toStack(v), p);
		clear_ref(p);
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
		if (v != NULL)
		{
			clear_ref(v);
		}
		v = pop(scope_arr);
		if (v == NULL)
		{
			// EOF
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

static CFunc stdlib[] = {
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
}