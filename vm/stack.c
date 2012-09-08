#include <stdlib.h>

#include "gc.h"
#include "value.h"
#include "stack.h"

Stack* new_stack()
{
	Stack* nstack = malloc(sizeof(Stack));
	nstack->size = 0;
	nstack->used = 0;
	nstack->nodes = NULL;
	return nstack;
}

void copy_stack(Stack *old, Stack *new)
{
	if (old->nodes != NULL)
	{
		new->nodes = calloc(old->size, sizeof(V));
		int i;
		for (i = 0; i < old->used; i++)
		{
			new->nodes[i] = add_ref(old->nodes[i]);
		}
	}
	else
	{
		new->nodes = NULL;
	}
	new->used = old->used;
	new->size = old->size;
}

void push(Stack *stack, V v)
{
	if (stack->nodes == NULL)
	{
		stack->size = 64;
		stack->nodes = calloc(stack->size, sizeof(V));
	}
	if (stack->used == stack->size)
	{
		stack->size *= 2;
		stack->nodes = realloc(stack->nodes, stack->size);
	}
	stack->nodes[stack->used++] = v;
}

void reverse(Stack *stack)
{
	if (stack->nodes)
	{
		int i;
		int u = stack->used;
		for (i = 0; i < u / 2; i++)
		{
			V v = stack->nodes[i];
			stack->nodes[i] = stack->nodes[u - i - 1];
			stack->nodes[u - i] = v;
		}
	}
}

V pop(Stack *stack)
{
	if (stack->used > 0)
	{
		V v = stack->nodes[--stack->used];
		if (stack->used < (stack->size / 4) && stack->size > 64)
		{
			stack->size /= 2;
			stack->nodes = realloc(stack->nodes, stack->size);
		}
		return v;
	}
	return NULL;
}

void clear_stack(Stack *stack)
{
	int i;
	for (i = 0; i < stack->used; i++)
	{
		clear_ref(stack->nodes[i]);
	}
}
