#include "idents.h"
#include "value.h"
#include "types.h"
#include "gc.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static ITreeNode *ident_store = NULL;

ITreeNode *create_ident(size_t length, const char *data)
{
	V t = make_new_value(T_IDENT, true, sizeof(String) + length + 1);
	String *s = &((StrValue*)t)->s;
	s->length = length;
	memcpy((char*)t + sizeof(StrValue), data, length + 1);
	s->hash = string_hash(length, data);
	ITreeNode *new = malloc(sizeof(ITreeNode));
	new->ident = t;
	new->left = NULL;
	new->right = NULL;
	return new;
}

V lookup_ident_at(ITreeNode **loc, size_t length, const char *data)
{
	if (!*loc)
	{
		*loc = create_ident(length, data);
		return (*loc)->ident;
	}
	int cmp = memcmp(data, getChars((*loc)->ident), length);
	if (cmp == 0)
	{
		return (*loc)->ident;
	}
	else if (cmp < 0)
	{
		return lookup_ident_at(&(*loc)->left, length, data);
	}
	else
	{
		return lookup_ident_at(&(*loc)->right, length, data);
	}
}

V lookup_ident(size_t length, const const char *data)
{
	return lookup_ident_at(&ident_store, length, data);
}

int count_idents_at(ITreeNode *loc)
{
	if (!loc)
	{
		return 0;
	}
	return 1 + count_idents_at(loc->left) + count_idents_at(loc->right);
}

int ident_count()
{
	return count_idents_at(ident_store);
}
