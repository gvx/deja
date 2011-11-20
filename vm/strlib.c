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
		Node *n = toStack(v1)->head;
		while (n != NULL)
		{
			if (n->data->type != T_STR)
			{
				clear_ref(v1);
				return TypeError;
			}
			newlength += toString(n->data)->length;
			n = n->next;
		}

		char *new = malloc(newlength + 1);
		char *currpoint = new;

		n = toStack(v1)->head;
		while (n != NULL)
		{
			s1 = toString(n->data);
			memcpy(currpoint, s1->data, s1->length);
			currpoint += s1->length;
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
