#include "idents.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static ITreeNode *ident_store = NULL;

ITreeNode *create_ident(size_t length, const char *data)
{
	ITreeNode *new = malloc(sizeof(ITreeNode) + length);
	new->type = T_IDENT;
	new->length = length;
	memcpy(new->data, data, length + 1);
	new->left = NULL;
	new->right = NULL;
	return new;
}

V lookup_ident_at(ITreeNode **loc, size_t length, const char *data)
{
	if (!*loc)
	{
		*loc = create_ident(length, data);
		return (V)(*loc);
	}
	int cmp = memcmp(data, (*loc)->data, length <= (*loc)->length ? length : (*loc)->length);
	if (!cmp)
	{ // to prevent :he to compare equal to :hello
		cmp = length - (*loc)->length;
	}
	if (cmp == 0)
	{
		return (V)(*loc);
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

int ident_depth_at(ITreeNode *loc)
{
	if (!loc)
	{
		return 0;
	}
	int ldepth = ident_depth_at(loc->left);
	int rdepth = ident_depth_at(loc->right);
	return 1 + (ldepth > rdepth ? ldepth : rdepth);
}

int ident_depth()
{
	return ident_depth_at(ident_store);
}
