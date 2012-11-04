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
	size_t i;
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

V unichar_to_value(unichar c)
{
	utf8 *adr = NULL;
	V r = empty_string_to_value(codepoint_length(c), adr);
	toNewString(r)->length = 1;
	encode_codepoint(c, *adr);
	return r;
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
	pushS(unichar_to_value(toNumber(v)));
	clear_ref(v);
	return Nothing;
}

Error new_chars(Stack* S, Stack* scope_arr)
{
	require(1);
	V source = popS();
	if (getType(source) != T_STR)
	{
		clear_ref(source);
		return TypeError;
	}
	utf8 chrs = toNewString(source)->text;
	utf8index index = 0;
	V list = new_list();
	Stack *st = toStack(list);
	size_t size = toNewString(source)->size;
	int i;
	for (i = 0; i < size; i++)
	{
		push(st, unichar_to_value(decode_codepoint(chrs, &index)));
	}
	pushS(list);
	return Nothing;
}

Error new_starts_with(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) != T_STR || getType(v2) != T_STR)
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	NewString *s1 = toNewString(v1);
	NewString *s2 = toNewString(v2);
	if (s1->size > s2->size || memcmp(s2->text, s1->text, s1->size))
	{
		pushS(add_ref(v_false));
	}
	else
	{
		pushS(add_ref(v_true));
	}
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error new_ends_with(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) != T_STR || getType(v2) != T_STR)
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	NewString *s1 = toNewString(v1);
	NewString *s2 = toNewString(v2);
	if (s1->size > s2->size || memcmp(s2->text + s2->size - s1->size, s1->text, s1->size))
	{
		pushS(add_ref(v_false));
	}
	else
	{
		pushS(add_ref(v_true));
	}
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error new_contains(Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) != T_STR || getType(v2) != T_STR)
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	NewString *s1 = toNewString(v1);
	NewString *s2 = toNewString(v2);
	if (s1->length > s2->length)
	{
		pushS(add_ref(v_false));
	}
	else
	{
		uint32_t i;
		utf8index index = 0;
		for (i = 0; i <= s2->length - s1->length; i++)
		{
			if (!memcmp(s2->text + index, s1->text, s1->size))
			{
				pushS(add_ref(v_true));
				clear_ref(v1);
				clear_ref(v2);
				return Nothing;
			}
			index = nextchar(s2->text, index);
		}
		pushS(add_ref(v_false));
	}
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error new_count(Stack* S, Stack* scope_arr)
{
	require(2);
	V haystack = popS();
	V needle = popS();
	if (getType(haystack) != T_STR || getType(needle) != T_STR)
	{
		clear_ref(haystack);
		clear_ref(needle);
		return TypeError;
	}
	int haystack_len = toNewString(haystack)->size;
	int needle_len = toNewString(needle)->size;
	if (needle_len == 0)
	{
		pushS(int_to_value(toNewString(haystack)->length + 1));
		clear_ref(haystack);
		clear_ref(needle);
		return Nothing;
	}

	utf8 haystack_c = toNewString(haystack)->text;
	utf8 needle_c = toNewString(needle)->text;
	utf8index ix;
	int count = 0;
	for (ix = 0; ix < haystack_len - needle_len + 1; )
	{
		if (!memcmp(haystack_c + ix, needle_c, needle_len))
		{
			count++;
			ix += needle_len;
		}
		else
		{
			ix = nextchar(haystack_c, ix);
		}
	}

	pushS(int_to_value(count));
	clear_ref(haystack);
	clear_ref(needle);
	return Nothing;
}
