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

Error div(Header* h, Stack* S, Stack* scope_arr)
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

Error print(Header* h, Stack* S, Stack* scope_arr)
{
	V v = pop(S);
	switch (v->type)
	{
		case T_NUM:
			printf("%f\n", toNumber(v));
			break;
	};
	clear_ref(v);
}

static CFunc stdlib[] = {
	{"+", add},
	{"add", add},
	{"-", sub},
	{"sub", sub},
	{"*", mul},
	{"mul", mul},
	{"/", div},
	{"div", div},
	{".", print},
	{NULL, NULL}
};

void open_lib(HashMap* hm)
{
	int i = 0;
	V s;
	V v;
	while (stdlib[i].name != NULL)
	{
		s = a_to_value(stdlib[i].name); 
		v = new_value(T_CFUNC);
		v->data.object = stdlib[i].cfunc;
		set_hashmap(hm, s, v);
		i++;
	}
}