#include "lib.h"

Error concat(Stack* S, Stack* scope_arr)
{
	String *s1;
	String *s2;
	int i;
	require(1);
	V v1 = popS();
	if (getType(v1) == T_STR)
	{
		require(1);
		V v2 = popS();
		if (getType(v2) != T_STR)
		{
			return TypeError;
		}
		s1 = toString(v1);
		s2 = toString(v2);
		char *new = malloc(s1->length + s2->length + 1);
		memcpy(new, toCharArr(s1), s1->length);
		memcpy(new + s1->length, toCharArr(s2), s2->length + 1);
		pushS(str_to_value(s1->length + s2->length, new));
		clear_ref(v1);
		clear_ref(v2);
		return Nothing;
	}
	else if (getType(v1) == T_LIST)
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
			newlength += toString(n[i])->length;
		}

		char *new = malloc(newlength + 1);
		char *currpoint = new;

		for (i = u - 1; i >= 0; i--)
		{
			s1 = toString(n[i]);
			memcpy(currpoint, toCharArr(s1), s1->length);
			currpoint += s1->length;
		}
		*currpoint = '\0';
		pushS(str_to_value(newlength, new));
		clear_ref(v1);
		return Nothing;
	}
	else if (getType(v1) == T_IDENT && v1 == get_ident("("))
	{
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
			newlength += toString(S->nodes[i])->length;
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
			s1 = toString(S->nodes[i]);
			memcpy(currpoint, toCharArr(s1), s1->length);
			currpoint += s1->length;
			clear_ref(popS());
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

Error contains(Stack* S, Stack* scope_arr)
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
	String *s1 = toString(v1);
	String *s2 = toString(v2);
	if (s1->length > s2->length)
	{
		pushS(add_ref(v_false));
	}
	else
	{
		uint32_t i;
		for (i = 0; i <= s2->length - s1->length; i++)
		{
			if (!memcmp(toCharArr(s2) + i, toCharArr(s1), s1->length))
			{
				pushS(add_ref(v_true));
				clear_ref(v1);
				clear_ref(v2);
				return Nothing;
			}
		}
		pushS(add_ref(v_false));
	}
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error starts_with(Stack* S, Stack* scope_arr)
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
	String *s1 = toString(v1);
	String *s2 = toString(v2);
	if (s1->length > s2->length || memcmp(toCharArr(s2), toCharArr(s1), s1->length))
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

Error ends_with(Stack* S, Stack* scope_arr)
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
	String *s1 = toString(v1);
	String *s2 = toString(v2);
	if (s1->length > s2->length || memcmp(toCharArr(s2) + s2->length - s1->length, toCharArr(s1), s1->length))
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

Error join(Stack* S, Stack* scope_arr)
{
	String *s1;
	String *s2;
	int i;
	require(2);
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_STR && getType(v2) == T_LIST)
	{
		s1 = toString(v1);
		int len = stack_size(toStack(v2));
		int newlength = s1->length * (len > 0 ? len - 1 : 0);
		for (i = toStack(v2)->used - 1; i >= 0; i--)
		{
			if (getType(toStack(v2)->nodes[i]) != T_STR)
			{
				clear_ref(v1);
				clear_ref(v2);
				return TypeError;
			}
			newlength += toString(toStack(v2)->nodes[i])->length;
		}

		char *new = malloc(newlength + 1);
		char *currpoint = new;

		for (i = toStack(v2)->used - 1; i >= 0; i--)
		{
			s2 = toString(toStack(v2)->nodes[i]);
			memcpy(currpoint, toCharArr(s2), s2->length);
			currpoint += s2->length;
			if (i > 0)
			{
				memcpy(currpoint, toCharArr(s1), s1->length);
				currpoint += s1->length;
			}
		}
		*currpoint = '\0';
		pushS(str_to_value(newlength, new));
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

Error split(Stack* S, Stack* scope_arr)
{
	String *s1;
	String *s2;
	require(2);
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_STR && getType(v2) == T_STR)
	{
		s1 = toString(v1);
		s2 = toString(v2);
		V r = new_list();
		Stack *rs = toStack(r);
		uint32_t start, laststart = 0;
		if (s1->length <= s2->length)
		{
			for (start = 0; start <= s2->length - s1->length; start++)
			{
				if (!memcmp(toCharArr(s1), toCharArr(s2) + start, s1->length))
				{
					V new = str_to_value(start - laststart, toCharArr(s2) + laststart);
					laststart = start + s1->length;
					push(rs, new);
				}
			}
		}
		push(rs, str_to_value(s2->length - laststart, toCharArr(s2) + laststart));
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

Error slice(Stack* S, Stack* scope_arr)
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
	String *string = toString(str);
	int len = string->length;
	if (s < -len)
		s = 0;
	else if (s < 0)
		s = len + s;
	else if (s > len)
		s = len;
	if (e <= 0)
		e = len + e;
	if (e < s)
		e = s;
	else if (e > len)
		e = len;
	pushS(str_to_value(e - s, toCharArr(string) + s));
	clear_ref(str);
	clear_ref(start);
	clear_ref(end);
	return Nothing;
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
	String *s = toString(v);
	if (s->length == 0)
	{
		clear_ref(v);
		return ValueError;
	}
	pushS(int_to_value((int)toCharArr(s)[0]));
	clear_ref(v);
	return Nothing;
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
	char x = toNumber(v);
	pushS(str_to_value(1, &x));
	clear_ref(v);
	return Nothing;
}

Error find(Stack* S, Stack* scope_arr)
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
	String *s1 = toString(v1);
	String *s2 = toString(v2);
	if (s1->length <= s2->length)
	{
		uint32_t i;
		for (i = 0; i <= s2->length - s1->length; i++)
		{
			if (!memcmp(toCharArr(s2) + i, toCharArr(s1), s1->length))
			{
				pushS(int_to_value(i));
				clear_ref(v1);
				clear_ref(v2);
				return Nothing;
			}
		}
	}
	pushS(int_to_value(-1));
	clear_ref(v1);
	clear_ref(v2);
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
	char *chrs = getChars(source);
	V list = new_list();
	Stack *st = toStack(list);
	int i;
	for (i = toString(source)->length - 1; i >= 0; i--)
	{
		push(st, str_to_value(1, chrs + i));
	}
	pushS(list);
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
	int haystack_len = toString(haystack)->length;
	int needle_len = toString(needle)->length;
	char *haystack_c = getChars(haystack);
	char *needle_c = getChars(needle);
	if (needle_len == 0)
	{
		pushS(int_to_value(haystack_len + 1));
		return Nothing;
	}
	if (needle_len > haystack_len)
	{
		pushS(int_to_value(0));
		return Nothing;
	}
	int ix;
	int count = 0;
	for (ix = 0; ix < haystack_len - needle_len + 1; ix++)
	{
		if (!memcmp(haystack_c + ix, needle_c, needle_len))
		{
			count++;
			ix += needle_len - 1;
		}
	}
	pushS(int_to_value(count));
	clear_ref(haystack);
	clear_ref(needle);
	return Nothing;
}

Error split_any(Stack* S, Stack* scope_arr)
{
	String *s1;
	String *s2;
	require(2);
	V v1 = popS();
	V v2 = popS();
	if (getType(v1) == T_STR && getType(v2) == T_STR)
	{
		s1 = toString(v1);
		s2 = toString(v2);
		V r = new_list();
		Stack *rs = toStack(r);
		uint32_t start, laststart = 0;
		for (start = 0; start < s2->length; start++)
		{
			if (memchr(toCharArr(s1), toCharArr(s2)[start], s1->length))
			{
				V new = str_to_value(start - laststart, toCharArr(s2) + laststart);
				laststart = start + 1;
				push(rs, new);
			}
		}
		push(rs, str_to_value(s2->length - laststart, toCharArr(s2) + laststart));
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
