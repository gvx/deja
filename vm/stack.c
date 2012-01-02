#include <stdlib.h>

#include "gc.h"
#include "value.h"
#include "stack.h"

Stack* new_stack()
{
	Stack* nstack = malloc(sizeof(Stack));
	nstack->size = 0;
	nstack->head = NULL;
	return nstack;
}

void copy_stack(Stack *old, Stack *new)
{
	StackArray *cur = old->head, **to = &(new->head);
	while (cur != NULL)
	{
		*to = calloc(1, sizeof(StackArray));
		int i;
		for (i = 0; i < cur->numitems; i++)
		{
			(*to)->items[i] = add_ref(cur->items[i]);
		}
		cur = cur->next;
		to = &(*to)->next;
	}
	new->size = old->size;
}

void push(Stack *stack, V v)
{
	StackArray *head = stack->head;
	if (!head || head->numitems == STACKSIZE - 1)
	{
		stack->head = calloc(1, sizeof(StackArray));
		stack->head->next = head;
		head = stack->head;
	}
	head->items[head->numitems++] = v;
	stack->size++;
}

V pop(Stack *stack)
{
	if (!stack->size)
		return NULL;
	StackArray *head = stack->head;
	V v = head->items[--head->numitems];
	stack->size--;
	if (!head->numitems && stack->size)
	{
		stack->head = head->next;
		free(head);
	}
	return v;
}

void clear_stack(Stack *stack)
{
	while (stack_size(stack) > 0)
	{
		clear_ref(pop(stack));
	}
	free(stack);
}