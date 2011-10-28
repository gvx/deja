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

uint32_t string_hash(int length, const char *key)
{
	uint32_t hash = 2166136261;
	int i;
	for (i = 0; i < length; i++)
	{
        hash = (16777619 * hash) ^ (*key);
        key++;
	}
	return hash;
};

V a_to_value(char* str)
{
	V t = new_value(T_STR);
	t->color = Green;
	String* s = malloc(sizeof(String));
	size_t l = strlen(str);
	s->length = l;
	char* str2 = malloc(l);
	memcpy(str2, str, l);
	s->hash = string_hash(l, str2);
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
	s->hash = string_hash(max, str2);
	s->data = str2;
	t->data.object = s;
	return t;
}

V get_ident(const char* name)
{
	V t = new_value(T_IDENT);
	t->color = Green;
	String* s = malloc(sizeof(String));
	size_t l = strlen(name);
	s->length = l;
	char* name2 = malloc(l);
	memcpy(name2, name, l + 1);
	s->hash = string_hash(l, name2);
	s->data = name2;
	t->data.object = s;
	return t;
}

V new_list(void)
{
	V t = new_value(T_STACK);
	Stack* s = new_stack();
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