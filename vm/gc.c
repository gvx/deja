#include "stack.h"
#include "scope.h"
#include "func.h"
#include "file.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <assert.h>

#define MAX_ROOTS 1024

static int root_size = 0;
static V roots[MAX_ROOTS];

void collect_cycles(void);

V make_new_value(int type, bool simple, int size)
{
	V t = malloc(sizeof(Value) + size);
	t->buffered = false;
	t->type = type;
	t->refs = 1;
	t->baserefs = 0;
	t->color = simple ? Green : Black;
	return t;
}

V add_ref(V t)
{
	if (isInt(t) || t->type == T_IDENT)
		return t;

	t->refs++;
	if (t->color != Green)
	{
		t->color = Black;
	}
	return t;
}

V add_base_ref(V t)
{
	if (isInt(t) || t->type == T_IDENT)
		return t;

	t->baserefs++;

	return add_ref(t);
}

V add_rooted(V t)
{
	if (isInt(t) || t->type == T_IDENT)
		return t;

	t->baserefs++;

	return t;
}

void free_value(V t)
{
	Scope* sc;
	Bucket* b;
	Bucket* bb;
	File* f;
	HashMap* hm;
	int n;
	switch (getType(t))
	{
		case T_STR:
		case T_IDENT:
		case T_FUNC:
			break;
		case T_STACK:
			free(toStack(t)->nodes);
			break;
		case T_DICT:
			hm = toHashMap(t);
			if (hm->map != NULL)
			{
				for (n = 0; n < hm->size; n++)
				{
					b = hm->map[n];
					while(b != NULL)
					{
						bb = b;
						b = b->next;
						free(bb);
					}
				}
				free(hm->map);
			}
			break;
		case T_SCOPE:
			sc = toScope(t);
			if (sc->hm.map != NULL)
			{
				for (n = 0; n < sc->hm.size; n++)
				{
					b = sc->hm.map[n];
					while(b != NULL)
					{
						bb = b;
						b = b->next;
						free(bb);
					}
				}
				free(sc->hm.map);
			}
			if (sc->index > 0)
			{
				if (MAXSCOPE == sc->index)
				{
					sc->index = 0;
					while (SCOPECACHE[--MAXSCOPE].sc.index == 0)
					{
						if (MAXSCOPE == 0)
						{
							MAXSCOPE--;
							break;
						}
					}
					MAXSCOPE++;
				}
				sc->index = 0;
				return;
			}
			break;
		case T_FILE:
			f = toFile(t);
			free(f->header.literals);
			free(f->code);
			break;
	}
	free(t);
}

void iter_children(V t, void (*iter)(V))
{
	if (isInt(t) || t->type == T_IDENT)
		return;

	Stack* s;
	Scope* sc;
	Bucket* b;
	File* f;
	HashMap* hm;
	V child;
	int i;
	switch (getType(t))
	{
		case T_FUNC:
			child = toFunc(t)->defscope;
			if (child && toScope(child)->parent)
			{ // do not iterate over the global scope, as it cannot be collected
				iter(child);
			}
			break;
		case T_STACK:
			s = toStack(t);
			for (i = 0; i < s->used; i++)
			{
				iter(s->nodes[i]);
			}
			break;
		case T_DICT:
			hm = toHashMap(t);
			if (hm->map != NULL)
			{
				for (i = 0; i < hm->size; i++)
				{
					b = hm->map[i];
					while(b != NULL)
					{
						iter(b->key);
						iter(b->value);
						b = b->next;
					}
				}
			}
			break;
		case T_PAIR:
			iter(toFirst(t));
			iter(toSecond(t));
			break;
		case T_SCOPE:
			sc = toScope(t);
			if (sc->parent && toScope(sc->parent)->parent)
			{ // do not iterate over the global scope, as it cannot be collected
				iter(sc->parent);
			}
			if (sc->func)
			{
				iter(sc->func);
			}
			if (sc->hm.map != NULL)
			{
				for (i = 0; i < sc->hm.size; i++)
				{
					b = sc->hm.map[i];
					while(b != NULL)
					{
						iter(b->key);
						iter(b->value);
						b = b->next;
					}
				}
			}
			break;
		case T_FILE:
			f = toFile(t);
			for (i = 0; (unsigned)i < f->header.n_literals; i++)
			{
				iter(f->header.literals[i]);
			}
			iter(f->name);
			break;
	}
}

void release_value(V t)
{
	iter_children(t, clear_ref);
	t->color = Black;
	if (!t->buffered)
	{
		free_value(t);
	}
}

void possible_root(V t)
{
	if (t->color != Purple)
	{
		t->color = Purple;
		if (!t->buffered)
		{
			t->buffered = true;
			roots[root_size++] = t;
			if (root_size >= MAX_ROOTS)
			{
				collect_cycles();
			}
		}
	}
}

void mark_gray(V);

void mark_gray_child(V child)
{
	if (isInt(child) || child->type == T_IDENT)
		return;

	if (child != NULL)
	{
		child->refs--;
		mark_gray(child);
	}
}

void mark_gray(V t)
{
	if (t->color != Gray)
	{
		t->color = Gray;
		iter_children(t, mark_gray_child);
	}
}

void mark_roots(void)
{
	int i;
	V t;
	for (i = 0; i < root_size; i++)
	{
		t = roots[i];
		if (t->color == Purple)
		{
			mark_gray(t);
		}
		else
		{
			t->buffered = false;
			roots[i] = NULL;
			if (t->color == Black && t->refs == 0)
			{
				free_value(t);
			}
		}
	}
}

void scan_black(V);

void scan_black_child(V child)
{
	if (isInt(child) || child->type == T_IDENT)
		return;

	if (child != NULL)
	{
		child->refs++;
		if (child->color != Black)
		{
			scan_black(child);
		}
	}
}

void scan_black(V t)
{
	t->color = Black;

	iter_children(t, scan_black_child);
}

void scan(V t)
{
	if (isInt(t) || t->type == T_IDENT)
		return;

	if (t->color == Gray)
	{
		if (t->refs > 0)
		{
			scan_black(t);
		}
		else
		{
			t->color = White;
			iter_children(t, scan);
		}
	}
}

void scan_roots(void)
{
	int i;
	for (i = 0; i < root_size; i++)
	{
		if (roots[i] != NULL)
		{
			scan(roots[i]);
		}
	}
}

void collect_white(V t)
{
	if (isInt(t) || t->type == T_IDENT)
		return;

	if (t->color == White && !t->buffered)
	{
		t->color = Black;
		iter_children(t, collect_white);
		free_value(t);
	}
}

void collect_roots(void)
{
	int i;
	V t;
	for (i = 0; i < root_size; i++)
	{
		if (roots[i] != NULL)
		{
			t = roots[i];
			t->buffered = false;
			roots[i] = NULL;
			collect_white(t);
		}
	}
	root_size = 0;
}

void collect_cycles(void)
{
	mark_roots();
	scan_roots();
	collect_roots();
}

void clear_ref(V t)
{
	if (t == NULL || isInt(t) || t->type == T_IDENT)
		return;

	if (--t->refs == 0)
	{
		release_value(t);
	}
	else if (t->color != Green && t->baserefs == 0)
	{
		possible_root(t);
	}
}

void clear_base_ref(V t)
{
	if (t == NULL || isInt(t) || t->type == T_IDENT)
		return;

	t->baserefs--;
	clear_ref(t);
}


V clear_rooted(V t)
{
	if (t == NULL || isInt(t) || t->type == T_IDENT)
		return t;

	t->baserefs--;

	return t;
}

bool is_simple(V t)
{
	return t == NULL || isInt(t) || t->type == T_IDENT || t->color == Green;
}
