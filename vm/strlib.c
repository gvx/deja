#include "lib.h"

Error concat(Header* h, Stack* S, Stack* scope_arr)
{
	String *s1;
	String *s2;
	require(1);
	V v1 = pop(S);
	if (v1->type == T_STR)
	{
		require(1);
		V v2 = pop(S);
		if (v2->type != T_STR)
		{
			return TypeError;
		}
		s1 = toString(v1);
		s2 = toString(v2);
		char *new = malloc(s1->length + s2->length + 1);
		memcpy(new, s1->data, s1->length);
		memcpy(new + s1->length, s2->data, s2->length + 1);
		push(S, str_to_value(s1->length + s2->length, new));
		clear_ref(v1);
		clear_ref(v2);
		return Nothing;
	}
	else if (v1->type == T_STACK)
	{
		int newlength = 0;
		int i;
		StackArray *n = toStack(v1)->head;
		while (n != NULL)
		{
			for (i = 0; i < n->numitems; i++)
			{
				if (n->items[i]->type != T_STR)
				{
					clear_ref(v1);
					return TypeError;
				}
				newlength += toString(n->items[i])->length;
			}
			n = n->next;
		}

		char *new = malloc(newlength + 1);
		char *currpoint = new;

		n = toStack(v1)->head;
		while (n != NULL)
		{
			for (i = 0; i < n->numitems; i++)
			{
				s1 = toString(n->items[i]);
				memcpy(currpoint, s1->data, s1->length);
				currpoint += s1->length;
			}
			n = n->next;
		}
		*currpoint = '\0';
		push(S, str_to_value(newlength, new));
		clear_ref(v1);
		return Nothing;
	}
	else
	{
		clear_ref(v1);
		return TypeError;
	}
}

Error contains(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type != T_STR || v2->type != T_STR)
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	String *s1 = toString(v1);
	String *s2 = toString(v2);
	if (s1->length > s2->length)
	{
		push(S, add_ref(v_false));
	}
	else
	{
		int i;
		for (i = 0; i <= s2->length - s1->length; i++)
		{
			if (!memcmp(s2->data + i, s1->data, s1->length))
			{
				push(S, add_ref(v_true));
				clear_ref(v1);
				clear_ref(v2);
				return Nothing;
			}
		}
		push(S, add_ref(v_false));
	}
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error starts_with(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type != T_STR || v2->type != T_STR)
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	String *s1 = toString(v1);
	String *s2 = toString(v2);
	if (s1->length > s2->length || memcmp(s2->data, s1->data, s1->length))
	{
		push(S, add_ref(v_false));
	}
	else
	{
		push(S, add_ref(v_true));
	}
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error ends_with(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type != T_STR || v2->type != T_STR)
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	String *s1 = toString(v1);
	String *s2 = toString(v2);
	if (s1->length > s2->length || memcmp(s2->data + s2->length - s1->length, s1->data, s1->length))
	{
		push(S, add_ref(v_false));
	}
	else
	{
		push(S, add_ref(v_true));
	}
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

Error join(Header* h, Stack* S, Stack* scope_arr)
{
	String *s1;
	String *s2;
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_STR && v2->type == T_STACK)
	{
		s1 = toString(v1);
		int len = stack_size(toStack(v2));
		int newlength = s1->length * (len > 0 ? len - 1 : 0);
		int i;
		StackArray *n = toStack(v1)->head;
		while (n != NULL)
		{
			for (i = 0; i < n->numitems; i++)
			{
				if (n->items[i]->type != T_STR)
				{
					clear_ref(v1);
					clear_ref(v2);
					return TypeError;
				}
				newlength += toString(n->items[i])->length;
			}
			n = n->next;
		}

		char *new = malloc(newlength + 1);
		char *currpoint = new;

		n = toStack(v2)->head;
		while (n != NULL)
		{
			for (i = 0; i < n->numitems; i++)
			{
				s2 = toString(n->items[i]);
				memcpy(currpoint, s2->data, s2->length);
				currpoint += s2->length;
				if (i < n->numitems - 1 || n->next != NULL)
				{
					memcpy(currpoint, s1->data, s1->length);
					currpoint += s1->length;
				}
			}
			n = n->next;
		}
		*currpoint = '\0';
		push(S, str_to_value(newlength, new));
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

Error split(Header* h, Stack* S, Stack* scope_arr)
{
	String *s1;
	String *s2;
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type == T_STR && v2->type == T_STR)
	{
		s1 = toString(v1);
		s2 = toString(v2);
		V r = new_list();
		Stack *rs = toStack(r);
		int start, laststart = 0;
		for (start = 0; start <= s2->length - s1->length; start++)
		{
			if (!memcmp(s1->data, s2->data + start, s1->length))
			{
				V new = str_to_value(start - laststart, s2->data + laststart);
				laststart = start + s1->length;
				push(rs, new);
			}
		}
		push(rs, str_to_value(s2->length - laststart, s2->data + laststart));
		push(S, r);
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

Error slice(Header* h, Stack* S, Stack* scope_arr)
{
	require(3);
	V str = pop(S);
	V start = pop(S);
	V end = pop(S);
	if (str->type != T_STR || start->type != T_NUM || end->type != T_NUM)
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
		e = len - e;
	if (e < s)
		e = s;
	else if (e > len)
		e = len;
	push(S, str_to_value(e - s, string->data + s));
	clear_ref(str);
	clear_ref(start);
	clear_ref(end);
	return Nothing;
}

Error ord(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	if (v->type != T_STR)
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
	push(S, int_to_value((int)s->data[0]));
	return Nothing;
}

Error chr(Header* h, Stack* S, Stack* scope_arr)
{
	require(1);
	V v = pop(S);
	if (v->type != T_NUM)
	{
		clear_ref(v);
		return TypeError;
	}
	char x = toNumber(v);
	push(S, str_to_value(1, &x));
	return Nothing;
}

Error find(Header* h, Stack* S, Stack* scope_arr)
{
	require(2);
	V v1 = pop(S);
	V v2 = pop(S);
	if (v1->type != T_STR || v2->type != T_STR)
	{
		clear_ref(v1);
		clear_ref(v2);
		return TypeError;
	}
	String *s1 = toString(v1);
	String *s2 = toString(v2);
	if (s1->length > s2->length)
	{
		push(S, int_to_value(-1));
	}
	else
	{
		int i;
		for (i = 0; i <= s2->length - s1->length; i++)
		{
			if (!memcmp(s2->data + i, s1->data, s1->length))
			{
				push(S, int_to_value(i));
				clear_ref(v1);
				clear_ref(v2);
				return Nothing;
			}
		}
		push(S, int_to_value(-1));
	}
	clear_ref(v1);
	clear_ref(v2);
	return Nothing;
}

