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
	size_t i;
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
	V haystack = popS();
	V needle = popS();
	if (getType(needle) != T_STR || getType(haystack) != T_STR)
	{
		clear_ref(needle);
		clear_ref(haystack);
		return TypeError;
	}
	NewString *needle_s = toNewString(needle);
	NewString *haystack_s = toNewString(haystack);
	if (needle_s->size > haystack_s->size ||
	    memcmp(haystack_s->text, needle_s->text, needle_s->size))
	{
		pushS(add_ref(v_false));
	}
	else
	{
		pushS(add_ref(v_true));
	}
	clear_ref(needle);
	clear_ref(haystack);
	return Nothing;
}

Error new_ends_with(Stack* S, Stack* scope_arr)
{
	require(2);
	V haystack = popS();
	V needle = popS();
	if (getType(needle) != T_STR || getType(haystack) != T_STR)
	{
		clear_ref(needle);
		clear_ref(haystack);
		return TypeError;
	}
	NewString *needle_s = toNewString(needle);
	NewString *haystack_s = toNewString(haystack);
	if (needle_s->size > haystack_s->size ||
	    memcmp(haystack_s->text + haystack_s->size - needle_s->size,
	           needle_s->text, needle_s->size))
	{
		pushS(add_ref(v_false));
	}
	else
	{
		pushS(add_ref(v_true));
	}
	clear_ref(needle);
	clear_ref(haystack);
	return Nothing;
}

Error new_contains(Stack* S, Stack* scope_arr)
{
	require(2);
	V haystack = popS();
	V needle = popS();
	if (getType(needle) != T_STR || getType(haystack) != T_STR)
	{
		clear_ref(needle);
		clear_ref(haystack);
		return TypeError;
	}
	NewString *needle_s = toNewString(needle);
	NewString *haystack_s = toNewString(haystack);
	if (needle_s->length > haystack_s->length)
	{
		pushS(add_ref(v_false));
	}
	else
	{
		uint32_t i;
		utf8index index = 0;
		for (i = 0; i <= haystack_s->length - needle_s->length; i++)
		{
			if (!memcmp(haystack_s->text + index, needle_s->text, needle_s->size))
			{
				pushS(add_ref(v_true));
				clear_ref(needle);
				clear_ref(haystack);
				return Nothing;
			}
			index = nextchar(haystack_s->text, index);
		}
		pushS(add_ref(v_false));
	}
	clear_ref(needle);
	clear_ref(haystack);
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
	size_t haystack_len = toNewString(haystack)->size;
	size_t needle_len = toNewString(needle)->size;
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

Error new_find(Stack* S, Stack* scope_arr)
{
	require(2);
	V haystack = popS();
	V needle = popS();
	if (getType(needle) != T_STR || getType(haystack) != T_STR)
	{
		clear_ref(needle);
		clear_ref(haystack);
		return TypeError;
	}
	NewString *needle_s = toNewString(needle);
	NewString *haystack_s = toNewString(haystack);
	if (needle_s->length <= haystack_s->length)
	{
		size_t haystack_len = haystack_s->size;
		size_t needle_len = needle_s->size;
		utf8 haystack_c = haystack_s->text;
		utf8 needle_c = needle_s->text;
		utf8index ix;
		int i = 0;
		for (ix = 0; ix < haystack_len - needle_len + 1; ix = nextchar(haystack_c, ix))
		{
			if (!memcmp(haystack_c + ix, needle_c, needle_len))
			{
				pushS(int_to_value(i));
				clear_ref(needle);
				clear_ref(haystack);
				return Nothing;
			}
			i++;
		}
	}
	pushS(int_to_value(-1));
	clear_ref(needle);
	clear_ref(haystack);
	return Nothing;
}

Error new_concat(Stack *S, Stack *scope_arr) /* concat( "a" "b" "c" ) */
{
	NewString *s1;
	int i;
	require(1);
	V v1 = popS();
	int newlength = 0;
	for (i = S->used - 1; i >= 0; i--)
	{
		int t = getType(S->nodes[i]);
		if (t == T_IDENT && S->nodes[i] == get_ident(")"))
		{
			break;
		}
		else if (t != T_STR)
		{
			clear_ref(v1);
			return TypeError;
		}
		newlength += toNewString(S->nodes[i])->size;
	}

	char *new = malloc(newlength + 1);
	char *currpoint = new;

	for (i = S->used - 1; i >= 0; i--)
	{
		if (getType(S->nodes[i]) == T_IDENT && S->nodes[i] == get_ident(")"))
		{
			clear_ref(popS());
			break;
		}
		s1 = toNewString(S->nodes[i]);
		memcpy(currpoint, s1->text, s1->size);
		currpoint += s1->size;
		clear_ref(popS());
	}
	*currpoint = '\0';
	pushS(str_to_value(newlength, new));
	clear_ref(v1);
	return Nothing;
}

Error new_concat_list(Stack *S, Stack *scope_arr) /* concat [ "a" "b" "c" ] */
{
	NewString *s1;
	int i;
	require(1);
	V v1 = popS();
	if (getType(v1) == T_LIST)
	{
		int newlength = 0;
		int u = toStack(v1)->used;
		V *n = toStack(v1)->nodes;
		for (i = u - 1; i >= 0; i--)
		{
			if (getType(n[i]) != T_STR)
			{
				clear_ref(v1);
				return TypeError;
			}
			newlength += toNewString(n[i])->size;
		}

		char *new = malloc(newlength + 1);
		char *currpoint = new;

		for (i = u - 1; i >= 0; i--)
		{
			s1 = toNewString(n[i]);
			memcpy(currpoint, s1->text, s1->size);
			currpoint += s1->size;
		}
		*currpoint = '\0';
		pushS(str_to_value(newlength, new));
		clear_ref(v1);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		return TypeError;
	}
}
