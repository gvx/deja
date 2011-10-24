#ifndef STACK_DEF
#define STACK_DEF

#include "value.h"

#define get_head(x) (x->head->data)

typedef struct Node
{
	struct Value *data;
	struct Node *next;
} Node;

typedef struct
{
	int size;
	Node *head;
} Stack;

Stack* new_stack();
void copy_stack(Stack*, Stack*);
void push(Stack*, V);
V pop(Stack*);
int stack_size(Stack*);
void clear_stack(Stack*);

#endif
