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
	Node *cur = old->head;
	Node **to = &new->head;
	while (cur != NULL)
	{
		*to = malloc(sizeof(Node));
		(*to)->data = cur->data;
		cur = cur->next;
		to = &(*to)->next;
	}
	*to = NULL;
	new->size = old->size;
}

void push(Stack *stack, V v)
{
	Node *new_node = malloc(sizeof(Node));
	if (new_node != NULL)
	{
		new_node->data = v;
		new_node->next = (stack->size++ > 0) ? stack->head : NULL;
		stack->head = new_node;
	}
}

V pop(Stack *stack)
{
	if (stack->size > 0)
	{
		Node *top = stack->head;
		V v = top->data;
		stack->size--;
		stack->head = top->next;
		free(top);
		return v;
	}
	return NULL;
}

int stack_size(Stack *stack)
{
	return stack->size;
}

void clear_stack(Stack *stack)
{
	while (stack_size(stack) > 0)
	{
		clear_ref(pop(stack));
	}
	free(stack);
}