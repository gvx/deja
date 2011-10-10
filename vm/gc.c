#include "gc.h"
#include "value.h"
#include "types.h"
#include "stack.h"
#include "scope.h"
#include "func.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_ROOTS 1024

static int root_size;
static V roots[MAX_ROOTS];

void collect_cycles(void);

V new_value(int type)
{
	V t = malloc(sizeof(Value));
	t->buffered = false;
	t->type = type;
	t->refs = 1;
	t->color = Black;
	return t;
}

V add_ref(V t)
{
	t->refs++;
	if (t->color != Green)
	{
		t->color = Black;
	}
	return t;
}

void free_value(V t)
{
	Stack* s;
	Scope* sc;
	Bucket* b;
	Bucket* bb;
	int n;
	switch (t->type)
	{
		case T_STR:
		case T_IDENT:
			free(t->data.string->data);
			free(t->data.string);
			break;
		case T_FUNC:
			free(toFunc(t));
			break;
		case T_STACK:
			s = toStack(t);
			while (pop(s));
			free(s);
			break;
		case T_SCOPE:
			sc = toScope(t);
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
			free(sc);
			break;
	}
	free(t);
}

void release_value(V t)
{
	V i;
	Stack* s;
	Scope* sc;
	Bucket* b;
	int n;
	switch (t->type)
	{
		case T_FUNC:
			clear_ref(toFunc(t)->defscope);
			break;
		case T_STACK:
			s = toStack(t);
			while (i = pop(s))
			{
				clear_ref(i);
			}
			break;
		case T_SCOPE:
			sc = toScope(t);
			clear_ref(sc->parent);
			clear_ref(sc->func);
			for (n = 0; n < sc->hm.size; n++)
			{
				b = sc->hm.map[n];
				while(b != NULL)
				{
					clear_ref(b->value);
					b = b->next;
				}
			}
			break;
	}
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
	child->refs--;
	mark_gray(child);
}

void mark_gray(V t)
{
	Stack* s;
	Node* c;
	Scope* sc;
	Bucket* b;
	V child;
	int i;
	if (t->color != Gray)
	{
		t->color = Gray;
		switch (t->type)
		{
			case T_FUNC:
				child = toFunc(t)->defscope;
				mark_gray_child(child);
				break;
			case T_STACK:
				s = toStack(t);
				c = s->head;
				for (i = 0; i < s->size; i++)
				{
					child = c->data;
					c = c->next;
					mark_gray_child(child);
				}
				break;
			case T_SCOPE:
				sc = toScope(t);
				mark_gray_child(sc->parent);
				mark_gray_child(sc->func);
				for (i = 0; i < sc->hm.size; i++)
				{
					b = sc->hm.map[i];
					while(b != NULL)
					{
						mark_gray_child(b->value);
						b = b->next;
					}
				}
				break;
		}
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
	child->refs++;
	if (child->color != Black)
	{
		scan_black(child);
	}
}

void scan_black(V t)
{
	Stack* s;
	Node* c;
	Scope* sc;
	Bucket* b;
	V child;
	int i;

	t->color = Black;

	switch (t->type)
	{
		case T_FUNC:
			child = toFunc(t)->defscope;
			scan_black_child(child);
			break;
		case T_STACK:
			s = toStack(t);
			c = s->head;
			for (i = 0; i < s->size; i++)
			{
				child = c->data;
				c = c->next;
				scan_black_child(child);
			}
			break;
		case T_SCOPE:
			sc = toScope(t);
			scan_black_child(sc->parent);
			scan_black_child(sc->func);
			for (i = 0; i < sc->hm.size; i++)
			{
				b = sc->hm.map[i];
				while(b != NULL)
				{
					scan_black_child(b->value);
					b = b->next;
				}
			}
			break;
	}
}

void scan(V t)
{
	Stack* s;
	Node* c;
	Scope* sc;
	Bucket* b;
	V child;
	int i;

	if (t->color == Gray)
	{
		if (t->refs > 0)
		{
			scan_black(t);
		}
		else
		{
			t->color = White;
			switch (t->type)
			{
				case T_FUNC:
					scan(toFunc(t)->defscope);
					break;
				case T_STACK:
					s = toStack(t);
					c = s->head;
					for (i = 0; i < s->size; i++)
					{
						child = c->data;
						scan(child);
						c = c->next;
					}
					break;
				case T_SCOPE:
					sc = t->data.object;
					scan(sc->parent);
					scan(sc->func);
					for (i = 0; i < sc->hm.size; i++)
					{
						b = sc->hm.map[i];
						while(b != NULL)
						{
							scan(b->value);
							b = b->next;
						}
					}
					break;
			}
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
	Stack* s;
	Node* c;
	Scope* sc;
	Bucket* b;
	V child;
	int i;

	if (t->color == White && !t->buffered)
	{
		t->color = Black;
		switch (t->type)
		{
			case T_FUNC:
				collect_white(toFunc(t)->defscope);
				break;
			case T_STACK:
				s = toStack(t);
				c = s->head;
				for (i = 0; i < s->size; i++)
				{
					child = c->data;
					collect_white(child);
					c = c->next;
				}
				break;
			case T_SCOPE:
				sc = toScope(t);
				collect_white(sc->parent);
				collect_white(sc->func);
				for (i = 0; i < sc->hm.size; i++)
				{
					b = sc->hm.map[i];
					while(b != NULL)
					{
						collect_white(b->value);
						b = b->next;
					}
				}
				break;
		}
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
	if (--t->refs == 0)
	{
		release_value(t);
	}
	else if (t->color != Green)
	{
		possible_root(t);
	}
}