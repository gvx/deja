#include "types.h"
#include "value.h"
#include "gc.h"
#include "stack.h"
#include <stdlib.h>
#include <string.h>

V int_to_value(int i)
{
	V t = new_value(T_NUM);
	t->color = Green;
	t->data.number = (double)i;
	return t;
}

V double_to_value(double d)
{
	V t = new_value(T_NUM);
	t->color = Green;
	t->data.number = d;
	return t;
}

V a_to_value(char* str)
{
	V t = new_value(T_STR);
	t->color = Green;
	String* s = malloc(sizeof(String));
	size_t l = strlen(str);
	s->length = l;
	char* str2 = malloc(l);
	memcpy(str2, str, l);
	s->data = str2;
	t->data.object = s;
	return t;
}

V str_to_value(int max, char* str)
{
	V t = new_value(T_STR);
	t->color = Green;
	String* s = malloc(sizeof(String));
	s->length = max;
	char* str2 = malloc(max);
	memcpy(str2, str, max);
	s->data = str2;
	t->data.object = s;
	return t;
}

V newlist(void)
{
	V t = new_value(T_STACK);
	Stack* s = newstack();
	t->data.object = s;
	return t;
}

bool truthy(V t)
{
	switch(t->type)
	{
		case T_NIL:
			return false;
		case T_NUM:
			return toNumber(t) != 0.0;
		case T_STR:
		case T_IDENT:
			return toString(t)->length > 0;
		case T_STACK:
			return toStack(t)->size > 0;
		default:
			return true;
	}	
}