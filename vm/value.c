#include "stack.h"
#include "hashmap.h"
#include "idents.h"
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
	String *s = toString(t);
	s->length = l;
	memcpy(toCharArr(s), str, l + 1);
	s->hash = string_hash(l, str);
	return t;
}

V str_to_value(int max, char* str)
{
	V t = make_new_value(T_STR, true, sizeof(String) + max + 1);
	String *s = toString(t);
	s->length = max;
	memcpy(toCharArr(s), str, max);
	max[toCharArr(s)] = '\0';
	s->hash = string_hash(max, str);
	return t;
}

V empty_str_to_value(int max, char **adr)
{
	V t = make_new_value(T_STR, true, sizeof(String) + max + 1);
	String *s = toString(t);
	s->length = max;
	*adr = toCharArr(s);
	max[toCharArr(s)] = '\0';
	s->hash = 0;
	return t;
}

V get_ident(const char* name)
{
	return lookup_ident(strlen(name), name);
}

V new_list(void)
{
	V t = make_new_value(T_STACK, false, sizeof(Stack));
	toStack(t)->size = 0;
	toStack(t)->used = 0;
	toStack(t)->nodes = NULL;
	return t;
}

V new_sized_dict(int size)
{
	V t = make_new_value(T_DICT, false, sizeof(HashMap));
	hashmap_from_value(t, size);
	return t;
}

V new_pair(V first, V second)
{
	V t = make_new_value(T_PAIR, true, sizeof(V) * 2);
	toFirst(t) = first;
	toSecond(t) = second;
	return t;
}

long int pair_ordinal(V p)
{
	if (getType(p) != T_PAIR)
	{
		return 0;
	}
	int o1 = pair_ordinal(toFirst(p));
	int o2 = pair_ordinal(toSecond(p));
	return (o1 > o2 ? o1 : o2) + 1;
}

unsigned long int gcd(unsigned long int u, unsigned long int v)
{
  // simple cases (termination)
  if (u == v)
    return u;
  if (u == 0)
    return v;
  if (v == 0)
    return u;

  // look for factors of 2
  if (~u & 1) // u is even
  {
    if (v & 1) // v is odd
      return gcd(u >> 1, v);
    else // both u and v are even
      return gcd(u >> 1, v >> 1) << 1;
  }
  if (~v & 1) // u is odd, v is even
    return gcd(u, v >> 1);

  // reduce larger argument
  if (u > v)
    return gcd((u - v) >> 1, v);
  return gcd((v - u) >> 1, u);
}

V new_frac(long int numerator, long int denominator)
{
	if (numerator == 0)
		return intToV(0);
	long int m = 1;
	if (numerator < 0)
	{
		numerator = -numerator;
		m = -m;
	}
	if (denominator < 0)
	{
		denominator = -denominator;
		m = -m;
	}
	long int divisor = gcd(numerator, denominator);
	numerator = m * (numerator / divisor);
	denominator = denominator / divisor;
	if (denominator == 1)
		return int_to_value(numerator);
	V t = make_new_value(T_FRAC, true, sizeof(Frac));
	toNumerator(t) = numerator;
	toDenominator(t) = denominator;
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
			return toString(t)->length > 0;
		case T_STACK:
			return toStack(t)->size > 0;
		case T_DICT:
			return toHashMap(t)->used > 0;
		default:
			return true;
	}
}

bool equal(V v1, V v2)
{
	if (v1 == v2) //identical objects
	{
		return true;
	}
	else if (getType(v1) == getType(v2))
	{
		if (getType(v1) == T_NUM)
		{
			return toNumber(v1) == toNumber(v2);
		}
		else if (getType(v1) == T_STR)
		{
			String* s1 = toString(v1);
			String* s2 = toString(v2);
			if (s1->length == s2->length)
			{
				return !memcmp(toCharArr(s1), toCharArr(s2), s1->length);
			}
		}
		else if (getType(v1) == T_PAIR)
		{
			// pairs count as simple datatypes, so this is safe
			return equal(toFirst(v1), toFirst(v2)) && equal(toSecond(v1), toSecond(v2));
		}
		else if (getType(v1) == T_FRAC)
		{
			return toNumerator(v1) == toNumerator(v2) &&
				toDenominator(v1) == toDenominator(v2);
		}
	}
	return false;
}
