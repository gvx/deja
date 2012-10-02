#include "strings.h"
#include "types.h"
#include "gc.h"

#include <string.h>

utf8index nextchar(utf8 chars, utf8index start)
{
	decode_codepoint(chars, &start);
	return start;
}

uint32_t new_string_hash(size_t length, const char *key)
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

size_t count_characters(size_t size, const utf8 chars)
{
	utf8index p = 0;
	size_t c = 0;
	while ((p = nextchar(chars, p)) <= size)
	{
		c++;
	}
	return c;
}

V charat(utf8 source, utf8index curr)
{
	return strslice(source, curr, nextchar(source, curr));
}

V strslice(utf8 source, utf8index from, utf8index to)
{
	size_t len = to >= from ? to - from : 0;
	return str_to_string(len, source + from);
}

V str_to_string(size_t max, char *str)
{
	V t = make_new_value(T_STR, true, sizeof(NewString) + max);
	NewString *s = toNewString(t);
	s->size = max;
	s->hash = 0;
	s->length = -1;
	memcpy(s->text, str, max);
	s->text[max] = '\0';
	return t;
}

V a_to_string(char* str)
{
	size_t size = strlen(str);
	V t = make_new_value(T_STR, true, sizeof(NewString) + size);
	NewString *s = toNewString(t);
	s->size = size;
	s->hash = 0;
	s->length = -1;
	memcpy(s->text, str, size + 1);
	return t;
}

V empty_string_to_value(size_t max, utf8 *adr)
{
	V t = make_new_value(T_STR, true, sizeof(NewString) + max);
	NewString *s = toNewString(t);
	s->size = max;
	s->hash = 0;
	s->length = -1;
	s->text[max] = '\0';
	*adr = s->text;
	return t;
}

uint32_t need_hash(V string)
{
	NewString *s = toNewString(string);
	if (s->hash == 0)
	{
		s->hash = new_string_hash(s->size, s->text);
	}
	return s->hash;
}

uint32_t string_length(V string)
{
	NewString *s = toNewString(string);
	if (s->length == (uint32_t)-1)
	{
		s->length = count_characters(s->size, s->text);
	}
	return s->length;
}

Error new_ord(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_STR)
	{
		clear_ref(v);
		return TypeError;
	}
	NewString *s = toNewString(v);
	if (s->size == 0)
	{
		clear_ref(v);
		return ValueError;
	}
	utf8index n = 0;
	pushS(int_to_value(decode_codepoint(s->text, &n)));
	clear_ref(v);
	return Nothing;
}

Error new_chr(Stack* S, Stack* scope_arr)
{
	require(1);
	V v = popS();
	if (getType(v) != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	unichar c = toNumber(v);
	utf8 *adr = NULL;
	V r = empty_string_to_value(codepoint_length(c), adr);
	encode_codepoint(c, *adr);
	pushS(r);
	clear_ref(v);
	return Nothing;
}
