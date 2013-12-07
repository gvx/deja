#include "strings.h"
#include "types.h"
#include "gc.h"

#include <string.h>

utf8index nextchar(utf8 chars, utf8index start)
{
	decode_codepoint(chars, &start);
	return start;
}

// MurmerHash3
// the original implementation is in the public domain
uint32_t new_string_hash(const size_t length, const char *key)
{
    uint32_t hash = 1; /* randomize seed? */
    size_t index, k;

    for (index = 0; index + 3 < length; index += 4)
    {
        k = *(uint32_t*)(key + index);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> (32 - 15));
        k *= 0x1b873593;
        hash ^= k;
        hash = (hash << 13) | (hash >> (32 - 13));
        hash = hash * 5 + 0xe6546b64;
    }

    k = 0;
    switch (length & 3) {
        case 3:
            k ^= *(uint8_t*)(key + index + 2) << 16;
        case 2:
            k ^= *(uint8_t*)(key + index + 1) << 8;
        case 1:
            k ^= *(uint8_t*)(key + index);
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> (32 - 15));
            k *= 0x1b873593;
            hash ^= k;
    }

    hash ^= length;

    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);
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

uint32_t string_length(NewString *s)
{
	if (s->length == -1)
	{
		s->length = count_characters(s->size, s->text);
	}
	return s->length;
}

Error ord(Stack* S, Stack* scope_arr)
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
	utf8 adr = NULL;
	V r = empty_string_to_value(codepoint_length(c), &adr);
	toNewString(r)->length = 1;
	encode_codepoint(c, adr);
	return r;
}

Error chr(Stack* S, Stack* scope_arr)
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

Error chars(Stack* S, Stack* scope_arr)
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
	size_t size = string_length(toNewString(source));
	size_t i;
	for (i = 0; i < size; i++)
	{
		push(st, unichar_to_value(decode_codepoint(chrs, &index)));
	}
	pushS(list);
	clear_ref(source);
	return Nothing;
}

Error starts_with(Stack* S, Stack* scope_arr)
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

Error ends_with(Stack* S, Stack* scope_arr)
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

Error contains(Stack* S, Stack* scope_arr)
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
	if (string_length(needle_s) > string_length(haystack_s))
	{
		pushS(add_ref(v_false));
	}
	else
	{
		uint32_t i;
		utf8index index = 0;
		for (i = 0; i <= string_length(haystack_s) - string_length(needle_s); i++)
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

Error count(Stack* S, Stack* scope_arr)
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
		pushS(int_to_value(string_length(toNewString(haystack)) + 1));
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

Error find(Stack* S, Stack* scope_arr)
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
	if (string_length(needle_s) <= string_length(haystack_s))
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

Error concat(Stack *S, Stack *scope_arr) /* concat( "a" "b" "c" ) */
{
	NewString *s1;
	int i;
	require(1);
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
	pushS(str_to_string(newlength, new));
	free(new);
	return Nothing;
}

Error concat_list(Stack *S, Stack *scope_arr) /* concat [ "a" "b" "c" ] */
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
		pushS(str_to_string(newlength, new));

		free(new);
		clear_ref(v1);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		return TypeError;
	}
}

Error join(Stack *S, Stack *scope_arr)
{
	NewString *s1;
	NewString *s2;
	int i;
	require(2);
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_STR && getType(v2) == T_LIST)
	{
		s1 = toNewString(v1);
		int len = stack_size(toStack(v2));
		int newlength = s1->size * (len > 0 ? len - 1 : 0);
		for (i = toStack(v2)->used - 1; i >= 0; i--)
		{
			if (getType(toStack(v2)->nodes[i]) != T_STR)
			{
				clear_ref(v1);
				clear_ref(v2);
				return TypeError;
			}
			newlength += toNewString(toStack(v2)->nodes[i])->size;
		}

		utf8 currpoint;
		V new = empty_string_to_value(newlength, &currpoint);

		for (i = toStack(v2)->used - 1; i >= 0; i--)
		{
			s2 = toNewString(toStack(v2)->nodes[i]);
			memcpy(currpoint, s2->text, s2->size);
			currpoint += s2->size;
			if (i > 0)
			{
				memcpy(currpoint, s1->text, s1->size);
				currpoint += s1->size;
			}
		}
		pushS(new);
		clear_ref(v1);
		clear_ref(v2);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error split(Stack *S, Stack *scope_arr)
{
	NewString *s1;
	NewString *s2;
	require(2);
	V v2 = popS();
	V v1 = popS();
	if (getType(v1) == T_STR && getType(v2) == T_STR)
	{
		s1 = toNewString(v1);
		s2 = toNewString(v2);
		V r = new_list();
		Stack *rs = toStack(r);
		utf8index start, laststart = 0;
		if (s1->size <= s2->size)
		{
			for (start = 0; start <= s2->size - s1->size;
			     start = nextchar(s2->text, start))
			{
				if (!memcmp(s1->text, s2->text + start, s1->size))
				{
					V new = str_to_string(start - laststart, s2->text + laststart);
					laststart = start + string_length(s1);
					push(rs, new);
				}
			}
		}
		push(rs, str_to_string(s2->size - laststart, s2->text + laststart));
		reverse(rs);
		pushS(r);
		clear_ref(v1);
		clear_ref(v2);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error slice(Stack *S, Stack *scope_arr)
{
	require(3);
	V str = popS();
	V start = popS();
	V end = popS();
	if (getType(str) != T_STR || getType(start) != T_NUM || getType(end) != T_NUM)
	{
		clear_ref(str);
		clear_ref(start);
		clear_ref(end);
		return TypeError;
	}
	int s = toNumber(start);
	int e = toNumber(end);
	NewString *string = toNewString(str);
	int len = string_length(string);
	if (s < -len)
		s = 0;
	else if (s < 0)
		s = len + s;
	else if (s > len)
		s = len;
	if (e < 0)
		e = len + e;
	else if (e > len)
		e = len;
	utf8index si = 0;
	e -= s;
	while (s--) si = nextchar(string->text, si);
	utf8index ei = si;
	while (e--) ei = nextchar(string->text, ei);
	pushS(strslice(string->text, si, ei));
	clear_ref(str);
	clear_ref(start);
	clear_ref(end);
	return Nothing;
}

Error split_any(Stack *S, Stack *scope_arr)
{
	NewString *s1;
	NewString *s2;
	require(2);
	V v2 = popS();
	V v1 = popS();
	if (getType(v1) == T_STR && getType(v2) == T_STR)
	{
		s1 = toNewString(v1);
		s2 = toNewString(v2);
		V r = new_list();
		Stack *rs = toStack(r);
		uint32_t start, laststart = 0;
		for (start = 0; start < s2->size; start = nextchar(s2->text, start))
		{
			if (memchr(s1->text, s2->text[start], s1->size))
			{
				V new = str_to_string(start - laststart, s2->text + laststart);
				laststart = start + 1;
				push(rs, new);
			}
		}
		push(rs, str_to_string(s2->size - laststart, s2->text + laststart));
		reverse(rs);
		pushS(r);
		clear_ref(v1);
		clear_ref(v2);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
}

Error is_digit(Stack *S, Stack *scope_arr)
{
	NewString *s;
	require(1);
	V v = popS();
	if (getType(v) == T_STR)
	{
		s = toNewString(v);
		uint32_t i;
		V rval = v_true;
		for (i = 0; i < s->size; i++)
		{
			// naive method works fine
			// because everything uses UTF-8
			if (s->text[i] > '9' || s->text[i] < '0')
			{
				rval = v_false;
				break;
			}
		}
		pushS(add_ref(rval));
		clear_ref(v);
		return Nothing;
	}
	else
	{
		clear_ref(v);
		return TypeError;
	}
}
