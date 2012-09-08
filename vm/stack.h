#ifndef STACK_DEF
#define STACK_DEF

#include "value.h"

#define get_head(x) (x->nodes[x->used - 1])
#define stack_size(x) (x->used)

typedef struct
{
	int size;
	int used;
	V *nodes;
} Stack;

Stack* new_stack();
void copy_stack(Stack*, Stack*);
void push(Stack*, V);
void reverse(Stack*);
V pop(Stack*);
void clear_stack(Stack*);

#endif
