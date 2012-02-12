#include "types.h"
#include "value.h"
#include "gc.h"
#include "stack.h"
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define make_value_from_double(d) \
	V t = make_new_value(T_NUM, true, sizeof(double)); \
	toDouble(t) = d; \
	return t;

V int_to_value(long int i)
{
	if (canBeInt(i))
	{
		return intToV(i);
	}
	make_value_from_double((double)i);
}

V double_to_value(double d)
{
	if (!fmod(d, 1.0) && canBeInt(d))
	{
		return intToV((long int)d);
	}
	make_value_from_double(d);
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
}

V a_to_value(char* str)
{
	size_t l = strlen(str);
	V t = make_new_value(T_STR, true, sizeof(String) + l + 1);
	String *s = &((StrValue*)t)->s;
	s->length = l;
	memcpy((char*)t + sizeof(StrValue), str, l + 1);
	s->hash = string_hash(l, str);
	return t;
}

V str_to_value(int max, char* str)
{
	V t = make_new_value(T_STR, true, sizeof(String) + max + 1);
	String *s = &((StrValue*)t)->s;
	s->length = max;
	memcpy((char*)t + sizeof(StrValue), str, max + 1);
	max[(char*)t + sizeof(StrValue)] = '\0';
	s->hash = string_hash(max, str);
	return t;
}

V empty_str_to_value(int max, char **adr)
{
	V t = make_new_value(T_STR, true, sizeof(String) + max + 1);
	String *s = &((StrValue*)t)->s;
	s->length = max;
	*adr = (char*)(s + 1);
	max[(char*)t + sizeof(StrValue)] = '\0';
	s->hash = 0;
	return t;
}

V get_ident(const char* name)
{
	size_t l = strlen(name);
	V t = make_new_value(T_IDENT, true, sizeof(String) + l + 1);
	String *s = &((StrValue*)t)->s;
	s->length = l;
	memcpy((char*)t + sizeof(StrValue), name, l + 1);
	s->hash = string_hash(l, name);
	return t;
}

V new_list(void)
{
	V t = make_new_value(T_STACK, false, sizeof(Stack));
	toStack(t)->size = 0;
	toStack(t)->head = NULL;
	return t;
}

V new_sized_dict(int size)
{
	V t = make_new_value(T_DICT, false, sizeof(HashMap));
	hashmap_from_value(t, size);
	return t;
}

bool truthy(V t)
{
	switch(getType(t))
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
