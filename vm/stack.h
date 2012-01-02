#ifndef STACK_DEF
#define STACK_DEF

#include "value.h"

#define get_head(x) (x->head->items[0])
#define stack_size(x) (x->size)

#define STACKSIZE 1024

typedef struct StackArray {
	struct StackArray *next;
	int numitems;
	V items[STACKSIZE];
} StackArray;

typedef struct Stack {
	StackArray *head;
	int size;
} Stack;

Stack* new_stack();
void copy_stack(Stack*, Stack*);
void push(Stack*, V);
V pop(Stack*);
void clear_stack(Stack*);

#endif
