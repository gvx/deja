#include <stdlib.h>

#include "gc.h"
#include "value.h"
#include "stack.h"

Stack* newstack()
{
	Stack* nstack = malloc(sizeof(Stack));
	nstack->size = 0;
	nstack->head = NULL;
	return nstack;
}

void push(Stack *stack, V v)
{
	Node *new_node = malloc(sizeof(Node));
	if (new_node != NULL)
	{
		new_node->data = add_ref(v);
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

int size_of(Stack *stack)
{
	return stack->size;
}
